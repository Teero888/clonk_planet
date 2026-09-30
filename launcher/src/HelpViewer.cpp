#include "HelpViewer.h"

#include "ClonkLauncher.h"
#include "LauncherRes.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QCursor>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QHelpEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollBar>
#include <QShortcut>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTreeWidget>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

#include <cmath>
#include <memory>

namespace {

// ------------------------------------------------------------------------------------ help data

const char *BUTTON_FACE = "#ece9d8"; // COLOR_BTNFACE of the XP theme

bool germanUi() {
    if (ClonkLauncher *l = ClonkLauncher::instance())
        return QString::fromStdString(l->get_cfg("General\\Language", "")).compare("DE", Qt::CaseInsensitive) == 0;
    return false;
}

// WinHelp's own texts (winhlp32 is a system component: English / German Windows); the launcher
// string table is used where it has the text
QString uiText(const char *key) {
    struct Text {
        const char *key, *us, *de;
    };
    static const Text texts[] = {
        {"topics_title", "Help Topics: %1", "Hilfethemen: %1"},
        {"help_topics", "Help &Topics", "&Hilfethemen"},
        {"back", "&Back", "&Zurück"},
        {"contents", "Contents", "Inhalt"},
        {"index", "Index", "Index"},
        {"open", "&Open", "Ö&ffnen"},
        {"close", "&Close", "&Schließen"},
        {"contents_hint", "Click a book, and then click Open. Or click another tab, such as Index.",
         "Klicken Sie auf ein Buch und anschließend auf \"Öffnen\". Oder klicken Sie auf eine andere "
         "Registerkarte, z.B. \"Index\"."},
        {"index_1", "&1   Type the first few letters of the word you're looking for.",
         "&1   Geben Sie die ersten Buchstaben des gesuchten Wortes ein."},
        {"index_2", "&2   Click the index entry you want, and then click Display.",
         "&2   Klicken Sie auf den gewünschten Indexeintrag und dann auf \"Anzeigen\"."},
        {"topics_found", "Topics Found", "Gefundene Themen"},
        {"topics_found_hint", "Click a topic, and then click Display.",
         "Klicken Sie auf ein Thema und dann auf \"Anzeigen\"."},
        {"file", "&File", "&Datei"},
        {"exit", "E&xit", "&Beenden"},
        {"edit", "&Edit", "&Bearbeiten"},
        {"copy", "&Copy", "&Kopieren"},
        {"options_menu", "&Options", "&Optionen"},
        {"on_top", "&Keep Help on Top", "Hilfe &immer im Vordergrund"},
        {"font", "&Font", "&Schriftart"},
        {"small", "&Small", "&Klein"},
        {"normal", "&Normal", "&Normal"},
        {"large", "&Large", "&Groß"},
        {"print", "&Print Topic...", "Thema &drucken..."},
    };
    const bool de = germanUi();
    for (const Text &t : texts)
        if (qstrcmp(t.key, key) == 0)
            return QString::fromUtf8(de ? t.de : t.us);
    return QString::fromUtf8(key);
}

struct ContentsItem {
    QString title;
    QString file; // empty for books
    QVector<ContentsItem> children;
};

class HelpData {
public:
    static HelpData &get() {
        static HelpData data;
        return data;
    }

    bool ok() const { return ok_; }
    QString dir() const { return dir_; }
    QString title() const { return title_; }
    QString caption() const { return caption_; }
    QColor background() const { return bg_; }
    QColor nsrBackground() const { return nsr_bg_; }
    const QVector<ContentsItem> &contents() const { return contents_; }
    const QVector<QPair<QString, QStringList>> &keywords() const { return keywords_; }
    QString notAvailableTopic() const { return not_available_; }
    QRectF windowRect() const { return window_rect_; }

    QString topicForId(int id) const { return ids_.value(id); }
    bool isPopup(const QString &file) const { return popups_.contains(file); }
    QString topicTitle(const QString &file) const { return titles_.value(file); }
    bool exists(const QString &file) const { return titles_.contains(file); }

