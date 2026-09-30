#include "IDSelectDlg.h"
#include "ClonkLauncher.h"
#include "DefinitionDB.h"
#include "IDListCtrl.h"
#include "LauncherRes.h"
#include "ScenStyle.h"

#include <QAbstractButton>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QRadioButton>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <algorithm>

namespace {

constexpr int IDC_LIST = 2194, IDC_VIEW = 2347, IDC_SYMBOLS = 2218, IDC_DETAILS = 2206;
constexpr int IdRole = Qt::UserRole + 1;

// grid of the icon view (large icon spacing of the XP list view with 32x32 icons)
constexpr int GridW = 75, GridH = 75;

// lstrcmpi: case insensitive "word sort" (hyphens and apostrophes are ignored first)
int wordCompare(const QString &a, const QString &b) {
    auto strip = [](QString s) { return s.remove('-').remove('\''); };
    const int r = strip(a).compare(strip(b), Qt::CaseInsensitive);
    if (r != 0)
        return r < 0 ? -1 : 1;
    const int r2 = a.compare(b, Qt::CaseInsensitive);
    return r2 < 0 ? -1 : (r2 > 0 ? 1 : 0);
}

QImage selectedIcon(const QImage &icon) {
    // ILD_SELECTED (ILD_BLEND50): icon blended 50% with the highlight color
    QImage out = icon.convertToFormat(QImage::Format_ARGB32);
    const QColor h = SysColor::highlight();
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x) {
            const QRgb p = out.pixel(x, y);
            if (qAlpha(p) == 0)
                continue;
            out.setPixel(x, y, qRgb((qRed(p) + h.red()) / 2, (qGreen(p) + h.green()) / 2, (qBlue(p) + h.blue()) / 2));
        }
    return out;
}

// Label lines of a large icon item: word wrap into the label width, at most two lines unless the
// item has the focus, words that don't fit are cut with an ellipsis
QStringList labelLines(const QFontMetrics &fm, const QString &text, int width, bool full) {
    QStringList lines;
    QString cur;
    for (const QString &w : text.split(' ', Qt::SkipEmptyParts)) {
        const QString cand = cur.isEmpty() ? w : cur + " " + w;
        if (fm.horizontalAdvance(cand) <= width || cur.isEmpty()) {
            cur = cand;
        } else {
            lines << cur;
            cur = w;
        }
    }
    if (!cur.isEmpty())
        lines << cur;
    if (!full && lines.size() > 2) {
        const QString rest = QStringList(lines.mid(1)).join(' ');
        lines = QStringList{lines[0], rest};
    }
    for (QString &l : lines)
        if (fm.horizontalAdvance(l) > width)
            l = fm.elidedText(l, Qt::ElideRight, width);
    return lines;
}

class IconDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override {
        const QRect r = opt.rect;
        const bool selected = opt.state & QStyle::State_Selected;
        const bool focus = opt.state & QStyle::State_HasFocus;
        const bool active = opt.state & QStyle::State_Active;
        const QImage icon = index.data(Qt::DecorationRole).value<QImage>();
        const QString text = index.data(Qt::DisplayRole).toString();
        const QPoint icon_pos(r.left() + (r.width() - 32) / 2, r.top() + 2);
        p->drawImage(icon_pos, selected && active ? selectedIcon(icon) : icon);
        const QFontMetrics fm(opt.font);
        const int label_w = r.width() - 4;
        const QStringList lines = labelLines(fm, text, label_w, selected);
        int tw = 0;
        for (const QString &l : lines)
            tw = std::max(tw, fm.horizontalAdvance(l));
        // label box: 4 pixels around the text, 3 pixels below the icon
        const QRect label(r.left() + (r.width() - tw) / 2 - 4, icon_pos.y() + 35, tw + 8, fm.height() * lines.size() + 2);
        if (selected)
            p->fillRect(label, active ? SysColor::highlight() : SysColor::btnFace());
        p->setPen(selected && active ? Qt::white : Qt::black);
        int y = label.top() + 1;
        for (const QString &l : lines) {
            p->drawText(QRect(label.left(), y, label.width(), fm.height()), Qt::AlignHCenter | Qt::AlignTop, l);
            y += fm.height();
        }
        if (focus && selected)
            drawFocusRect(*p, label);
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override { return {GridW, GridH}; }
};

