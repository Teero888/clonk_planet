#include "QuickStartDlg.h"

#include "ClonkLauncher.h"
#include "GroupEdit.h"
#include "LauncherRes.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QRegularExpression>
#include <QSoundEffect>
#include <QRawFont>
#include <QUrl>
#include <map>

std::function<void(QWidget *)> QuickStartDlg::helpHandler;

namespace {

// Windows XP system colors (the launcher runs without visual styles)
const QColor kHighlight(255, 255, 255); // COLOR_3DHIGHLIGHT
const QColor kLight(241, 239, 226);     // COLOR_3DLIGHT
const QColor kFace(236, 233, 216);      // COLOR_3DFACE
const QColor kShadow(172, 168, 153);    // COLOR_3DSHADOW
const QColor kDkShadow(113, 111, 100);  // COLOR_3DDKSHADOW

// the controls moved in OnInitDialog
enum {
    IDC_HELP = 9,
    IDC_CLOSE = 2,
    IDC_UP = 2034,
    IDC_NEXT = 2061,
    IDC_PREV = 2053,
    IDC_DONTSHOW = 2081,
    IDC_DESC = 2221,
};

// CSoundResource::Play (0x429220): only with Sound\FESamples
void playWave(int id) {
    ClonkLauncher *l = ClonkLauncher::instance();
    if (!l || l->get_cfg("Sound\\FESamples", "1") == "0")
        return;
    static std::map<int, QSoundEffect *> effects;
    QSoundEffect *&fx = effects[id];
    if (!fx) {
        const QString path = LauncherRes::resPath("wave", id);
        if (!QFile::exists(path))
            return;
        fx = new QSoundEffect(qApp);
        fx->setSource(QUrl::fromLocalFile(path));
    }
    fx->stop();
    fx->play();
}

// two pixel edges as DrawEdge draws them: outer top-left, outer bottom-right, inner top-left,
// inner bottom-right
void drawEdge(QPainter &p, const QRect &r, const QColor &otl, const QColor &obr, const QColor &itl,
              const QColor &ibr) {
    const int x0 = r.left(), y0 = r.top(), x1 = r.right(), y1 = r.bottom();
    p.setPen(otl);
    p.drawLine(x0, y0, x1 - 1, y0);
    p.drawLine(x0, y0, x0, y1 - 1);
    p.setPen(obr);
    p.drawLine(x0, y1, x1, y1);
    p.drawLine(x1, y0, x1, y1);
    p.setPen(itl);
    p.drawLine(x0 + 1, y0 + 1, x1 - 2, y0 + 1);
    p.drawLine(x0 + 1, y0 + 1, x0 + 1, y1 - 2);
    p.setPen(ibr);
    p.drawLine(x0 + 1, y1 - 1, x1 - 1, y1 - 1);
    p.drawLine(x1 - 1, y0 + 1, x1 - 1, y1 - 1);
}

// WS_EX_CLIENTEDGE
void drawClientEdge(QPainter &p, const QRect &r) { drawEdge(p, r, kShadow, kHighlight, kDkShadow, kLight); }

// WS_EX_DLGMODALFRAME (without caption / border style): raised edge plus one pixel of face color
void drawModalFrame(QPainter &p, const QRect &r) {
    drawEdge(p, r, kLight, kDkShadow, kHighlight, kShadow);
    p.setPen(kFace);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(2, 2, -3, -3));
}

// Windows XP standard font smoothing: grayscale antialiasing where the font's gasp table asks for
// it (Comic Sans MS: bold from 11 pixels, regular from 16)
QFont withGdiSmoothing(QFont font) {
    static std::map<QString, bool> cache;
    const QString key = font.key();
    auto it = cache.find(key);
    bool gray = false;
    if (it != cache.end()) {
        gray = it->second;
    } else {
        QFont probe = font;
        probe.setStyleStrategy(QFont::NoAntialias);
        const QByteArray t = QRawFont::fromFont(probe).fontTable("gasp");
        auto u16 = [&](int o) { return o + 2 <= t.size() ? (static_cast<uchar>(t[o]) << 8) | static_cast<uchar>(t[o + 1]) : 0; };
        const int ppem = font.pixelSize();
        for (int r = 0, n = u16(2); r < n; ++r) {
            if (ppem <= u16(4 + r * 4)) {
                gray = (u16(6 + r * 4) & 0x2) != 0; // GASP_DOGRAY
                break;
            }
        }
        cache[key] = gray;
    }
    font.setStyleStrategy(gray ? QFont::PreferAntialias : QFont::NoAntialias);
    return font;
}

// Focus rectangle (DrawFocusRect): every other pixel inverted with the DC's background color
void drawFocusRect(QPainter &p, const QRect &r, const QColor &xor_color) {
    p.save();
    p.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    p.setPen(xor_color);
    for (int x = r.left(); x <= r.right(); ++x)
        for (int y : {r.top(), r.bottom()})
            if ((x + y) & 1)
                p.drawPoint(x, y);
    for (int y = r.top() + 1; y < r.bottom(); ++y)
        for (int x : {r.left(), r.right()})
            if ((x + y) & 1)
                p.drawPoint(x, y);
    p.restore();
}