    // body of a topic file: the non scrolling region and the scrolling text (HTML fragments)
    void topicHtml(const QString &file, QString *nsr, QString *text) const {
        QFile f(QDir(dir_).filePath(file));
        QString html;
        if (f.open(QIODevice::ReadOnly))
            html = QString::fromUtf8(f.readAll());
        nsr->clear();
        const int a = html.indexOf("<!--NSR-->"), b = html.indexOf("<!--/NSR-->");
        if (a >= 0 && b > a) {
            *nsr = html.mid(a + 10, b - a - 10);
            html.remove(a, b + 11 - a);
        }
        const int s = html.indexOf("<div class=\"scroll\">"), e = html.lastIndexOf("</div>");
        *text = (s >= 0 && e > s) ? html.mid(s + 20, e - s - 20) : html;
    }

private:
    HelpData() {
        const QDir base(QDir(QCoreApplication::applicationDirPath()).filePath("data/help"));
        const QString lang = germanUi() ? "de" : "en";
        dir_ = base.filePath(lang);
        if (!QFile::exists(QDir(dir_).filePath("context.json")))
            dir_ = base.filePath("en");
        const QJsonObject help = readJson("help.json").object();
        title_ = help.value("title").toString("Clonk Planet Help");
        const QJsonArray windows = help.value("windows").toArray();
        caption_ = title_;
        if (!windows.isEmpty()) {
            const QJsonObject w = windows.first().toObject();
            // WinHelp shows the caption of the main window definition (German in Planet.hlp)
            if (germanUi() && !w.value("caption").toString().isEmpty())
                caption_ = w.value("caption").toString();
            if (w.value("color").isString())
                bg_ = QColor(w.value("color").toString());
            if (w.value("nsr_color").isString())
                nsr_bg_ = QColor(w.value("nsr_color").toString());
            // position and size in 1/1024 of the screen
            window_rect_ = QRectF(w.value("x").toInt() / 1024.0, w.value("y").toInt() / 1024.0,
                                  w.value("width").toInt() / 1024.0, w.value("height").toInt() / 1024.0);
        }
        const QJsonObject ctx = readJson("context.json").object();
        const QJsonObject ids = ctx.value("ids").toObject();
        for (auto it = ids.begin(); it != ids.end(); ++it)
            ids_.insert(it.key().toInt(), it.value().toString());
        const QJsonArray topics = readJson("topics.json").array();
        for (const QJsonValue &v : topics) {
            const QJsonObject t = v.toObject();
            const QString file = t.value("file").toString();
            titles_.insert(file, t.value("title").toString());
            if (t.value("popup").toBool())
                popups_.insert(file, true);
        }
        // the first topic of Planet.hlp is the popup "Subject not available"
        if (!topics.isEmpty() && topics.first().toObject().value("popup").toBool())
            not_available_ = topics.first().toObject().value("file").toString();
        const QJsonObject contents = readJson("contents.json").object();
        contents_ = parseContents(contents.value("items").toArray());
        for (const QJsonValue &v : readJson("keywords.json").array()) {
            const QJsonObject k = v.toObject();
            QStringList files;
            for (const QJsonValue &f : k.value("files").toArray())
                files << f.toString();
            keywords_.append({k.value("keyword").toString(), files});
        }
        std::sort(keywords_.begin(), keywords_.end(), [](const auto &a, const auto &b) {
            return QString::compare(a.first, b.first, Qt::CaseInsensitive) < 0;
        });
        ok_ = !titles_.isEmpty();
    }

    QJsonDocument readJson(const QString &name) const {
        QFile f(QDir(dir_).filePath(name));
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return QJsonDocument::fromJson(f.readAll());
    }

    static QVector<ContentsItem> parseContents(const QJsonArray &items) {
        QVector<ContentsItem> out;
        for (const QJsonValue &v : items) {
            const QJsonObject o = v.toObject();
            ContentsItem item;
            item.title = o.value("title").toString();
            item.file = o.value("file").toString();
            if (o.contains("children"))
                item.children = parseContents(o.value("children").toArray());
            out.append(item);
        }
        return out;
    }