// report view: icon and label in the first column, the label is highlighted when selected
class DetailsDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override {
        const QRect r = opt.rect;
        const QFontMetrics fm(opt.font);
        const QString text = index.data(Qt::DisplayRole).toString();
        p->fillRect(r, Qt::white);
        if (index.column() != 0) {
            p->setPen(Qt::black);
            p->drawText(r.adjusted(6, 0, -6, 0), Qt::AlignLeft | Qt::AlignVCenter,
                        fm.elidedText(text, Qt::ElideRight, r.width() - 12));
            return;
        }
        const bool selected = opt.state & QStyle::State_Selected;
        const bool active = opt.state & QStyle::State_Active;
        const QImage icon = index.data(IdRole + 1).value<QImage>();
        p->drawImage(QPoint(r.left() + 2, r.top()), icon);
        const int tw = fm.horizontalAdvance(text);
        const QRect label(r.left() + 36, r.top(), std::min(tw + 6, r.width() - 36), r.height());
        if (selected)
            p->fillRect(label, active ? SysColor::highlight() : SysColor::btnFace());
        p->setPen(selected && active ? Qt::white : Qt::black);
        p->drawText(label.adjusted(2, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, text);
        if (selected && (opt.state & QStyle::State_HasFocus))
            drawFocusRect(*p, label);
    }

    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override {
        const QFontMetrics fm(opt.font);
        const int w = fm.horizontalAdvance(index.data(Qt::DisplayRole).toString()) + (index.column() == 0 ? 44 : 12);
        return {w, 33};
    }
};

// header of the report view (classic, not themed)
class ClassicHeader : public QHeaderView {
public:
    using QHeaderView::QHeaderView;

protected:
    void paintSection(QPainter *p, const QRect &r, int logical) const override {
        p->save();
        p->fillRect(r, SysColor::btnFace());
        const int l = r.left(), t = r.top(), rr = r.right(), b = r.bottom();
        p->setPen(Qt::white);
        p->drawLine(l, t, rr - 1, t);
        p->drawLine(l, t, l, b - 1);
        p->setPen(SysColor::light());
        p->drawLine(l + 1, t + 1, rr - 2, t + 1);
        p->drawLine(l + 1, t + 1, l + 1, b - 2);
        p->setPen(SysColor::btnShadow());
        p->drawLine(l + 1, b - 1, rr - 1, b - 1);
        p->drawLine(rr - 1, t + 1, rr - 1, b - 1);
        p->setPen(SysColor::dkShadow());
        p->drawLine(l, b, rr, b);
        p->drawLine(rr, t, rr, b);
        p->setPen(Qt::black);
        p->setFont(font());
        const QString text = model()->headerData(logical, orientation()).toString();
        p->drawText(r.adjusted(7, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter,
                    QFontMetrics(font()).elidedText(text, Qt::ElideRight, r.width() - 11));
        p->restore();
    }
    QSize sizeHint() const override {
        QSize s = QHeaderView::sizeHint();
        s.setHeight(21);
        return s;
    }
};

} // namespace