bool focusCuesShown(const QWidget *w) {
    const auto *dlg = qobject_cast<const QuickStartDlg *>(w->window());
    return dlg && dlg->focusCues();
}

// BS_BITMAP push button (classic look): the bitmap centered, clipped to the inside of the edge. A
// push button takes the focus when clicked and is the default button while it has it (black frame).
class BitmapButton : public QAbstractButton {
public:
    BitmapButton(const QPixmap &pix, QWidget *parent) : QAbstractButton(parent), pix_(pix) {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const bool focus = window()->focusWidget() == this;
        const bool down = isDown();
        const QRect r = rect();
        const QRect face = focus ? r.adjusted(1, 1, -1, -1) : r;
        p.fillRect(r, kFace);
        QPoint pos((width() - pix_.width()) / 2, (height() - pix_.height()) / 2);
        if (down)
            pos += QPoint(1, 1);
        p.save();
        p.setClipRect(face.adjusted(2, 2, -2, -2));
        p.drawPixmap(pos, pix_);
        p.restore();
        if (focus) {
            p.setPen(Qt::black);
            p.drawRect(r.adjusted(0, 0, -1, -1));
        }
        if (down) {
            // DFCS_PUSHED: flat shadow frame
            p.setPen(focus ? kShadow : kDkShadow);
            p.drawRect(face.adjusted(0, 0, -1, -1));
            if (!focus) {
                p.setPen(kShadow);
                p.drawRect(face.adjusted(1, 1, -2, -2));
            }
        } else {
            drawEdge(p, face, kHighlight, kDkShadow, kLight, kShadow);
        }
        if (focus && focusCuesShown(this))
            drawFocusRect(p, r.adjusted(4, 4, -4, -4), kFace);
    }

private:
    QPixmap pix_;
};

// GDI text metrics: ascent / descent of the VDMX table of the font if it has an entry for the
// size (what Windows uses), else the hinted metrics
struct LineMetrics {
    int ascent = 0, descent = 0;
};
LineMetrics gdiMetrics(const QFont &font) {
    static std::map<QString, LineMetrics> cache;
    const QString key = font.key();
    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;
    const QFontMetrics fm(font);
    LineMetrics m{fm.ascent(), fm.descent()};
    const QByteArray t = QRawFont::fromFont(font).fontTable("VDMX");
    auto u16 = [&](int o) { return o + 2 <= t.size() ? (static_cast<uchar>(t[o]) << 8) | static_cast<uchar>(t[o + 1]) : 0; };
    auto s16 = [&](int o) { return static_cast<int>(static_cast<short>(u16(o))); };
    const int ppem = font.pixelSize();
    if (t.size() >= 6 && ppem > 0) {
        const int num_ratios = u16(4);
        for (int r = 0; r < num_ratios; ++r) {
            const int ro = 6 + r * 4;
            if (ro + 4 > t.size())
                break;
            const int x_ratio = static_cast<uchar>(t[ro + 1]), y_start = static_cast<uchar>(t[ro + 2]),
                      y_end = static_cast<uchar>(t[ro + 3]);
            // 1:1 aspect ratio (or the "all ratios" entry)
            if (!(x_ratio == 0 && y_start == 0 && y_end == 0) && !(y_start <= x_ratio && x_ratio <= y_end))
                continue;
            const int group = u16(6 + num_ratios * 4 + r * 2);
            const int recs = u16(group);
            for (int e = 0; e < recs; ++e) {
                const int eo = group + 4 + e * 6;
                if (u16(eo) == ppem) {
                    m.ascent = s16(eo + 2);
                    m.descent = -s16(eo + 4);
                    break;
                }
            }
            break;
        }
    }
    cache[key] = m;
    return m;
}

// The check box class of the launcher (vtable 0x4669f8, 0x402a00): transparent background
// (WM_CTLCOLOR reflection 0x402ad0 returns a null brush), frontend font (0x402b10), sounds when
// clicked (0x402bc0: 7000 checked, 7001 unchecked). Drawn as a classic 13x13 check box.
class ClonkCheckBox : public QCheckBox {
public:
    ClonkCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent) { setFocusPolicy(Qt::StrongFocus); }

protected:
    // 0x402bc0 (WM_LBUTTONUP): default handling, then the sound of the new state
    void mouseReleaseEvent(QMouseEvent *event) override {
        QCheckBox::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton)
            playWave(isChecked() ? 7000 : 7001);
    }
    bool hitButton(const QPoint &pos) const override { return rect().contains(pos); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const QRect box(0, (height() - 13) / 2, 13, 13);
        drawClientEdge(p, box);
        p.fillRect(box.adjusted(2, 2, -2, -2), isDown() ? kFace : Qt::white);
        if (isChecked()) {
            // the 7x7 check mark of DrawFrameControl(DFCS_BUTTONCHECK)
            static const char *mark[7] = {"......#", ".....##", "#...###", "##.###.", "#####..", ".###...", "..#...."};
            p.setPen(Qt::black);
            for (int y = 0; y < 7; ++y)
                for (int x = 0; x < 7; ++x)
                    if (mark[y][x] == '#')
                        p.drawPoint(box.left() + 3 + x, box.top() + 3 + y);
        }
        p.setFont(font());
        p.setPen(Qt::black);
        // DrawText(DT_VCENTER | DT_SINGLELINE) with the GDI line height
        const LineMetrics m = gdiMetrics(font());
        const int baseline = (height() - m.ascent - m.descent) / 2 + m.ascent;
        const int x = box.right() + 6;
        p.drawText(QPoint(x, baseline), text());
        if (window()->focusWidget() == this && focusCuesShown(this)) {
            const int top = (height() - m.ascent - m.descent) / 2;
            const int w = QFontMetrics(font()).horizontalAdvance(text());
            drawFocusRect(p, QRect(x - 1, top - 1, w + 2, m.ascent + m.descent + 3), Qt::white);
        }
    }
};