    bool ok_ = false;
    QString dir_, title_, caption_, not_available_;
    QColor bg_ = QColor(0xff, 0xff, 0xe2), nsr_bg_ = QColor(0xc0, 0xc0, 0xc0);
    QRectF window_rect_ = QRectF(0.27, 0.18, 0.46, 0.58);
    QHash<int, QString> ids_;
    QHash<QString, QString> titles_;
    QHash<QString, bool> popups_;
    QVector<ContentsItem> contents_;
    QVector<QPair<QString, QStringList>> keywords_;
};

// ------------------------------------------------------------------------------------ icons

// the icons of the WinHelp 4 contents tab (closed book, open book, topic page)
QPixmap bookIcon(bool open) {
    static const char *closed_xpm[] = {
        "16 16 7 1",
        "  c None",
        ". c #000000",
        "d c #000080",
        "b c #0000ff",
        "l c #8080ff",
        "w c #ffffff",
        "g c #808080",
        "                ",
        "    .........   ",
        "   .dlbbbbbbd.  ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dlbbbbbbd.w. ",
        "  .dddddddd.w.  ",
        "  ..........g.  ",
        "   .wwwwwwww.   ",
        "    ........    ",
    };
    static const char *open_xpm[] = {
        "16 16 6 1",
        "  c None",
        ". c #000000",
        "d c #000080",
        "b c #0000ff",
        "w c #ffffff",
        "g c #808080",
        "                ",
        "                ",
        "                ",
        " ...        ... ",
        " .ww..    ..ww. ",
        " .wwww.  .wwww. ",
        " .wgwww..wwgww. ",
        " .wwgwww.wwwgw. ",
        " .wwwwgw.wwwww. ",
        " .wgwwww.wgwww. ",
        " .wwgwww.wwgww. ",
        " ..wwwgw.wwww.. ",
        " bb..www.ww..bb ",
        "  bbb...b..bbb  ",
        "    dddd.dddd   ",
        "                ",
    };
    return QPixmap(open ? open_xpm : closed_xpm);
}

QPixmap pageIcon() {
    static const char *page_xpm[] = {
        "16 16 5 1",
        "  c None",
        ". c #000000",
        "w c #ffffff",
        "g c #808080",
        "q c #800080",
        "                ",
        "  ........      ",
        "  .wwwwww.w.    ",
        "  .wwwwww.ww.   ",
        "  .wwwqqq....   ",
        "  .wwqwwwqwg.   ",
        "  .wwwwwwqwg.   ",
        "  .wwwwwqwwg.   ",
        "  .wwwwqwwwg.   ",
        "  .wwwwqwwwg.   ",
        "  .wwwwwwwwg.   ",
        "  .wwwwqwwwg.   ",
        "  .wwwwwwwwg.   ",
        "  .ggggggggg.   ",
        "  ...........   ",
        "                ",
    };
    return QPixmap(page_xpm);
}

// ------------------------------------------------------------------------------------ topic text

double g_font_scale = 1.0; // Options > Font: Small / Normal / Large

// scales the point sizes of the converted topics (Options > Font)
QString scaleFonts(const QString &html) {
    if (g_font_scale == 1.0)
        return html;
    static const QRegularExpression re("font-size:([0-9.]+)pt");
    QString out;
    int last = 0;
    auto it = re.globalMatch(html);
    while (it.hasNext()) {
        const auto m = it.next();
        out += html.mid(last, m.capturedStart() - last);
        out += QString("font-size:%1pt").arg(m.captured(1).toDouble() * g_font_scale, 0, 'g', 3);
        last = m.capturedEnd();
    }
    out += html.mid(last);
    return out;
}

// hotspots: jumps green with solid underline, popups green with dotted underline (WinHelp 4)
void styleHotspots(QTextDocument *doc) {
    QTextCursor cursor(doc);
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.isAnchor() || cf.anchorHref().isEmpty())
                continue;
            QTextCharFormat fmt;
            fmt.setForeground(QColor(0, 128, 0));
            fmt.setFontUnderline(true);
            fmt.setUnderlineStyle(cf.anchorHref().contains("?popup") ? QTextCharFormat::DotLine
                                                                       : QTextCharFormat::SingleUnderline);
            cursor.setPosition(frag.position());
            cursor.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(fmt);
        }
    }
}

// a QTextBrowser set up for topic text: help directory as search path, colours, no own navigation
class TopicBrowser : public QTextBrowser {
public:
    explicit TopicBrowser(QWidget *parent, const QColor &bg) : QTextBrowser(parent) {
        setOpenLinks(false);
        setOpenExternalLinks(false);
        setFrameShape(QFrame::NoFrame);
        setSearchPaths({HelpData::get().dir()});
        QPalette pal = palette();
        pal.setColor(QPalette::Base, bg);
        pal.setColor(QPalette::Window, bg);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::Link, QColor(0, 128, 0));
        pal.setColor(QPalette::LinkVisited, QColor(0, 128, 0));
        setPalette(pal);
        viewport()->setAutoFillBackground(true);
        QFont f("Arial");
        f.setPointSizeF(8);
        document()->setDefaultFont(f);
        document()->setDefaultStyleSheet("p { margin-top: 0px; margin-bottom: 0px; } a { color: #008000; }");
    }

    void setTopicHtml(const QString &fragment) {
        setHtml("<html><body>" + scaleFonts(fragment) + "</body></html>");
        styleHotspots(document());
    }
};

enum class LinkKind { None, Jump, Popup, External, Contents };