IDSelectDlg::IDSelectDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_IDSELECT, parent) {
    // IDSelectDlg::ctor: view mode and sort column from the profile (UnitSelectDlg)
    if (ClonkLauncher *l = ClonkLauncher::instance()) {
        view_mode_ = QString::fromStdString(l->get_cfg("UnitSelectDlg\\m_ViewMode", "0")).toInt();
        sort_column_ = QString::fromStdString(l->get_cfg("UnitSelectDlg\\SortColumn", "0")).toInt();
    }
    // FUN_0041aec0
    setWindowTitle(LauncherRes::str(50511));
    get<QAbstractButton>(1)->setText(LauncherRes::str(50026));
    get<QAbstractButton>(2)->setText(LauncherRes::str(50006));
    get<QLabel>(IDC_VIEW)->setText(LauncherRes::str(50361));
    get<QAbstractButton>(IDC_DETAILS)->setText(LauncherRes::str(50158));
    get<QAbstractButton>(IDC_SYMBOLS)->setText(LauncherRes::str(50338));
    applyClassicButtons(body());

    QWidget *placeholder = control(IDC_LIST);
    const QRect geo = placeholder->geometry();
    placeholder->hide();

    symbols_ = new QListWidget(body());
    symbols_->setGeometry(geo);
    symbols_->setViewMode(QListView::IconMode);
    symbols_->setMovement(QListView::Static);
    symbols_->setResizeMode(QListView::Adjust);
    symbols_->setFlow(QListView::LeftToRight);
    symbols_->setWrapping(true);
    symbols_->setGridSize(QSize(GridW, GridH));
    symbols_->setIconSize(QSize(32, 32));
    symbols_->setUniformItemSizes(true);
    symbols_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    symbols_->setItemDelegate(new IconDelegate(symbols_));
    symbols_->setFont(LauncherRes::feFont());
    symbols_->setFrameShape(QFrame::NoFrame);
    symbols_->setStyleSheet("QListWidget { background: white; }");

    details_ = new QTreeWidget(body());
    details_->setGeometry(geo);
    details_->setColumnCount(3);
    details_->setHeader(new ClassicHeader(Qt::Horizontal, details_));
    details_->setHeaderLabels({LauncherRes::str(50241), LauncherRes::str(50352), LauncherRes::str(50157)});
    details_->setRootIsDecorated(false);
    details_->setIconSize(QSize(32, 32));
    details_->setUniformRowHeights(true);
    details_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    details_->setFont(LauncherRes::feFont());
    details_->header()->setSectionsClickable(true);
    details_->header()->setStretchLastSection(false);
    details_->setStyleSheet("QTreeWidget { background: white; }");
    details_->setItemDelegate(new DetailsDelegate(details_));
    details_->setAllColumnsShowFocus(false);

    // sunken client edge around both views
    for (QAbstractScrollArea *v : {static_cast<QAbstractScrollArea *>(symbols_), static_cast<QAbstractScrollArea *>(details_)}) {
        v->setFrameShape(QFrame::StyledPanel);
        v->setFrameShadow(QFrame::Sunken);
        v->setLineWidth(2);
    }

    auto accept_if_selected = [this]() {
        // IDSelectDlg::OnNotifyList1 (NM_DBLCLK)
        if (!selectedIds().isEmpty())
            onOK();
    };
    connect(symbols_, &QListWidget::itemDoubleClicked, this, accept_if_selected);
    connect(details_, &QTreeWidget::itemDoubleClicked, this, accept_if_selected);
    connect(details_->header(), &QHeaderView::sectionClicked, this, [this](int column) {
        // IDSelectDlg::OnNotifyList1 (LVN_COLUMNCLICK): toggle the direction, sort by the column
        sort_dir_ = -sort_dir_;
        sort_column_ = column;
        if (ClonkLauncher *l = ClonkLauncher::instance())
            l->set_cfg("UnitSelectDlg\\SortColumn", std::to_string(sort_column_));
        sort(column);
    });
    connect(get<QAbstractButton>(IDC_SYMBOLS), &QAbstractButton::clicked, this, [this]() { setViewMode(0); });
    connect(get<QAbstractButton>(IDC_DETAILS), &QAbstractButton::clicked, this, [this]() { setViewMode(1); });
}

void IDSelectDlg::setup(uint32_t category, const QString &caption, const DefinitionDB *defs) {
    // IDSelectDlg::OnInitDialog
    category_ = category;
    caption_ = caption;
    defs_ = defs;
    setWindowTitle(LauncherRes::str(50511) + ": " + caption_);
    order_.clear();
    if (defs_) {
        for (int i = 0; const Def *d = defs_->get(i, category_); ++i)
            order_ << d->id;
        // LVS_SORTASCENDING: items are inserted sorted by their text
        std::stable_sort(order_.begin(), order_.end(), [this](const QString &a, const QString &b) {
            return wordCompare(defs_->byId(a)->name, defs_->byId(b)->name) < 0;
        });
    }
    sort(sort_column_);
    setViewMode(view_mode_);
}