// ---- rich edit 2221 (WS_EX_CLIENTEDGE, read only, no scroll bars) ----
// The description is streamed in as RTF (EM_STREAMIN SF_RTF) and then every character gets the face
// "MS Sans Serif", or the frontend font for descriptions of groups (0x4261f0). Drawn here like the
// rich edit does: one paragraph per \par, word wrap, line height from the font's GDI metrics.

struct RtfFormat {
    bool bold = false, italic = false, underline = false;
    int half_points = 24; // RTF default 12pt
};
struct RtfRun {
    RtfFormat fmt;
    QString text;
};
struct RtfParagraph {
    std::vector<RtfRun> runs;
    RtfFormat mark; // format of the paragraph mark (height of empty paragraphs)
};

QChar cp1252(uchar c) {
    static const ushort high[32] = {0x20AC, 0x81, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
                                    0x2039, 0x0152, 0x8D, 0x017D, 0x8F, 0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
                                    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D, 0x017E, 0x0178};
    return (c >= 0x80 && c < 0xA0) ? QChar(high[c - 0x80]) : QChar(c);
}

std::vector<RtfParagraph> parseRtf(const QByteArray &data, int plain_half_points) {
    std::vector<RtfParagraph> paras(1);
    auto add = [&](const RtfFormat &f, QChar c) {
        auto &runs = paras.back().runs;
        const RtfFormat &last = runs.empty() ? RtfFormat() : runs.back().fmt;
        if (runs.empty() || last.bold != f.bold || last.italic != f.italic || last.underline != f.underline ||
            last.half_points != f.half_points)
            runs.push_back({f, QString()});
        runs.back().text += c;
    };
    if (!data.startsWith("{\\rtf")) {
        // plain text: the control's font
        RtfFormat f;
        f.half_points = plain_half_points;
        paras.back().mark = f;
        for (const char ch : data) {
            if (ch == '\n') {
                paras.push_back({});
                paras.back().mark = f;
            } else if (ch != '\r' && ch != 0) {
                add(f, cp1252(static_cast<uchar>(ch)));
            }
        }
        return paras;
    }
    struct State {
        RtfFormat fmt;
        bool skip = false;
        int uc = 1;
    };
    std::vector<State> stack(1);
    int skip_chars = 0; // characters after \uN
    static const QStringList destinations = {"fonttbl", "colortbl", "stylesheet", "info", "pict", "header",
                                             "footer", "object", "listtable", "listoverridetable", "rsidtbl",
                                             "generator", "xmlnstbl", "themedata", "latentstyles"};
    const int n = data.size();
    for (int i = 0; i < n;) {
        const uchar c = static_cast<uchar>(data[i]);
        State &st = stack.back();
        if (c == '{') {
            stack.push_back(stack.back());
            ++i;
        } else if (c == '}') {
            if (stack.size() > 1)
                stack.pop_back();
            ++i;
        } else if (c == '\\') {
            ++i;
            if (i >= n)
                break;
            const uchar d = static_cast<uchar>(data[i]);
            if (!QChar(d).isLetter()) {
                ++i;
                if (d == '\'') {
                    const int v = data.mid(i, 2).toInt(nullptr, 16);
                    i += 2;
                    if (skip_chars > 0)
                        --skip_chars;
                    else if (!st.skip)
                        add(st.fmt, cp1252(static_cast<uchar>(v)));
                } else if (d == '*') {
                    st.skip = true;
                } else if (d == '\r' || d == '\n') {
                    if (!st.skip) {
                        paras.back().mark = st.fmt;
                        paras.push_back({});
                    }
                } else if (d == '~') {
                    if (!st.skip)
                        add(st.fmt, QChar(0xA0));
                } else if (d == '_') {
                    if (!st.skip)
                        add(st.fmt, QChar('-'));
                } else if (d == '\\' || d == '{' || d == '}') {
                    if (!st.skip)
                        add(st.fmt, QChar(d));
                }
                continue;
            }
            QByteArray word;
            while (i < n && QChar(static_cast<uchar>(data[i])).isLetter())
                word += data[i++];
            QByteArray num;
            if (i < n && data[i] == '-')
                num += data[i++];
            while (i < n && QChar(static_cast<uchar>(data[i])).isDigit())
                num += data[i++];
            if (i < n && data[i] == ' ')
                ++i;
            const bool has_num = !num.isEmpty() && num != "-";
            const int v = num.toInt();
            if (destinations.contains(QString::fromLatin1(word))) {
                st.skip = true;
            } else if (st.skip) {
                // inside a skipped destination
            } else if (word == "par") {
                paras.back().mark = st.fmt;
                paras.push_back({});
            } else if (word == "line") {
                add(st.fmt, QChar::LineSeparator);
            } else if (word == "tab") {
                add(st.fmt, QChar('\t'));
            } else if (word == "b") {
                st.fmt.bold = !has_num || v != 0;
            } else if (word == "i") {
                st.fmt.italic = !has_num || v != 0;
            } else if (word == "ul") {
                st.fmt.underline = !has_num || v != 0;
            } else if (word == "ulnone") {
                st.fmt.underline = false;
            } else if (word == "fs") {
                if (has_num && v > 0)
                    st.fmt.half_points = v;
            } else if (word == "plain") {
                st.fmt = RtfFormat();
            } else if (word == "uc") {
                st.uc = v;
            } else if (word == "u") {
                add(st.fmt, QChar(static_cast<ushort>(v < 0 ? v + 65536 : v)));
                skip_chars = st.uc;
            } else if (word == "emdash") {
                add(st.fmt, QChar(0x2014));
            } else if (word == "endash") {
                add(st.fmt, QChar(0x2013));
            } else if (word == "bullet") {
                add(st.fmt, QChar(0x2022));
            } else if (word == "lquote" || word == "rquote") {
                add(st.fmt, QChar(word == "lquote" ? 0x2018 : 0x2019));
            } else if (word == "ldblquote" || word == "rdblquote") {
                add(st.fmt, QChar(word == "ldblquote" ? 0x201C : 0x201D));
            }
        } else {
            ++i;
            if (c == '\r' || c == '\n' || c == 0)
                continue;
            if (skip_chars > 0) {
                --skip_chars;
                continue;
            }
            if (!st.skip)
                add(st.fmt, cp1252(c));
        }
    }
    // the text after the last \par (normally nothing) is the last paragraph
    if (paras.back().runs.empty() && paras.size() > 1)
        paras.back().mark = paras[paras.size() - 2].mark;
    return paras;
}