LinkKind parseLink(const QUrl &url, QString *file) {
    const QString s = url.toString();
    if (s.startsWith("help:contents"))
        return LinkKind::Contents;
    if (url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "mailto" || url.scheme() == "ftp")
        return LinkKind::External;
    if (s.startsWith('#') || s.isEmpty())
        return LinkKind::None;
    *file = url.path();
    if (file->isEmpty() || !HelpData::get().exists(*file))
        return LinkKind::None;
    return url.query() == "popup" ? LinkKind::Popup : LinkKind::Jump;
}

void followLink(QWidget *from, const QUrl &url, const QPoint &global_pos);

// ------------------------------------------------------------------------------------ popup

// popup window of a popup topic: pale yellow, black border, shadow; closes on any click
class HelpPopup : public QDialog {
public:
    static constexpr int SHADOW = 4;

    HelpPopup(QWidget *parent, const QString &file, const QPoint &pos)
        : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint) {
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_TranslucentBackground);
        const HelpData &hd = HelpData::get();
        text_ = new TopicBrowser(this, hd.background());
        text_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        text_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        text_->document()->setDocumentMargin(6);
        QString nsr, body;
        hd.topicHtml(file, &nsr, &body);
        text_->setTopicHtml(nsr + body);
        // WinHelp sizes popups to the text, up to about a third of the screen width
        const QScreen *screen = QGuiApplication::screenAt(pos) ? QGuiApplication::screenAt(pos)
                                                                : QGuiApplication::primaryScreen();
        const QRect avail = screen ? screen->availableGeometry() : QRect(0, 0, 1024, 768);
        const int max_width = std::max(300, avail.width() / 3);
        // measure on a copy: the browser lays its document out for its own (initial) width
        std::unique_ptr<QTextDocument> doc(text_->document()->clone());
        doc->setTextWidth(max_width);
        doc->size();
        const int width = std::min<int>(max_width, static_cast<int>(std::ceil(doc->idealWidth())) + 2);
        doc->setTextWidth(width);
        const int height = static_cast<int>(std::ceil(doc->size().height()));
        text_->setGeometry(1, 1, width, height);
        resize(width + 2 + SHADOW, height + 2 + SHADOW);
        // below / right of the point, kept on the screen
        QPoint p = pos + QPoint(0, 4);
        if (p.x() + this->width() > avail.right())
            p.setX(std::max(avail.left(), avail.right() - this->width()));
        if (p.y() + this->height() > avail.bottom())
            p.setY(std::max(avail.top(), pos.y() - this->height() - 4));
        move(p);
        connect(text_, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
            const QPoint gp = QCursor::pos();
            QWidget *owner = parentWidget();
            close();
            followLink(owner, url, gp);
        });
        text_->viewport()->installEventFilter(this);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const QRect box(0, 0, width() - SHADOW, height() - SHADOW);
        p.fillRect(box.translated(SHADOW, SHADOW), QColor(0, 0, 0, 110));
        p.fillRect(box, HelpData::get().background());
        p.setPen(Qt::black);
        p.drawRect(box.adjusted(0, 0, -1, -1));
    }

    bool eventFilter(QObject *obj, QEvent *ev) override {
        // a click on the text (not on a hotspot) closes the popup
        if (obj == text_->viewport() && ev->type() == QEvent::MouseButtonRelease) {
            auto *me = static_cast<QMouseEvent *>(ev);
            if (text_->anchorAt(me->position().toPoint()).isEmpty()) {
                close();
                return true;
            }
        }
        return QDialog::eventFilter(obj, ev);
    }

    void keyPressEvent(QKeyEvent *) override { close(); }

private:
    TopicBrowser *text_;
};

// ------------------------------------------------------------------------------------ topic window

class TopicsDialog;

// the WinHelp 4 main window: menu, button bar (Help Topics, Back, Options), non scrolling region,
// topic text
class HelpWindow : public QDialog {
public:
    static HelpWindow *instance(QWidget *parent, bool create = true) {
        static QPointer<HelpWindow> window;
        if (!window && create)
            window = new HelpWindow(parent);
        return window;
    }