void IDSelectDlg::sort(int column) {
    // compare function 0x41ae30: name (case insensitive), value (higher first), description unsorted
    if (defs_) {
        const int dir = sort_dir_;
        std::stable_sort(order_.begin(), order_.end(), [&](const QString &ia, const QString &ib) {
            const Def *a = defs_->byId(ia);
            const Def *b = defs_->byId(ib);
            int r = 0;
            if (column == 0)
                r = wordCompare(a->name, b->name) * dir;
            else if (column == 1)
                r = a->value > b->value ? -dir : (a->value < b->value ? dir : 0);
            return r < 0;
        });
    }
    fill();
}

void IDSelectDlg::fill() {
    const QStringList sel = selectedIds();
    symbols_->clear();
    details_->clear();
    if (!defs_)
        return;
    for (const QString &id : order_) {
        const Def *d = defs_->byId(id);
        auto *item = new QListWidgetItem(d->name);
        item->setData(Qt::DecorationRole, d->picture);
        item->setData(IdRole, d->id);
        symbols_->addItem(item);
        auto *row = new QTreeWidgetItem({d->name, QString::number(d->value), d->desc});
        row->setData(0, IdRole + 1, d->picture);
        row->setData(0, IdRole, d->id);
        details_->addTopLevelItem(row);
        if (sel.contains(id)) {
            item->setSelected(true);
            row->setSelected(true);
        }
    }
    // LVSCW_AUTOSIZE_USEHEADER
    const QFontMetrics fm(details_->font());
    for (int c = 0; c < 3; ++c) {
        int w = fm.horizontalAdvance(details_->headerItem()->text(c)) + 12;
        for (int i = 0; i < details_->topLevelItemCount(); ++i)
            w = std::max(w, fm.horizontalAdvance(details_->topLevelItem(i)->text(c)) + (c == 0 ? 42 : 12));
        details_->setColumnWidth(c, w);
    }
}

void IDSelectDlg::setViewMode(int mode) {
    // IDSelectDlg::OnSymbolsClicked / OnDetailsClicked: LVS_ICON / LVS_REPORT
    const QStringList sel = selectedIds();
    view_mode_ = mode == 1 ? 1 : 0;
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->set_cfg("UnitSelectDlg\\m_ViewMode", std::to_string(view_mode_));
    get<QRadioButton>(IDC_SYMBOLS)->setChecked(view_mode_ == 0);
    get<QRadioButton>(IDC_DETAILS)->setChecked(view_mode_ == 1);
    symbols_->setVisible(view_mode_ == 0);
    details_->setVisible(view_mode_ == 1);
    // keep the selection in both views
    for (int i = 0; i < symbols_->count(); ++i)
        symbols_->item(i)->setSelected(sel.contains(symbols_->item(i)->data(IdRole).toString()));
    for (int i = 0; i < details_->topLevelItemCount(); ++i)
        details_->topLevelItem(i)->setSelected(sel.contains(details_->topLevelItem(i)->data(0, IdRole).toString()));
}

QStringList IDSelectDlg::selectedIds() const {
    QStringList out;
    if (view_mode_ == 1 && details_) {
        for (int i = 0; i < details_->topLevelItemCount(); ++i)
            if (details_->topLevelItem(i)->isSelected())
                out << details_->topLevelItem(i)->data(0, IdRole).toString();
    } else if (symbols_) {
        for (int i = 0; i < symbols_->count(); ++i)
            if (symbols_->item(i)->isSelected())
                out << symbols_->item(i)->data(IdRole).toString();
    }
    return out;
}

void IDSelectDlg::onOK() {
    // IDSelectDlg::OnOK: every selected object once
    for (const QString &id : selectedIds())
        selected_.increaseIDCount(id, true);
    accept();
}