class RichEditView : public QWidget {
public:
    using QWidget::QWidget;

    // FUN_004261f0: contents and face of all text
    void setContents(const QByteArray &rtf, const QString &face) {
        face_ = face;
        paras_ = parseRtf(rtf, 16);
        update();
    }

protected:
    QFont fontFor(const RtfFormat &f) const {
        QFont font(face_);
        font.setPixelSize(qRound(f.half_points * 96.0 / 144.0));
        font.setBold(f.bold);
        font.setItalic(f.italic);
        font.setUnderline(f.underline);
        font.setHintingPreference(QFont::PreferFullHinting);
        return withGdiSmoothing(font);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), Qt::white);
        drawClientEdge(p, rect());
        // formatting rectangle: inside the client edge, one pixel inset
        const QRect area = rect().adjusted(3, 3, -3, -2);
        p.setClipRect(rect().adjusted(2, 2, -2, -2));
        p.setPen(Qt::black);
        int y = area.top();
        struct Piece {
            QFont font;
            QString text;
            int width;
            LineMetrics m;
        };
        for (const RtfParagraph &para : paras_) {
            // words (with their trailing spaces) of all runs
            std::vector<Piece> pieces;
            for (const RtfRun &run : para.runs) {
                const QFont font = fontFor(run.fmt);
                const QFontMetrics fm(font);
                const LineMetrics m = gdiMetrics(font);
                QString word;
                auto flush = [&]() {
                    if (!word.isEmpty())
                        pieces.push_back({font, word, fm.horizontalAdvance(word), m});
                    word.clear();
                };
                for (const QChar ch : run.text) {
                    if (ch == QChar::LineSeparator) {
                        flush();
                        pieces.push_back({font, QString(ch), 0, m});
                        continue;
                    }
                    if (ch != ' ' && !word.isEmpty() && word.back() == ' ')
                        flush();
                    word += ch;
                }
                flush();
            }
            const LineMetrics mark = gdiMetrics(fontFor(para.mark));
            size_t i = 0;
            do {
                // fill one line
                size_t end = i;
                int w = 0;
                while (end < pieces.size()) {
                    const Piece &pc = pieces[end];
                    if (pc.text == QString(QChar::LineSeparator)) {
                        ++end;
                        break;
                    }
                    const int trimmed = QFontMetrics(pc.font).horizontalAdvance(QString(pc.text).remove(QRegularExpression(" +$")));
                    if (end > i && w + trimmed > area.width())
                        break;
                    w += pc.width;
                    ++end;
                }
                LineMetrics lm = (end == i || pieces.empty()) ? mark : LineMetrics{};
                for (size_t k = i; k < end; ++k) {
                    lm.ascent = qMax(lm.ascent, pieces[k].m.ascent);
                    lm.descent = qMax(lm.descent, pieces[k].m.descent);
                }
                if (end == pieces.size() && para.runs.empty())
                    lm = mark;
                int x = area.left();
                const int baseline = y + lm.ascent;
                // runs of the same font are drawn in one piece (kerning), tabs go to the next
                // default tab stop (every half inch)
                for (size_t k = i; k < end;) {
                    const QFont &font = pieces[k].font;
                    QString text;
                    for (; k < end && pieces[k].font == font; ++k)
                        if (pieces[k].text != QString(QChar::LineSeparator))
                            text += pieces[k].text;
                    p.setFont(font);
                    const QFontMetrics fm(font);
                    const QStringList parts = text.split('\t');
                    for (int t = 0; t < parts.size(); ++t) {
                        if (t > 0)
                            x = area.left() + ((x - area.left()) / 48 + 1) * 48;
                        p.drawText(QPoint(x, baseline), parts[t]);
                        x += fm.horizontalAdvance(parts[t]);
                    }
                }
                y += lm.ascent + lm.descent;
                i = end;
            } while (i < pieces.size() && y < area.bottom());
            if (y >= area.bottom())
                break;
        }
    }