    explicit HelpWindow(QWidget *parent)
        : QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
        const HelpData &hd = HelpData::get();
        setWindowTitle(hd.caption());
        setFont(LauncherRes::sysFont());
        setAttribute(Qt::WA_DeleteOnClose);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(BUTTON_FACE));
        setPalette(pal);
        setAutoFillBackground(true);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // menu bar
        auto *menu = new QMenuBar(this);
        menu->setFont(LauncherRes::sysFont());
        QMenu *file = menu->addMenu(uiText("file"));
        file->addAction(uiText("exit"), this, &QWidget::close);
        QMenu *edit = menu->addMenu(uiText("edit"));
        edit->addAction(uiText("copy"), this, [this]() { copy(); });
        menu->addMenu(buildOptionsMenu(menu));
        layout->setMenuBar(menu);

        // button bar
        auto *bar = new QWidget(this);
        auto *bar_layout = new QHBoxLayout(bar);
        bar_layout->setContentsMargins(2, 2, 2, 3);
        bar_layout->setSpacing(1);
        auto addButton = [&](const QString &text) {
            auto *b = new QPushButton(text, bar);
            b->setFont(LauncherRes::sysFont());
            b->setFocusPolicy(Qt::NoFocus);
            b->setFixedHeight(23);
            b->setMinimumWidth(std::max(60, b->fontMetrics().horizontalAdvance(QString(text).remove('&')) + 18));
            bar_layout->addWidget(b);
            return b;
        };
        QPushButton *topics = addButton(uiText("help_topics"));
        back_ = addButton(uiText("back"));
        QPushButton *options = addButton(LauncherRes::str(50520)); // "Options"
        bar_layout->addStretch(1);
        layout->addWidget(bar);
        connect(topics, &QPushButton::clicked, this, [this]() { showTopicsDialog(); });
        connect(back_, &QPushButton::clicked, this, [this]() { goBack(); });
        options_menu_ = buildOptionsMenu(this);
        connect(options, &QPushButton::clicked, this, [this, options]() {
            options_menu_->exec(options->mapToGlobal(QPoint(0, options->height())));
        });

        // non scrolling region + topic text
        nsr_ = new TopicBrowser(this, hd.nsrBackground());
        nsr_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        nsr_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        nsr_->document()->setDocumentMargin(4);
        nsr_->hide();
        layout->addWidget(nsr_);
        auto *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Plain);
        line->setFixedHeight(1);
        line->setStyleSheet("color: #808080; background: #808080;");
        nsr_line_ = line;
        line->hide();
        layout->addWidget(line);
        text_ = new TopicBrowser(this, hd.background());
        text_->document()->setDocumentMargin(6);
        text_->setContextMenuPolicy(Qt::CustomContextMenu);
        layout->addWidget(text_, 1);
        for (TopicBrowser *b : {nsr_, text_})
            connect(b, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
                followLink(this, url, QCursor::pos());
            });
        connect(text_, &QWidget::customContextMenuRequested, this,
                [this](const QPoint &p) { options_menu_->exec(text_->viewport()->mapToGlobal(p)); });

        // default size and position of the main window definition (1/1024 of the screen)
        const QScreen *screen = parent ? parent->screen() : QGuiApplication::primaryScreen();
        const QRect avail = screen ? screen->availableGeometry() : QRect(0, 0, 1024, 768);
        const QRectF r = hd.windowRect();
        const int w = std::max(420, static_cast<int>(r.width() * avail.width()));
        const int h = std::max(360, static_cast<int>(r.height() * avail.height()));
        resize(w, h);
        move(avail.left() + static_cast<int>(r.x() * avail.width()),
             avail.top() + static_cast<int>(r.y() * avail.height()));
        back_->setEnabled(false);
    }

    void showTopic(const QString &file, bool add_history = true) {
        const HelpData &hd = HelpData::get();
        if (!hd.exists(file))
            return;
        if (add_history && !current_.isEmpty() && current_ != file)
            history_.append(current_);
        current_ = file;
        QString nsr, body;
        hd.topicHtml(file, &nsr, &body);
        nsr_->setVisible(!nsr.trimmed().isEmpty());
        nsr_line_->setVisible(nsr_->isVisible());
        if (nsr_->isVisible()) {
            nsr_->setTopicHtml(nsr);
            nsr_->document()->setTextWidth(std::max(100, width()));
            nsr_->setFixedHeight(static_cast<int>(std::ceil(nsr_->document()->size().height())));
        }
        text_->setTopicHtml(body);
        text_->verticalScrollBar()->setValue(0);
        back_->setEnabled(!history_.isEmpty());
    }

    void showTopicsDialog();

