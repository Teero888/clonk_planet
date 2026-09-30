// Actions on tree items: New (0x42daf0 / NewDlg), Rename (0x42c080 / OnEndLabelEdit 0x42c3e0),
// Delete (0x42c0f0), copy / move / import by drag & drop (0x42f1d0), the tree's context menu
// (0x42fdc0) and keyboard shortcuts (ExplorerTree::OnKeyDown 0x42d4a0).

#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "C4TextDoc.h"
#include "ExplorerTreeView.h"
#include "GroupEdit.h"
#include "LauncherRes.h"

#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QRandomGenerator>

namespace {

constexpr int ROLE_ID = Qt::UserRole + 1;

QString fmt(int id, const QStringList &args) {
    QString s = LauncherRes::str(id);
    for (const QString &a : args) {
        const int i = s.indexOf("%s");
        if (i < 0)
            break;
        s.replace(i, 2, a);
    }
    return s;
}

// Win32 list view (LVS_LIST) row: small icon at +2, label after it, only the label is highlighted
class ListRowDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override {
        return QSize(QFontMetrics(opt.font).horizontalAdvance(index.data().toString()) + 16 + 8, 17);
    }
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override {
        const QRect r = opt.rect;
        const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        p->drawPixmap(r.x() + 2, r.y(), icon.pixmap(16, 16));
        const QString text = index.data().toString();
        QRect tr(r.x() + 2 + 16 + 2, r.y() + 1, QFontMetrics(opt.font).horizontalAdvance(text) + 3, 15);
        const bool sel = opt.state & QStyle::State_Selected;
        if (sel)
            p->fillRect(tr, QColor("#316ac5"));
        p->setFont(opt.font);
        p->setPen(sel ? Qt::white : Qt::black);
        p->drawText(tr.adjusted(1, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
    }
};

// NewDlg (IDD 3029)
class NewDlg : public ClonkDialog {
public:
    NewDlg(QWidget *parent, const QString &location, bool developer) : ClonkDialog(LauncherRes::IDD_NEW, parent) {
        // NewDlg::OnInitDialog
        setWindowTitle(LauncherRes::str(50519));
        get<QLabel>(2316)->setText(fmt(50221, {location}));
        list_ = new QListWidget(body());
        list_->setGeometry(control(2203)->geometry());
        control(2203)->hide();
        list_->setViewMode(QListView::ListMode); // LVS_LIST
        list_->setFlow(QListView::TopToBottom);
        list_->setWrapping(true);
        list_->setIconSize(QSize(16, 16));
        list_->setFont(LauncherRes::sysFont());
        list_->setItemDelegate(new ListRowDelegate(list_));
        list_->setSpacing(0);
        list_->setFrameShape(QFrame::StyledPanel);
        list_->setStyleSheet(
            "QListWidget { border: 2px solid; border-color: #a6a6a6 #ffffff #ffffff #a6a6a6; background: white; outline: none; }"
            "QListWidget::item { padding: 0px 2px 0px 0px; margin: 0px; height: 17px; color: black; }"
            "QListWidget::item:selected { background: #316ac5; color: white; }");
        const int min_newable = developer ? 1 : 2;
        const QPixmap icons = LauncherRes::bitmap(1015);
        for (int t = 0; t < T_Count; ++t) {
            const ExplorerType &type = explorerType(t);
            if (type.newable < min_newable)
                continue;
            QPixmap icon = icons.copy(type.icon * 16, 0, 16, 16);
            QImage img = icon.toImage().convertToFormat(QImage::Format_ARGB32);
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if ((img.pixel(x, y) & 0xFFFFFF) == 0xFF00FF)
                        img.setPixel(x, y, 0);
            auto *item = new QListWidgetItem(QIcon(QPixmap::fromImage(img)), LauncherRes::str(type.name_id), list_);
            item->setData(Qt::UserRole, t);
        }
        list_->setCurrentRow(-1);
        // NM_DBLCLK = OK
        connect(list_, &QListWidget::itemDoubleClicked, this, [this]() { onOK(); });
    }

    int type() const { return type_; }

protected:
    void onOK() override {
        // NewDlg::OnOK: selected type, T_Unknown if none
        if (QListWidgetItem *it = list_->currentItem())
            type_ = it->data(Qt::UserRole).toInt();
        accept();
    }

private:
    QListWidget *list_ = nullptr;
    int type_ = T_Unknown;
};

} // namespace
// ExplorerTree 0x42fb50: "Title\Title\Title" of an item and its parents, "Working directory" for none
QString ClonkLauncher::itemLocation(QStandardItem *item) {
    if (!item)
        return LauncherRes::str(50706);
    QString s;
    for (QStandardItem *i = item; i; i = i->parent()) {
        ExplorerItem *e = explorerItem(i);
        const QString t = e ? e->title : i->text();
        s = s.isEmpty() ? t : t + "\\" + s;
    }
    return s;
}

QStandardItem *ClonkLauncher::selectedTreeItem() {
    const QModelIndexList sel = tree->selectionModel()->selectedIndexes();
    return sel.isEmpty() ? nullptr : tree_model->itemFromIndex(sel.first());
}

// ExplorerTree 0x430210: original RedWolf item (own group header, files: their group)
bool ClonkLauncher::isOriginalItem(QStandardItem *tree_item) {
    ExplorerItem *e = explorerItem(tree_item);
    if (!e)
        return false;
    if (e->original)
        return true;
    const bool is_group = explorerType(e->type).group || e->type == T_Directory;
    return !is_group && tree_item->parent() && isOriginalItem(tree_item->parent());
}

// ExplorerTree 0x430240: may the item be changed? Original items only where their parent is not
// original (check_parent: delete / move), network references never.
bool ClonkLauncher::canModify(QStandardItem *tree_item, bool check_parent) {
    ExplorerItem *e = explorerItem(tree_item);
    if (!e)
        return false;
    // developer view with the child protection switched off allows everything
    if (developer_view && get_cfg("Explorer\\Kindersicherung", "1") == "0")
        return true;
    if (isNetworkReference(e)) {
        clonkMessage(this, LauncherRes::str(51115)); // "Network reference items may not be edited."
        return false;
    }
    if (!isOriginalItem(tree_item))
        return true;
    if (check_parent && (!tree_item->parent() || !isOriginalItem(tree_item->parent())))
        return true;
    clonkMessage(this, fmt(51108, {e->title}));
    return false;
}

// ExplorerTree 0x42daf0
void ClonkLauncher::onNewClicked() {
    // player view: new items always go to the working directory
    if (!developer_view)
        tree->clearSelection();
    QStandardItem *target_item = selectedTreeItem();
    ExplorerItem *target = explorerItem(target_item);
    if (target_item)
        loadChildren(target_item);

    NewDlg dlg(this, itemLocation(target_item), developer_view);
    if (dlg.exec() != QDialog::Accepted || dlg.type() == T_Unknown)
        return;
    const int type = dlg.type();

    // create the item in the working directory
    const QDir work(planet_data_path);
    QString name;
    if (type == T_Directory) {
        name = LauncherRes::str(50704);
        for (int n = 2; work.exists(name); ++n)
            name = QString("%1 %2").arg(LauncherRes::str(50704)).arg(n);
        if (!work.mkdir(name))
            return;
    } else {
        const QString ext = explorerType(type).ext;
        name = "New." + ext;
        for (int n = 2; work.exists(name); ++n)
            name = QString("New%1.%2").arg(n).arg(ext);
        const std::vector<uint8_t> tmpl = LauncherRes::binary(explorerType(type).template_id);
        bool ok = !tmpl.empty();
        if (ok) {
            const bool is_group = tmpl.size() > 2 && tmpl[0] == 0x1e && tmpl[1] == 0x8c;
            if (is_group) {
                // group templates are packed C4Groups: keep them packed with the maker of the user
                C4Group g;
                ok = g.loadFromMemory(tmpl);
                if (ok) {
                    C4GroupWriter w;
                    w.addFromGroup(g);
                    w.setMaker(GroupEdit::maker().toStdString());
                    ok = w.writeToFile(work.filePath(name).toStdString(), true);
                }
            } else {
                QFile f(work.filePath(name));
                ok = f.open(QIODevice::WriteOnly) &&
                     f.write(reinterpret_cast<const char *>(tmpl.data()), tmpl.size()) == static_cast<qint64>(tmpl.size());
            }
        }
        if (!ok) {
            clonkMessage(this, LauncherRes::str(50603)); // "Extract failure."
            return;
        }
        if (type == T_Player) {
            // random portrait out of Graphics.c4g (0x42a470)
            C4Group gfx(work.filePath("Graphics.c4g").toStdString());
            std::vector<std::string> portraits;
            for (const auto &e : gfx.getEntries())
                if (QString::fromStdString(e.name).startsWith("Portrait", Qt::CaseInsensitive) &&
                    QString::fromStdString(e.name).endsWith(".bmp", Qt::CaseInsensitive))
                    portraits.push_back(e.name);
            if (!portraits.empty()) {
                const std::string pick = "Portrait" + std::to_string(QRandomGenerator::global()->bounded(static_cast<int>(portraits.size())) + 1) + ".bmp";
                auto data = gfx.getFile(pick);
                if (!data.empty())
                    GroupEdit::writeFile(ItemPath(work.filePath(name)), "Portrait.bmp", data);
            }
        }
    }

    // move it to the target (0x42f1d0 without confirmation); organizing needs registration except for players
    ItemPath created(work.filePath(name));
    QStandardItem *new_item = nullptr;
    if (target) {
        if (!dropItem(created, target_item, true, false, type == T_Player)) {
            GroupEdit::remove(created);
            return;
        }
        new_item = findTreeItem(target->path.child(name));
    } else {
        if (!isRegistered() && type != T_Player) {
            clonkMessage(this, LauncherRes::str(51120));
            GroupEdit::remove(created);
            return;
        }
        refreshTree();
        new_item = findTreeItem(created);
    }
    if (new_item) {
        tree->setCurrentIndex(new_item->index());
        startRename();
    }
}

// ExplorerTree 0x42c080: edit the label of the selected item
void ClonkLauncher::startRename() {
    QStandardItem *it = selectedTreeItem();
    if (!it || !canModify(it, false))
        return;
    renaming_item = it->data(ROLE_ID).toInt();
    updating_tree = true;
    it->setEditable(true);
    updating_tree = false;
    tree->edit(it->index());
}

// ExplorerTree::OnEndLabelEdit (0x42c3e0)
void ClonkLauncher::finishRename(QStandardItem *it) {
    ExplorerItem *e = explorerItem(it);
    renaming_item = 0;
    updating_tree = true;
    it->setEditable(false);
    updating_tree = false;
    if (!e)
        return;
    const QString new_title = it->text().trimmed();
    if (new_title.isEmpty() || new_title == e->title) {
        updating_tree = true;
        it->setText(e->title);
        updating_tree = false;
        return;
    }
    logStatus(fmt(51809, {e->title})); // "Renaming %s..."

    // player view: file name = new title + extension, developer view: the new file name as typed
    QString new_file = new_title;
    const QString ext = explorerType(e->type).ext;
    if (!developer_view && !ext.isEmpty())
        new_file = QString("%1.%2").arg(new_title, ext);

    ItemPath path = e->path;
    bool ok = true;
    if (new_file != e->filename) {
        if (QFileInfo::exists(path.parent().normalized().disk + "/" + new_file) && path.subs.isEmpty())
            ok = false;
        else
            ok = GroupEdit::rename(path, new_file);
        if (ok) {
            path = path.parent().child(new_file);
            // activation lists refer to file names
            for (const char *key : {"Explorer\\Participants", "Explorer\\Definitions"}) {
                QStringList list = QString::fromStdString(get_cfg(key, "")).split(';', Qt::SkipEmptyParts);
                for (QString &m : list)
                    if (m.compare(e->filename, Qt::CaseInsensitive) == 0)
                        m = new_file;
                set_cfg(key, list.join(';').toStdString());
            }
            saveConfig();
        }
    }
    // player view: the title inside the item follows the new name
    if (ok && !developer_view) {
        C4Group grp;
        if (GroupEdit::open(path, grp)) {
            switch (e->type) {
            case T_Player: {
                C4TextDoc p(grp.getFile("Player.txt"));
                p.set("Player", "Name", new_title);
                ok = GroupEdit::writeFile(path, "Player.txt", p.toBytes());
                break;
            }
            case T_ScenarioFolder:
            case T_Group:
                if (grp.hasEntry("Title.txt"))
                    ok = GroupEdit::remove(path.child("Title.txt"));
                break;
            case T_Scenario: {
                C4TextDoc s(grp.getFile("Scenario.txt"));
                s.set("Head", "Title", new_title);
                ok = GroupEdit::writeFile(path, "Scenario.txt", s.toBytes());
                if (ok && grp.hasEntry("Title.txt"))
                    ok = GroupEdit::remove(path.child("Title.txt"));
                break;
            }
            case T_Definition: {
                if (grp.hasEntry("DefCore.txt")) {
                    C4TextDoc d(grp.getFile("DefCore.txt"));
                    d.set("DefCore", "Name", new_title);
                    ok = GroupEdit::writeFile(path, "DefCore.txt", d.toBytes());
                }
                if (ok && grp.hasEntry("Names.txt"))
                    ok = GroupEdit::remove(path.child("Names.txt"));
                break;
            }
            default:
                break;
            }
        }
    }
    if (!ok)
        clonkMessage(this, LauncherRes::str(50612)); // "Rename failure."
    refreshTree();
    if (QStandardItem *n = findTreeItem(path))
        tree->setCurrentIndex(n->index());
}

// ExplorerTree 0x42c0f0
void ClonkLauncher::onDeleteClicked() {
    QStandardItem *it = selectedTreeItem();
    ExplorerItem *e = explorerItem(it);
    if (!e || !canModify(it, true))
        return;
    const bool original = isOriginalItem(it);
    if (!clonkMessage(this, fmt(original ? 51102 : 51127, {e->title}), MsgButtons::OKCancel))
        return;
    logStatus(fmt(51801, {e->title})); // "Deleting %s..."
    if (e->activatable && e->activated)
        setActivated(it, false);
    const ItemPath parent = e->path.parent();
    if (!GroupEdit::remove(e->path)) {
        clonkMessage(this, LauncherRes::str(50601)); // "Delete failure."
        return;
    }
    refreshTree();
    if (QStandardItem *p = findTreeItem(parent))
        tree->setCurrentIndex(p->index());
}

// ExplorerTree 0x42fcf0: may items of type src be dropped into target type (player view)?
static bool dropAllowed(int src, int target, bool *as_portrait) {
    *as_portrait = false;
    switch (src) {
    case T_Directory:
        return target == -1 || target == T_Directory;
    case T_Player:
        return target == -1 || target == T_Directory;
    case T_ScenarioFolder:
    case T_Scenario:
        return target == -1 || target == T_Directory || target == T_ScenarioFolder;
    case T_Bitmap:
        *as_portrait = target == T_Player || target == T_CrewMember;
        return *as_portrait;
    case T_Definition:
    case T_ObjectFolder:
        return target == -1 || target == T_Directory || target == T_Definition || target == T_Scenario ||
               target == T_ScenarioFolder;
    case T_CrewMember:
        return target == T_Player;
    default:
        return false;
    }
}

// ExplorerTree 0x42f1d0: copy (or move) an item into a group of the tree
bool ClonkLauncher::dropItem(const ItemPath &src, QStandardItem *target_item, bool move, bool confirm, bool is_new) {
    if (!is_new && !isRegistered()) {
        clonkMessage(this, LauncherRes::str(51120)); // organizing is for registered users
        return false;
    }
    ExplorerItem *target = explorerItem(target_item);
    if (target && !explorerType(target->type).group && target->type != T_Directory) {
        clonkMessage(this, fmt(51137, {target->title})); // "Cannot copy any item to %s."
        return false;
    }
    if (target_item && !canModify(target_item, false))
        return false;

    ExplorerItem src_item;
    initExplorerItem(src_item, src, explorerContext());
    const int src_type = explorerTypeFor(src.name(), GroupEdit::isDirectory(src));
    QString dst_name = src.name();
    bool as_portrait = false;
    if (!developer_view && target) {
        if (!dropAllowed(src_type, target->type, &as_portrait)) {
            clonkMessage(this, fmt(51142, {LauncherRes::str(explorerType(src_type).name_id),
                                           LauncherRes::str(explorerType(target->type).name_id)}));
            return false;
        }
        if (as_portrait)
            dst_name = "Portrait.bmp";
    }
    const QString src_title = src_item.title.isEmpty() ? src.name() : src_item.title;
    const QString dst_title = itemLocation(target_item);
    if (confirm) {
        const QString q = as_portrait ? fmt(51128, {src_title, dst_title, LauncherRes::str(51918)})
                                      : fmt(move ? 51129 : 51126, {src_title, dst_title});
        if (!clonkMessage(this, q, MsgButtons::OKCancel))
            return false;
    }
    logStatus(fmt(move ? 51806 : 51800, {src_title, dst_title})); // "Copying / Moving %s to %s..."

    const ItemPath dst_group = target ? target->path : ItemPath(planet_data_path);
    if (dst_group.child(dst_name) == src || dst_group.child(dst_name).normalized() == src.normalized()) {
        if (is_new)
            return true; // already in place (new item in the working directory)
        clonkMessage(this, LauncherRes::str(50610)); // "An object cannot be copied onto itself."
        return false;
    }
    if (!GroupEdit::copy(src, dst_group, dst_name)) {
        clonkMessage(this, LauncherRes::str(move ? 50607 : 50600));
        return false;
    }
    if (move && !GroupEdit::remove(src)) {
        clonkMessage(this, LauncherRes::str(50607));
        return false;
    }
    refreshTree();
    if (QStandardItem *n = findTreeItem(dst_group.child(dst_name)))
        tree->setCurrentIndex(n->index());
    return true;
}

void ClonkLauncher::onItemDropped(const QModelIndex &source, const QModelIndex &target, bool move) {
    ExplorerItem *s = explorerItem(source);
    if (!s)
        return;
    QStandardItem *src_item = tree_model->itemFromIndex(source);
    if (move && !canModify(src_item, true))
        return;
    dropItem(s->path, tree_model->itemFromIndex(target), move, true, false);
}

void ClonkLauncher::onFilesDropped(const QStringList &files, const QModelIndex &target) {
    for (const QString &f : files)
        if (!dropItem(ItemPath(f), tree_model->itemFromIndex(target), false, true, false))
            break;
}

// ExplorerTree 0x42fdc0
void ClonkLauncher::onTreeContextMenu(const QPoint &pos) {
    ExplorerItem *e = selectedExplorerItem();
    QMenu menu(this);
    menu.setFont(LauncherRes::sysFont());
    auto add = [&](int text_id, bool enabled, auto fn) {
        QAction *a = menu.addAction(LauncherRes::str(text_id));
        a->setEnabled(enabled);
        connect(a, &QAction::triggered, this, fn);
    };
    add(50023, true, [this]() { onNewClicked(); });
    add(e && e->activated ? 50009 : 50001, e && e->activatable && !e->locked, [this]() { onActivateClicked(); });
    add(50030, e != nullptr, [this]() { startRename(); });
    add(50012, e != nullptr, [this]() { onDeleteClicked(); });
    if (developer_view)
        add(50014, e != nullptr, [this]() { duplicateItem(); });
    menu.addSeparator();
    add(50029, e && e->has_properties, [this]() { showProps(); });
    if (e && e->type == T_Scenario)
        add(50028, true, [this]() { showScenarioPresets(); });
    if (developer_view) {
        // Pack / Unpack and Explode for groups on disk
        const bool is_group = e && (explorerType(e->type).group || e->type == T_Directory) && e->path.normalized().subs.isEmpty();
        const bool unpacked = e && GroupEdit::isDirectory(e->path);
        menu.addSeparator();
        add(unpacked ? 50027 : 50035, is_group && (unpacked || e->type != T_Directory),
            [this, unpacked]() { packItem(unpacked, false); });
        add(50017, is_group && !unpacked && e->type != T_Directory, [this]() { packItem(false, true); });
    }
    menu.addSeparator();
    const bool reference = isNetworkReference(e);
    if (reference)
        add(50022, true, [this]() { sendNetworkMessage(); }); // Message
    add(reference ? 50021 : 50033, e && e->type == T_Scenario, [this]() { launchGame(); });
    menu.exec(pos);
}

// ExplorerTree::OnKeyDown (0x42d4a0)
void ClonkLauncher::onTreeKey(int key) {
    switch (key) {
    case Qt::Key_Space:
        onActivateClicked();
        break;
    case Qt::Key_Insert:
        onNewClicked();
        break;
    case Qt::Key_Delete:
        onDeleteClicked();
        break;
    case Qt::Key_F2:
        startRename();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        // ExplorerDlg::OnOK (0x430910): edit editable items, start scenarios
        ExplorerItem *e = selectedExplorerItem();
        if (e && explorerType(e->type).editor)
            openInEditor();
        else if (e && e->type == T_Scenario)
            launchGame();
        break;
    }
    case Qt::Key_F5:
        refreshTree();
        break;
    case Qt::Key_F6:
        // switch player / developer view in developer mode (0x430960)
        if (developerMode()) {
            (developer_view ? radio_player : radio_developer)->setChecked(true);
        }
        break;
    default:
        break;
    }
}