private:
    QString face_;
    std::vector<RtfParagraph> paras_;
};

// StretchDIBits with COLORONCOLOR: every destination pixel takes the source pixel under its center
QPixmap stretchColorOnColor(const QPixmap &pix, const QSize &size) {
    const QImage src = pix.toImage().convertToFormat(QImage::Format_RGB32);
    QImage dst(size, QImage::Format_RGB32);
    const int sw = src.width(), sh = src.height(), dw = size.width(), dh = size.height();
    for (int y = 0; y < dh; ++y) {
        const int sy = qMin(sh - 1, static_cast<int>((2 * y + 1) * static_cast<qint64>(sh) / (2 * dh)));
        const QRgb *in = reinterpret_cast<const QRgb *>(src.constScanLine(sy));
        QRgb *out = reinterpret_cast<QRgb *>(dst.scanLine(y));
        for (int x = 0; x < dw; ++x)
            out[x] = in[qMin(sw - 1, static_cast<int>((2 * x + 1) * static_cast<qint64>(sw) / (2 * dw)))];
    }
    return QPixmap::fromImage(dst);
}

// Desc<language>.rtf of a group, else any Desc*.rtf (item loader 0x431c50)
QByteArray descriptionFile(const ItemPath &path, const QString &language) {
    C4Group grp;
    if (!GroupEdit::open(path, grp))
        return {};
    auto d = grp.getFile(("Desc" + language + ".rtf").toStdString());
    if (d.empty())
        for (const auto &e : grp.getEntries()) {
            const QString n = QString::fromStdString(e.name);
            if (n.startsWith("Desc", Qt::CaseInsensitive) && n.endsWith(".rtf", Qt::CaseInsensitive)) {
                d = grp.getFile(e.name);
                break;
            }
        }
    return QByteArray(reinterpret_cast<const char *>(d.data()), static_cast<qsizetype>(d.size()));
}

QString dataDir() {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l ? l->planetDataPath() : QDir::currentPath();
}

bool germanLanguage() {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l && QString::fromStdString(l->getLanguage()).compare("DE", Qt::CaseInsensitive) == 0;
}

} // namespace

// QuickStartDlg::QuickStartDlg (0x424e60) + OnInitDialog (0x4253c0)
QuickStartDlg::QuickStartDlg(QWidget *parent) : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    // ctor: picture rects of the main page and of the folder page (SetRect: right/bottom exclusive)
    main_rects_[0] = QRect(QPoint(45, 60), QPoint(194, 174));
    main_rects_[1] = QRect(QPoint(200, 60), QPoint(349, 174));
    main_rects_[2] = QRect(QPoint(355, 60), QPoint(504, 174));
    folder_rects_[0] = QRect(QPoint(45, 25), QPoint(194, 139));
    folder_rects_[1] = QRect(QPoint(200, 25), QPoint(349, 139));
    folder_rects_[2] = QRect(QPoint(355, 25), QPoint(504, 139));

    // OnInitDialog: bitmaps, German variants one id lower
    const int de = germanLanguage() ? 1 : 0;
    background_ = LauncherRes::bitmap(1021);
    logo_ = LauncherRes::bitmap(1035 - de);
    for (int i = 0; i < 6; ++i)
        pictures_[i] = LauncherRes::bitmap(1024 + i);
    texts_[0] = LauncherRes::bitmap(1033 - de); // Tutorial Mouse Control
    texts_[1] = LauncherRes::bitmap(1031 - de); // Tutorial Keyboard Control
    texts_[2] = LauncherRes::bitmap(1023 - de); // Easy Worlds

    // the dialog gets the size of the background, centered on the main window (CenterWindow)
    setFixedSize(background_.isNull() ? QSize(550, 412) : background_.size());

    // controls (template 3025, moved to their pixel positions in OnInitDialog)
    auto button = [this](int id, int bitmap, const QRect &r) {
        auto *b = new BitmapButton(LauncherRes::bitmap(bitmap), this);
        b->setObjectName(QString("ctrl_%1").arg(id));
        b->setGeometry(r);
        return b;
    };
    btn_up_ = button(IDC_UP, 1001, QRect(22, 25, 18, 18));
    btn_prev_ = button(IDC_PREV, 1004, QRect(22, 122, 18, 18));
    btn_next_ = button(IDC_NEXT, 1005, QRect(510, 122, 18, 18));
    btn_close_ = button(IDC_CLOSE, 1002, QRect(527, 7, 17, 14));
    btn_help_ = button(IDC_HELP, 1003, QRect(508, 7, 17, 14));

    desc_frame_ = new RichEditView(this);
    desc_frame_->setObjectName(QString("ctrl_%1").arg(IDC_DESC));
    desc_frame_->setGeometry(45, 150, 460, 200);
    desc_frame_->hide();

    dont_show_ = new ClonkCheckBox(LauncherRes::str(50013), this); // "Don't display this screen in the future."
    dont_show_->setObjectName(QString("ctrl_%1").arg(IDC_DONTSHOW));
    dont_show_->setFont(LauncherRes::feFont());
    dont_show_->setGeometry(144, 368, 325, 24);
    dont_show_->setFocus();

    connect(btn_prev_, &QAbstractButton::clicked, this, [this]() { scroll(-1); });
    connect(btn_next_, &QAbstractButton::clicked, this, [this]() { scroll(1); });
    connect(btn_up_, &QAbstractButton::clicked, this, [this]() { back(); });
    connect(btn_close_, &QAbstractButton::clicked, this, [this]() { reject(); });
    connect(btn_help_, &QAbstractButton::clicked, this, [this]() { showHelp(); });

    // tab order of the template: check box, "<", ">", "^", "x", "?"
    setTabOrder(dont_show_, btn_prev_);
    setTabOrder(btn_prev_, btn_next_);
    setTabOrder(btn_next_, btn_up_);
    setTabOrder(btn_up_, btn_close_);
    setTabOrder(btn_close_, btn_help_);
    installEventFilter(this);
    // mouse messages go to the child windows only (the dialog doesn't see moves over them)
    for (QWidget *w : findChildren<QWidget *>(Qt::FindDirectChildrenOnly))
        w->setAttribute(Qt::WA_NoMousePropagation);
    for (QWidget *w : {static_cast<QWidget *>(dont_show_), static_cast<QWidget *>(btn_prev_), static_cast<QWidget *>(btn_next_),
                       static_cast<QWidget *>(btn_up_), static_cast<QWidget *>(btn_close_), static_cast<QWidget *>(btn_help_)})
        w->installEventFilter(this);

    updateButtons();
}