private:
    QMenu *buildOptionsMenu(QWidget *parent) {
        auto *m = new QMenu(uiText("options_menu"), parent);
        m->addAction(uiText("copy"), this, [this]() { copy(); });
        QMenu *font = m->addMenu(uiText("font"));
        const std::pair<const char *, double> sizes[] = {{"small", 0.85}, {"normal", 1.0}, {"large", 1.25}};
        for (const auto &[key, scale] : sizes) {
            QAction *a = font->addAction(uiText(key));
            a->setCheckable(true);
            const double s = scale;
            connect(font, &QMenu::aboutToShow, a, [a, s]() { a->setChecked(g_font_scale == s); });
            connect(a, &QAction::triggered, this, [this, s]() {
                g_font_scale = s;
                if (!current_.isEmpty())
                    showTopic(current_, false);
            });
        }
        QAction *top = m->addAction(uiText("on_top"));
        top->setCheckable(true);
        connect(m, &QMenu::aboutToShow, top, [this, top]() { top->setChecked(windowFlags() & Qt::WindowStaysOnTopHint); });
        connect(top, &QAction::triggered, this, [this](bool on) {
            setWindowFlag(Qt::WindowStaysOnTopHint, on);
            show();
        });
        return m;
    }

    void copy() {
        // WinHelp copies the whole topic unless text is selected
        QString sel = text_->textCursor().selectedText();
        QApplication::clipboard()->setText(sel.isEmpty() ? text_->toPlainText() : sel.replace(QChar(0x2029), '\n'));
    }

    void goBack() {
        if (history_.isEmpty())
            return;
        const QString file = history_.takeLast();
        showTopic(file, false);
    }

    TopicBrowser *nsr_ = nullptr, *text_ = nullptr;
    QFrame *nsr_line_ = nullptr;
    QPushButton *back_ = nullptr;
    QMenu *options_menu_ = nullptr;
    QString current_;
    QStringList history_;
};

// ------------------------------------------------------------------------------------ Help Topics

// "Help Topics: <title>" with the Contents and Index tabs (HELP_FINDER)
class TopicsDialog : public QDialog {
public:
    static TopicsDialog *instance() { return current_; }

    TopicsDialog(QWidget *parent, int tab)
        : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint) {
        current_ = this;
        const HelpData &hd = HelpData::get();
        setWindowTitle(uiText("topics_title").arg(hd.title()));
        setFont(LauncherRes::sysFont());
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(BUTTON_FACE));
        setPalette(pal);
        setAutoFillBackground(true);
        setFixedSize(412, 392);

        tabs_ = new QTabWidget(this);
        tabs_->setGeometry(7, 7, 398, 342);
        tabs_->setFont(LauncherRes::sysFont());

        // Contents tab
        auto *contents = new QWidget;
        auto *hint = new QLabel(uiText("contents_hint"), contents);
        hint->setWordWrap(true);
        hint->setGeometry(10, 10, 374, 30);
        tree_ = new QTreeWidget(contents);
        tree_->setGeometry(10, 44, 374, 262);
        tree_->setHeaderHidden(true);
        tree_->setRootIsDecorated(false);
        tree_->setIndentation(18);
        tree_->setIconSize(QSize(16, 16));
        tree_->setFont(LauncherRes::sysFont());
        tree_->setUniformRowHeights(true);
        tree_->setStyleSheet("QTreeWidget { background: white; border: 1px solid #7f9db9; }");
        fillTree(tree_->invisibleRootItem(), hd.contents());
        tabs_->addTab(contents, uiText("contents"));

        // Index tab
        auto *index = new QWidget;
        auto *l1 = new QLabel(uiText("index_1"), index);
        l1->setGeometry(10, 10, 374, 16);
        edit_ = new QLineEdit(index);
        edit_->setGeometry(28, 28, 356, 21);
        l1->setBuddy(edit_);
        auto *l2 = new QLabel(uiText("index_2"), index);
        l2->setGeometry(10, 58, 374, 16);
        list_ = new QListWidget(index);
        list_->setGeometry(28, 76, 356, 230);
        list_->setStyleSheet("QListWidget { background: white; border: 1px solid #7f9db9; }");
        l2->setBuddy(list_);
        for (const auto &kw : hd.keywords())
            list_->addItem(kw.first);
        if (list_->count())
            list_->setCurrentRow(0);
        tabs_->addTab(index, uiText("index"));

        // buttons: Open / Close / Display (changes with the selection), Cancel
        action_ = new QPushButton(uiText("open"), this);
        action_->setGeometry(412 - 7 - 75 - 6 - 75, 358, 75, 23);
        action_->setDefault(true);
        auto *cancel = new QPushButton(LauncherRes::str(50006), this); // "Cancel"
        cancel->setGeometry(412 - 7 - 75, 358, 75, 23);
        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(action_, &QPushButton::clicked, this, [this]() { activate(); });

        connect(tree_, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *, int) { activate(); });
        connect(tree_, &QTreeWidget::currentItemChanged, this, [this]() { updateButton(); });
        connect(tree_, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *it) {
            it->setIcon(0, icon(bookIcon(true)));
            updateButton();
        });
        connect(tree_, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem *it) {
            it->setIcon(0, icon(bookIcon(false)));
            updateButton();
        });
        connect(list_, &QListWidget::itemDoubleClicked, this, [this]() { activate(); });
        connect(edit_, &QLineEdit::textEdited, this, [this](const QString &text) {
            // select the first entry starting with the typed letters
            for (int i = 0; i < list_->count(); ++i)
                if (list_->item(i)->text().startsWith(text, Qt::CaseInsensitive)) {
                    list_->setCurrentRow(i);
                    list_->scrollToItem(list_->item(i), QAbstractItemView::PositionAtTop);
                    break;
                }
        });
        connect(tabs_, &QTabWidget::currentChanged, this, [this]() {
            updateButton();
            (tabs_->currentIndex() == 0 ? static_cast<QWidget *>(tree_) : edit_)->setFocus();
        });
        tabs_->setCurrentIndex(tab);
        if (tree_->topLevelItemCount())
            tree_->setCurrentItem(tree_->topLevelItem(0));
        updateButton();
    }

    ~TopicsDialog() override {
        if (current_ == this)
            current_ = nullptr;
    }