QuickStartDlg::~QuickStartDlg() = default;

bool QuickStartDlg::enabled(ClonkLauncher *l) {
    if (!l)
        return false;
    // Config.Explorer.ShowQuickStart and ExplorerDlg view mode 0 (player view; developer view only
    // exists in developer mode)
    if (l->get_cfg("Explorer\\ShowQuickStart", "1") == "0")
        return false;
    const bool developer_view = l->developerMode() && l->get_cfg("Explorer\\Mode", "0") == "1";
    return !developer_view;
}

void QuickStartDlg::showAtStartup(ClonkLauncher *l) {
    if (!enabled(l))
        return;
    auto *dlg = new QuickStartDlg(l);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open();
}

int QuickStartDlg::exec() {
    in_exec_ = true;
    const int r = QDialog::exec();
    in_exec_ = false;
    return r;
}

// SharedDlg::Handler_414200: help contents
void QuickStartDlg::showHelp() {
    if (helpHandler) {
        helpHandler(this);
        return;
    }
    if (ClonkLauncher *l = ClonkLauncher::instance())
        QMetaObject::invokeMethod(l, "showHelp");
}

// OnDestroy (0x426c00): the check box clears Explorer\ShowQuickStart
void QuickStartDlg::done(int result) {
    if (dont_show_->isChecked())
        if (ClonkLauncher *l = ClonkLauncher::instance())
            l->set_cfg("Explorer\\ShowQuickStart", "0");
    QDialog::done(result);
}

void QuickStartDlg::keyPressEvent(QKeyEvent *event) {
    // Enter: the default button - a focused push button, else IDOK (the hidden button 1
    // "Menu system", CDialog::OnOK ends the dialog like IDCANCEL)
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        auto *b = qobject_cast<QAbstractButton *>(focusWidget());
        if (b && b != dont_show_ && b->isVisible())
            b->click();
        else
            accept();
        return;
    }
    QDialog::keyPressEvent(event);
}

// WM_ERASEBKGND (0x425390) + OnPaint (0x425230)
void QuickStartDlg::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawPixmap(0, 0, background_);
    if (items_.empty()) {
        p.drawPixmap(110, 10, logo_);
        for (int i = 0; i < 3; ++i)
            p.drawPixmap(main_rects_[i].topLeft(), pictures_[i * 2 + (hover_ == i + 1 ? 1 : 0)]);
        if (hover_ >= 1 && hover_ <= 3)
            p.drawPixmap(110, 175, texts_[hover_ - 1]);
        // a folder without scenarios leaves the (empty) statics visible on the main page
        if (statics_visible_)
            for (int slot = 0; slot < 3; ++slot)
                drawStaticFrame(p, slot);
        return;
    }
    // FUN_00425e20: Title.bmp of the items in the three slots, stretched to the window rect of the
    // static (slot_images_: what is on screen there, see setFrames); then the statics draw their frames
    for (int slot = 0; slot < 3; ++slot) {
        p.drawImage(folder_rects_[slot].topLeft(), slot_images_[slot]);
        drawStaticFrame(p, slot);
    }
}