private:
    // the icons are not tinted in the selection (tree view of the common controls)
    static QIcon icon(const QPixmap &pix) {
        QIcon i;
        i.addPixmap(pix, QIcon::Normal);
        i.addPixmap(pix, QIcon::Selected);
        return i;
    }

    void fillTree(QTreeWidgetItem *parent, const QVector<ContentsItem> &items) {
        for (const ContentsItem &c : items) {
            auto *it = new QTreeWidgetItem(parent, {c.title});
            if (c.file.isEmpty()) {
                it->setIcon(0, icon(bookIcon(false)));
                fillTree(it, c.children);
            } else {
                it->setIcon(0, icon(pageIcon()));
                it->setData(0, Qt::UserRole, c.file);
            }
        }
    }

    void updateButton() {
        if (tabs_->currentIndex() == 1) {
            action_->setText(LauncherRes::str(50317)); // "Display"
            action_->setEnabled(list_->currentItem() != nullptr);
            return;
        }
        QTreeWidgetItem *it = tree_->currentItem();
        action_->setEnabled(it != nullptr);
        if (it && it->data(0, Qt::UserRole).toString().isEmpty())
            action_->setText(it->isExpanded() ? uiText("close") : uiText("open"));
        else
            action_->setText(LauncherRes::str(50317)); // "Display"
    }

    void activate() {
        QString file;
        if (tabs_->currentIndex() == 1) {
            const int row = list_->currentRow();
            const auto &kws = HelpData::get().keywords();
            if (row < 0 || row >= kws.size())
                return;
            const QStringList files = kws[row].second;
            if (files.isEmpty())
                return;
            file = files.size() == 1 ? files.first() : chooseTopic(files);
        } else {
            QTreeWidgetItem *it = tree_->currentItem();
            if (!it)
                return;
            file = it->data(0, Qt::UserRole).toString();
            if (file.isEmpty()) {
                it->setExpanded(!it->isExpanded());
                return;
            }
        }
        if (file.isEmpty())
            return;
        QWidget *owner = parentWidget();
        if (auto *w = dynamic_cast<HelpWindow *>(owner))
            owner = w->parentWidget();
        accept();
        HelpWindow *w = HelpWindow::instance(owner);
        w->showTopic(file);
        w->show();
        w->raise();
        w->activateWindow();
    }

    // several topics for one keyword: "Topics Found"
    QString chooseTopic(const QStringList &files) {
        QDialog dlg(this);
        dlg.setWindowTitle(uiText("topics_found"));
        dlg.setFont(LauncherRes::sysFont());
        auto *lay = new QVBoxLayout(&dlg);
        lay->addWidget(new QLabel(uiText("topics_found_hint"), &dlg));
        auto *list = new QListWidget(&dlg);
        for (const QString &f : files) {
            auto *it = new QListWidgetItem(HelpData::get().topicTitle(f), list);
            it->setData(Qt::UserRole, f);
        }
        list->setCurrentRow(0);
        lay->addWidget(list);
        auto *row = new QHBoxLayout;
        row->addStretch(1);
        auto *ok = new QPushButton(LauncherRes::str(50317), &dlg);
        auto *cancel = new QPushButton(LauncherRes::str(50006), &dlg);
        ok->setDefault(true);
        row->addWidget(ok);
        row->addWidget(cancel);
        lay->addLayout(row);
        connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
        connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
        if (dlg.exec() != QDialog::Accepted || !list->currentItem())
            return {};
        return list->currentItem()->data(Qt::UserRole).toString();
    }

    static inline TopicsDialog *current_ = nullptr;
    QTabWidget *tabs_ = nullptr;
    QTreeWidget *tree_ = nullptr;
    QLineEdit *edit_ = nullptr;
    QListWidget *list_ = nullptr;
    QPushButton *action_ = nullptr;
};