void QuickStartDlg::drawStaticFrame(QPainter &p, int slot) const {
    if (frames_[slot] == 3)
        drawModalFrame(p, folder_rects_[slot]);
    else if (frames_[slot] == 2)
        drawClientEdge(p, folder_rects_[slot]);
}

// what the dialog paints in the rect of a static: background and the item's picture
QImage QuickStartDlg::slotBase(int slot) const {
    const QRect r = folder_rects_[slot];
    QImage img(r.size(), QImage::Format_RGB32);
    QPainter p(&img);
    p.drawPixmap(QPoint(0, 0), background_, r);
    const Entry *e = entryAt(first_ + slot);
    if (e && !e->info.picture.isNull())
        p.drawPixmap(0, 0, e->info.picture);
    return img;
}

// InvalidateRect of the statics' rects: the dialog paints the pictures again
void QuickStartDlg::repaintSlots() {
    for (int slot = 0; slot < 3; ++slot)
        slot_images_[slot] = slotBase(slot);
    update();
}

// FUN_00425ec0: ModifyStyleEx(..., SWP_FRAMECHANGED) of the three statics, the hovered one gets
// WS_EX_DLGMODALFRAME (3 pixels, raised), the others WS_EX_CLIENTEDGE (2 pixels, sunken). While the
// statics are visible, SetWindowPos moves the client area bits along with the new client origin (the
// static paints nothing itself) and only the newly uncovered pixels are painted by the dialog again:
// the picture of a slot whose frame changed stays displaced by a pixel until the next full repaint
// (as in the screenshots).
void QuickStartDlg::setFrames(int hover) {
    for (int slot = 0; slot < 3; ++slot) {
        const int old_inset = frames_[slot];
        const int new_inset = hover == slot + 1 ? 3 : 2;
        if (old_inset == new_inset)
            continue; // ModifyStyleEx without change does nothing
        frames_[slot] = new_inset;
        if (!statics_visible_)
            continue;
        const QImage prev = slot_images_[slot];
        const QSize size = folder_rects_[slot].size();
        const QRect old_client(old_inset, old_inset, size.width() - 2 * old_inset, size.height() - 2 * old_inset);
        const QRect new_client(new_inset, new_inset, size.width() - 2 * new_inset, size.height() - 2 * new_inset);
        QPainter p(&slot_images_[slot]);
        p.drawImage(new_client.topLeft(), slotBase(slot), new_client);
        p.setClipRect(new_client);
        p.drawImage(new_client.topLeft(), prev, old_client);
    }
}

int QuickStartDlg::hitTest(const QPoint &pt, bool folder) const {
    // FUN_004257a0: PtInRect with inclusive right / bottom edge
    int hit = 0;
    for (int i = 0; i < 3; ++i) {
        const QRect &r = folder ? folder_rects_[i] : main_rects_[i];
        if (pt.x() >= r.left() && pt.x() <= r.right() + 1 && pt.y() >= r.top() && pt.y() <= r.bottom() + 1)
            hit = i + 1;
    }
    return hit;
}

// OnMousemove (0x425700)
void QuickStartDlg::hoverAt(const QPoint &pt) {
    const int hit = hitTest(pt, !items_.empty());
    if (hit == hover_)
        return;
    hover_ = hit;
    setFrames(hover_);
    update();
    if (hover_ != 0)
        playWave(7009);
    updateDescription();
}

void QuickStartDlg::mouseMoveEvent(QMouseEvent *event) {
    hoverAt(event->position().toPoint());
    QDialog::mouseMoveEvent(event);
}

// OnLbuttonup (0x425840)
void QuickStartDlg::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QDialog::mouseReleaseEvent(event);
        return;
    }
    if (!clickAt(event->position().toPoint()))
        QDialog::mouseReleaseEvent(event);
}

bool QuickStartDlg::clickAt(const QPoint &pt) {
    playWave(7005);
    if (!items_.empty()) {
        const int hit = hitTest(pt, true);
        if (hit != 0)
            startScenario(first_ + hit - 1);
        return true;
    }
    switch (hitTest(pt, false)) {
    case 1:
        openFolder("Tutorial.c4f\\Mouse.c4f");
        return true;
    case 2:
        openFolder("Tutorial.c4f\\Keyboard.c4f");
        return true;
    case 3:
        openFolder("Easy.c4f");
        return true;
    default:
        return false;
    }
}

// FUN_00425930: every *.c4s of the folder (in group order) becomes an item
bool QuickStartDlg::openFolder(const QString &folder) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const ItemPath group = ItemPath(dataDir(), folder.split(QRegularExpression("[\\\\/]"), Qt::SkipEmptyParts)).normalized();
    C4Group grp;
    QStringList names;
    bool ok = false;
    if (GroupEdit::isDirectory(group)) {
        ok = true;
        names = QDir(group.disk).entryList({"*.c4s"}, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    } else if (GroupEdit::open(group, grp)) {
        ok = true;
        const QRegularExpression wildcard(QRegularExpression::wildcardToRegularExpression("*.c4s"),
                                          QRegularExpression::CaseInsensitiveOption);
        for (const auto &e : grp.getEntries()) {
            const QString n = QString::fromStdString(e.name);
            if (wildcard.match(n).hasMatch())
                names << n;
        }
    }
    if (!ok) {
        QApplication::restoreOverrideCursor();
        return false;
    }

    ClonkLauncher *l = ClonkLauncher::instance();
    ExplorerContext ctx;
    if (l)
        ctx = l->explorerContext();
    ctx.data_dir = dataDir();
    ctx.developer_view = false; // Item::Init(..., 0): player view items
    for (const QString &n : names) {
        Entry e;
        if (!initExplorerItem(e.item, group.child(n), ctx))
            continue;
        e.info = loadExplorerInfo(e.item, ctx);
        e.desc = descriptionFile(e.item.path, ctx.language);
        // FUN_00425e20 / 0x4017d0: stretched to the rect of the static
        if (!e.info.picture.isNull())
            e.info.picture = stretchColorOnColor(e.info.picture, folder_rects_[0].size());
        items_.push_back(e);
    }

    // the statics become visible, buttons, description, everything painted again
    statics_visible_ = true;
    updateButtons();
    updateDescription();
    repaintSlots();
    QApplication::restoreOverrideCursor();
    return true;
}

// On2053Clicked / On2061Clicked
void QuickStartDlg::scroll(int delta) {
    if (items_.empty())
        return;
    first_ += delta;
    repaintSlots(); // FUN_00425f60
    updateButtons();
}

// On2034Clicked: back to the main page (the hovered index is kept)
void QuickStartDlg::back() {
    items_.clear();
    first_ = 0;
    statics_visible_ = false;
    updateButtons();
    update();
}

// FUN_00425fd0: "<" if scrolled, ">" if there are more items after the three visible, "^" on a
// folder page
void QuickStartDlg::updateButtons() {
    const int count = static_cast<int>(items_.size());
    if (hidden_focus_ && focusWidget() != this)
        hidden_focus_ = nullptr;
    // ModifyStyle(WS_VISIBLE): a hidden button keeps the focus (and gets it back with its
    // default button look when it is shown again); in Qt the dialog holds it meanwhile
    auto show = [this](QAbstractButton *b, bool visible) {
        if (!visible && b->isVisibleTo(this) && focusWidget() == b) {
            hidden_focus_ = b;
            setFocus();
        }
        b->setVisible(visible);
        if (visible && hidden_focus_ == b) {
            b->setFocus();
            hidden_focus_ = nullptr;
        }
    };
    show(btn_prev_, count != 0 && first_ >= 1);
    show(btn_next_, count != 0 && count - first_ >= 4);
    show(btn_up_, count != 0);
}

void QuickStartDlg::setFocusCues(bool on) {
    if (focus_cues_ == on)
        return;
    focus_cues_ = on;
    for (QWidget *w : findChildren<QWidget *>())
        w->update();
}

// WM_CHANGEUISTATE: Alt or keyboard navigation shows the focus rectangles
bool QuickStartDlg::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_Alt:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            setFocusCues(true);
            break;
        default:
            break;
        }
    }
    return QDialog::eventFilter(watched, event);
}

const QuickStartDlg::Entry *QuickStartDlg::entryAt(int index) const {
    if (index < 0 || index >= static_cast<int>(items_.size()))
        return nullptr;
    return &items_[static_cast<size_t>(index)];
}

// FUN_004263e0 / 0x4261f0: the rich edit shows the description of the hovered item (empty for an
// empty slot) in MS Sans Serif, or the frontend font for descriptions of groups
void QuickStartDlg::updateDescription() {
    if (items_.empty() || hover_ < 1 || hover_ > 3) {
        desc_frame_->hide();
        return;
    }
    const Entry *e = entryAt(first_ + hover_ - 1); // FUN_00426460
    const QString face = (e && e->info.frontend_font ? LauncherRes::feFont() : LauncherRes::sysFont()).family();
    static_cast<RichEditView *>(desc_frame_)->setContents(e ? e->desc : QByteArray(), face);
    desc_frame_->show();
}

// FUN_004264a0: starts the scenario (the original runs the engine itself with the first player
// files of the data directory and waits for it, the quick start screen and the main window hidden)
void QuickStartDlg::startScenario(int index) {
    const Entry *e = entryAt(index);
    ClonkLauncher *l = ClonkLauncher::instance();
    if (!e || !l)
        return;
    const ItemPath path = e->item.path;
    l->startScenario(path);
    // the launcher hides itself while the game runs: hide this screen too and show it again
    // afterwards (not while in exec(), hiding would end it)
    if (!in_exec_ && !l->isVisible() && isVisible()) {
        hide();
        class ShowFilter : public QObject {
        public:
            ShowFilter(QuickStartDlg *dlg, QObject *parent) : QObject(parent), dlg_(dlg) {}
            bool eventFilter(QObject *, QEvent *ev) override {
                if (ev->type() == QEvent::Show) {
                    if (dlg_)
                        dlg_->show();
                    deleteLater();
                }
                return false;
            }

        private:
            QPointer<QuickStartDlg> dlg_;
        };
        l->installEventFilter(new ShowFilter(this, l));
    }
}