void openTopicsDialog(QWidget *parent) {
    if (TopicsDialog *d = TopicsDialog::instance()) {
        d->raise();
        d->activateWindow();
        return;
    }
    static int last_tab = 0;
    auto *dlg = new TopicsDialog(parent, last_tab);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dlg, &QDialog::finished, dlg, [dlg]() {
        if (auto *tabs = dlg->findChild<QTabWidget *>())
            last_tab = tabs->currentIndex();
    });
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

void HelpWindow::showTopicsDialog() { openTopicsDialog(this); }

void showPopup(QWidget *parent, const QString &file, const QPoint &pos) {
    auto *popup = new HelpPopup(parent, file, pos);
    popup->show();
}

void followLink(QWidget *from, const QUrl &url, const QPoint &global_pos) {
    QString file;
    switch (parseLink(url, &file)) {
    case LinkKind::Jump: {
        HelpWindow *w = HelpWindow::instance(from);
        w->showTopic(file);
        w->show();
        w->raise();
        break;
    }
    case LinkKind::Popup:
        showPopup(from, file, global_pos);
        break;
    case LinkKind::External:
        QDesktopServices::openUrl(url);
        break;
    case LinkKind::Contents:
        openTopicsDialog(from);
        break;
    case LinkKind::None:
        break;
    }
}

// control id of a widget created by DialogBuilder ("ctrl_<id>"), searching the parents
int controlId(QWidget *w, QWidget *stop) {
    static const QRegularExpression re("^ctrl_(\\d+)$");
    for (; w && w != stop; w = w->parentWidget()) {
        const auto m = re.match(w->objectName());
        if (m.hasMatch())
            return m.captured(1).toInt();
    }
    return 0;
}

// WM_HELP handling of a dialog: F1 and "What's This?" clicks
class ContextHelpFilter : public QObject {
public:
    explicit ContextHelpFilter(QWidget *dialog) : QObject(dialog), dialog_(dialog) {}

    bool eventFilter(QObject *obj, QEvent *ev) override {
        if (ev->type() != QEvent::QueryWhatsThis && ev->type() != QEvent::WhatsThis)
            return false;
        auto *w = qobject_cast<QWidget *>(obj);
        if (!w || w->window() != dialog_)
            return false;
        const int id = controlId(w, dialog_);
        if (!id)
            return false;
        if (ev->type() == QEvent::QueryWhatsThis) {
            ev->accept();
            return true;
        }
        showHelpPopup(dialog_, id, static_cast<QHelpEvent *>(ev)->globalPos());
        return true;
    }

private:
    QWidget *dialog_;
};

} // namespace

// ------------------------------------------------------------------------------------ public API

void showHelpContents(QWidget *parent) {
    // ExplorerDlg::On4001Clicked: WinHelp(0, HELP_FINDER)
    openTopicsDialog(parent);
}

void showHelpContext(QWidget *parent, int context) {
    const HelpData &hd = HelpData::get();
    const QString file = hd.topicForId(context);
    if (file.isEmpty()) {
        openTopicsDialog(parent);
        return;
    }
    if (hd.isPopup(file)) {
        showPopup(parent, file, QCursor::pos());
        return;
    }
    HelpWindow *w = HelpWindow::instance(parent);
    w->showTopic(file);
    w->show();
    w->raise();
    w->activateWindow();
}

void showHelpPopup(QWidget *parent, int context, const QPoint &global_pos) {
    const HelpData &hd = HelpData::get();
    QString file = hd.topicForId(context);
    if (file.isEmpty())
        file = hd.notAvailableTopic();
    if (file.isEmpty())
        return;
    if (!hd.isPopup(file)) {
        showHelpContext(parent, context);
        return;
    }
    showPopup(parent, file, global_pos);
}

void installContextHelp(QWidget *dialog) {
    auto *filter = new ContextHelpFilter(dialog);
    qApp->installEventFilter(filter);
    auto *f1 = new QShortcut(QKeySequence(Qt::Key_F1), dialog);
    f1->setContext(Qt::WindowShortcut);
    QObject::connect(f1, &QShortcut::activated, dialog, [dialog]() {
        QWidget *focus = QApplication::focusWidget();
        const int id = controlId(focus, dialog);
        if (!id)
            return;
        showHelpPopup(dialog, id, QCursor::pos());
    });
}
