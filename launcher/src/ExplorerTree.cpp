// Tree of the main window: ExplorerTree (0x42b000-0x432000) and the ExplorerDlg handlers that work
// on the selected item (0x412a20-0x416000).

#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "C4TextDoc.h"
#include "GroupEdit.h"
#include "LauncherRes.h"
#include "Utils.h"
#include "NetStartDlg.h"
#include "NetworkBrowser.h"
#include "ScenarioProperties.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <cmath>
#include <QPlainTextEdit>
#include <iostream>

namespace {

constexpr int ROLE_ID = Qt::UserRole + 1;     // key into ClonkLauncher::explorer_items
constexpr int ROLE_LOCKED = Qt::UserRole + 3; // checkbox locked by the selected scenario
constexpr int ROLE_LOADED = Qt::UserRole + 4; // children loaded
constexpr int ROLE_DUMMY = Qt::UserRole + 5;  // "Empty" placeholder child of unloaded groups

QStringList splitModules(const std::string &list) {
    return QString::fromStdString(list).split(';', Qt::SkipEmptyParts);
}

std::string joinModules(const QStringList &list) {
    return list.join(';').toStdString();
}

} // namespace

ExplorerContext ClonkLauncher::explorerContext() const {
    ExplorerContext ctx;
    ctx.data_dir = planet_data_path;
    ctx.developer_view = developer_view;
    ctx.registered = isRegistered();
    ctx.language = QString::fromStdString(language);
    ctx.participants = splitModules(get_cfg("Explorer\\Participants", ""));
    ctx.definitions = splitModules(get_cfg("Explorer\\Definitions", ""));
    return ctx;
}

ExplorerItem *ClonkLauncher::explorerItem(QStandardItem *item) {
    if (!item)
        return nullptr;
    auto it = explorer_items.find(item->data(ROLE_ID).toInt());
    return it == explorer_items.end() ? nullptr : &it->second;
}

ExplorerItem *ClonkLauncher::explorerItem(const QModelIndex &index) {
    return explorerItem(tree_model->itemFromIndex(index));
}

ExplorerItem *ClonkLauncher::selectedExplorerItem() {
    const QModelIndexList sel = tree->selectionModel()->selectedIndexes();
    return sel.isEmpty() ? nullptr : explorerItem(sel.first());
}

QList<ItemPath> ClonkLauncher::activeDefinitions() const {
    // activated definition packs in the data directory (Explorer\Definitions)
    QList<ItemPath> out;
    for (const QString &m : splitModules(get_cfg("Explorer\\Definitions", ""))) {
        const QString path = QDir(planet_data_path).filePath(m);
        if (QFileInfo::exists(path))
            out << ItemPath(path);
    }
    return out;
}

QStandardItem *ClonkLauncher::makeTreeItem(const ExplorerItem &e) {
    const int id = next_explorer_item++;
    explorer_items[id] = e;
    auto *item = new QStandardItem(e.title);
    item->setEditable(false);
    item->setData(id, ROLE_ID);
    item->setIcon(e.custom_icon.isNull() ? get_atlas_icon(e.icon) : QIcon(e.custom_icon));
    if (e.bold) {
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    if (e.activatable) {
        item->setCheckable(true);
        item->setCheckState(e.activated ? Qt::Checked : Qt::Unchecked);
    }
    if (e.expandable) {
        // ExplorerTree::InsertItem: groups get an "Empty" child until they are expanded
        auto *dummy = new QStandardItem("Empty");
        dummy->setData(true, ROLE_DUMMY);
        item->appendRow(dummy);
    } else {
        item->setData(true, ROLE_LOADED);
    }
    return item;
}

void ClonkLauncher::loadChildren(QStandardItem *parent) {
    if (!parent || parent->data(ROLE_LOADED).toBool())
        return;
    ExplorerItem *e = explorerItem(parent);
    if (!e)
        return;
    updating_tree = true;
    parent->setData(true, ROLE_LOADED);
    parent->removeRows(0, parent->rowCount());
    std::vector<ExplorerItem> children = listExplorerItems(e->path, explorerContext());
    sortExplorerItems(children);
    for (const ExplorerItem &c : children)
        parent->appendRow(makeTreeItem(c));
    updating_tree = false;
}

void ClonkLauncher::refresh_resources() {
    refreshTree();
}

void ClonkLauncher::refreshTree() {
    // remember selection and expanded groups
    ItemPath selected;
    if (ExplorerItem *s = selectedExplorerItem())
        selected = s->path;
    QList<ItemPath> expanded;
    std::function<void(QStandardItem *)> collect = [&](QStandardItem *p) {
        for (int r = 0; r < p->rowCount(); ++r) {
            QStandardItem *c = p->child(r);
            if (tree->isExpanded(c->index()))
                if (ExplorerItem *e = explorerItem(c))
                    expanded << e->path;
            collect(c);
        }
    };
    if (tree_model)
        collect(tree_model->invisibleRootItem());

    updating_tree = true;
    tree_model->clear();
    explorer_items.clear();
    // developer view: system font, 16 pixel rows, lines and buttons; player view: frontend font
    tree->setShowLines(developer_view);
    tree->setFont(developer_view ? LauncherRes::sysFont() : LauncherRes::feFont());
    tree->setStyleSheet(QString("QTreeView { background-color: white; border: none; selection-background-color: transparent; }"
                                "QTreeView::item { padding: 0px; height: %1px; }"
                                "QTreeView::branch { image: none; }").arg(developer_view ? 16 : 20));
    tree->setItemsExpandable(true);

    std::vector<ExplorerItem> roots = listExplorerItems(ItemPath(planet_data_path), explorerContext());
    sortExplorerItems(roots);
    for (const ExplorerItem &e : roots)
        tree_model->appendRow(makeTreeItem(e));
    updating_tree = false;

    for (const ItemPath &p : expanded)
        if (QStandardItem *it = findTreeItem(p))
            tree->expand(it->index());
    if (!selected.isEmpty()) {
        if (QStandardItem *it = findTreeItem(selected)) {
            tree->setCurrentIndex(it->index());
            return;
        }
    }
    showItemInfo(nullptr);
}

QStandardItem *ClonkLauncher::findTreeItem(const ItemPath &path) {
    // walk down from the root, loading groups on the way
    std::function<QStandardItem *(QStandardItem *)> find = [&](QStandardItem *p) -> QStandardItem * {
        for (int r = 0; r < p->rowCount(); ++r) {
            QStandardItem *c = p->child(r);
            ExplorerItem *e = explorerItem(c);
            if (!e)
                continue;
            if (e->path == path)
                return c;
            // is path inside this item?
            const QString mine = e->path.toString() + "/";
            if (path.toString().startsWith(mine)) {
                loadChildren(c);
                return find(c);
            }
        }
        return nullptr;
    };
    return find(tree_model->invisibleRootItem());
}

void ClonkLauncher::onTreeExpanded(const QModelIndex &index) {
    QStandardItem *item = tree_model->itemFromIndex(index);
    loadChildren(item);
    if (ExplorerItem *e = explorerItem(item))
        if (e->custom_icon.isNull())
            item->setIcon(get_atlas_icon(e->icon_open));
}

void ClonkLauncher::onTreeCollapsed(const QModelIndex &index) {
    QStandardItem *item = tree_model->itemFromIndex(index);
    if (ExplorerItem *e = explorerItem(item))
        if (e->custom_icon.isNull())
            item->setIcon(get_atlas_icon(e->icon));
}

void ClonkLauncher::onTreeSelection(const QItemSelection &, const QItemSelection &) {
    ExplorerItem *item = selectedExplorerItem();
    updatePackageLocks(item);
    showItemInfo(item);
}

// ExplorerDlg::ShowItemInfo (0x4131d0)
void ClonkLauncher::showItemInfo(ExplorerItem *item) {
    // author line: group maker, or Author.txt of RedWolf Design / Treffpunkt Clonk groups
    QString author;
    if (item && !item->maker.isEmpty()) {
        author = item->maker;
        if (isModule(LauncherRes::str(51600).split(';'), item->maker)) {
            C4Group grp;
            if (GroupEdit::open(item->path, grp)) {
                const auto a = grp.getFile("Author.txt");
                if (!a.empty())
                    author = QString::fromLatin1(reinterpret_cast<const char *>(a.data()), static_cast<int>(a.size())).trimmed();
            }
        }
    }
    author_label->setText(author.isEmpty() ? QString() : LauncherRes::str(50109).replace("%s", author));

    ExplorerInfo info;
    if (item)
        info = loadExplorerInfo(*item, explorerContext());

    // picture: definition graphics are drawn centered with a shadow, other pictures stretched
    const bool has_picture = !info.picture.isNull();
    preview_frame->setVisible(has_picture);
    if (has_picture) {
        QPixmap pix = info.picture;
        if (info.transparent) {
            pix = applyClonkTransparency(pix);
            QSize area = preview->size() - QSize(40, 20);
            pix = pix.scaled(area, Qt::KeepAspectRatio, Qt::FastTransformation);
            auto *effect = new QGraphicsDropShadowEffect(this);
            effect->setBlurRadius(0);
            effect->setColor(QColor(0x80, 0x80, 0x80));
            effect->setOffset(3, 3);
            preview->setGraphicsEffect(effect);
        } else {
            pix = pix.scaled(preview->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
            preview->setGraphicsEffect(nullptr);
        }
        preview->setPixmap(pix);
    } else {
        preview->setGraphicsEffect(nullptr);
        preview->setPixmap(QPixmap());
    }

    // description: below the picture, or the whole area if there is none (controls 2191 / 2189)
    desc_full_area = !has_picture;
    layoutControls();
    QFont f = info.frontend_font ? LauncherRes::feFont() : LauncherRes::sysFont();
    desc->document()->setDefaultFont(f);
    desc->setHtml(info.html);

    // buttons
    btn_new->setEnabled(true);
    if (!item) {
        btn_activate->setEnabled(false);
        btn_rename->setEnabled(false);
        btn_delete->setEnabled(false);
        btn_props->setEnabled(false);
        btn_activate->setText(LauncherRes::str(50001));
        btn_start->setText(LauncherRes::str(50033));
        return;
    }
    btn_activate->setText(LauncherRes::str(item->activated ? 50009 : 50001)); // Activate / Deactivate
    btn_activate->setEnabled(item->activatable && !item->locked);
    btn_rename->setEnabled(true);
    btn_delete->setEnabled(true);
    btn_props->setEnabled(item->has_properties);
    btn_start->setText(LauncherRes::str(isNetworkReference(item) ? 50021 : 50033)); // Join / Start
}

// Scenario [Definitions]: packs required by the selected scenario are locked on, the others off
void ClonkLauncher::updatePackageLocks(ExplorerItem *selected) {
    bool lock = false;
    QStringList required;
    if (selected && selected->type == T_Scenario) {
        C4Group grp;
        if (GroupEdit::open(selected->path, grp)) {
            C4TextDoc s(grp.getFile("Scenario.txt"));
            if (s.getInt("Definitions", "LocalOnly")) {
                lock = true;
            }
            for (int i = 1; i <= 10; ++i) {
                const QString d = s.get("Definitions", QString("Definition%1").arg(i));
                if (!d.isEmpty()) {
                    required << QFileInfo(QString(d).replace('\\', '/')).fileName();
                    lock = true;
                }
            }
        }
    }
    updating_tree = true;
    QStandardItem *root = tree_model->invisibleRootItem();
    for (int r = 0; r < root->rowCount(); ++r) {
        QStandardItem *it = root->child(r);
        ExplorerItem *e = explorerItem(it);
        if (!e || e->type != T_Definition || !e->activatable)
            continue;
        e->locked = lock;
        it->setData(lock, ROLE_LOCKED);
        if (lock)
            it->setCheckState(isModule(required, e->filename) ? Qt::Checked : Qt::Unchecked);
        else
            it->setCheckState(e->activated ? Qt::Checked : Qt::Unchecked);
    }
    updating_tree = false;
    tree->viewport()->update();
}

// ExplorerDlg::OnActivateClicked (0x413710)
void ClonkLauncher::onActivateClicked() {
    const QModelIndexList sel = tree->selectionModel()->selectedIndexes();
    if (sel.isEmpty())
        return;
    QStandardItem *it = tree_model->itemFromIndex(sel.first());
    ExplorerItem *e = explorerItem(it);
    if (!e || !e->activatable || e->locked)
        return;
    setActivated(it, !e->activated);
}

void ClonkLauncher::setActivated(QStandardItem *tree_item, bool activated) {
    ExplorerItem *e = explorerItem(tree_item);
    if (!e)
        return;
    e->activated = activated;
    const std::string key = e->type == T_Player ? "Explorer\\Participants" : "Explorer\\Definitions";
    QStringList list = splitModules(get_cfg(key, ""));
    list.erase(std::remove_if(list.begin(), list.end(),
                              [&](const QString &m) { return m.compare(e->filename, Qt::CaseInsensitive) == 0; }),
               list.end());
    if (activated)
        list << e->filename;
    set_cfg(key, joinModules(list));
    saveConfig();
    updateCheckState(tree_item);
    if (selectedExplorerItem() == e)
        btn_activate->setText(LauncherRes::str(activated ? 50009 : 50001));
}

void ClonkLauncher::updateCheckState(QStandardItem *tree_item) {
    ExplorerItem *e = explorerItem(tree_item);
    if (!e || !e->activatable)
        return;
    updating_tree = true;
    tree_item->setCheckState(e->activated ? Qt::Checked : Qt::Unchecked);
    updating_tree = false;
}

void ClonkLauncher::onViewModeChanged(bool developer) {
    if (developer_view == developer)
        return;
    developer_view = developer;
    set_cfg("Explorer\\Mode", developer ? "1" : "0");
    saveConfig();
    applyViewMode();
    refreshTree();
}

// ExplorerDlg start validation (0x4152c0)
bool ClonkLauncher::validateStart(ExplorerItem *scenario) {
    if (!scenario || scenario->type != T_Scenario) {
        clonkMessage(this, LauncherRes::str(51136));
        return false;
    }
    C4Group grp;
    if (!GroupEdit::open(scenario->path, grp))
        return false;
    C4TextDoc s(grp.getFile("Scenario.txt"));

    if (!developer_view && !isNetworkReference(scenario)) {
        // number of activated players
        int players = 0;
        for (const QString &m : splitModules(get_cfg("Explorer\\Participants", "")))
            if (QFileInfo::exists(QDir(planet_data_path).filePath(m)))
                ++players;
        if (players < 1) {
            clonkMessage(this, LauncherRes::str(51138).replace("%i", "1"));
            return false;
        }
        const int max_players = s.getInt("Head", "MaxPlayer", 0);
        if (max_players > 0 && players > max_players) {
            clonkMessage(this, LauncherRes::str(51141).replace("%i", QString::number(max_players)));
            return false;
        }
    }

    // mission access password gained by previous missions
    const QString mission = s.get("Head", "MissionAccess");
    if (!mission.isEmpty() && !isModule(splitModules(get_cfg("General\\MissionAccess", "")), mission)) {
        clonkMessage(this, LauncherRes::str(51117));
        return false;
    }

    // unregistered players can only start scenarios with [Head] Access
    if (!isRegistered() && !scenario->scenario_access) {
        clonkMessage(this, LauncherRes::str(51121));
        return false;
    }

    // required object definitions must exist
    if (!s.getInt("Definitions", "LocalOnly")) {
        QStringList missing;
        for (int i = 1; i <= 10; ++i) {
            QString d = s.get("Definitions", QString("Definition%1").arg(i));
            if (d.isEmpty())
                continue;
            d.replace('\\', '/');
            if (!QFileInfo::exists(QDir(planet_data_path).filePath(d)))
                missing << d;
        }
        if (!missing.isEmpty()) {
            clonkMessage(this, LauncherRes::str(51123).replace("%s", missing.join("\n")));
            return false;
        }
    }
    return true;
}

void ClonkLauncher::startScenario(const ItemPath &scenario) {
    if (QStandardItem *it = findTreeItem(scenario)) {
        tree->setCurrentIndex(it->index());
        launchGame();
    }
}

// ExplorerDlg::OnStartClicked (0x413720)
void ClonkLauncher::launchGame() {
    ExplorerItem *scenario = selectedExplorerItem();
    if (!validateStart(scenario))
        return;

    // network: joining a reference, or the start dialog of a network game
    QStringList extra;
    if (isNetworkReference(scenario)) {
        NetReference ref;
        NetworkBrowser::readReference(scenario->path, ref);
        if (const int msg = NetworkBrowser::joinCheck(ref, developer_view)) {
            clonkMessage(this, LauncherRes::str(msg));
            return;
        }
        extra = NetworkBrowser::engineArgs();
    } else if (get_cfg("Network\\Active", "0") == "1") {
        if (!runNetStartDialog(this, extra))
            return;
    }
    saveConfig();

    QString clonk_bin = executablePath("clonk");
    if (!QFile::exists(clonk_bin)) {
        clonkMessage(this, clonk_bin);
        return;
    }

    auto relative = [&](const ItemPath &p) {
        QString rel = QDir(planet_data_path).relativeFilePath(p.disk);
        for (const QString &sub : p.subs)
            rel += "/" + sub;
        return rel;
    };

    // scenario, activated players, activated (or scenario forced) definitions
    QStringList args;
    args << relative(scenario->path);
    for (const QString &m : splitModules(get_cfg("Explorer\\Participants", "")))
        if (QFileInfo::exists(QDir(planet_data_path).filePath(m)))
            args << m;
    QStandardItem *root = tree_model->invisibleRootItem();
    for (int r = 0; r < root->rowCount(); ++r) {
        ExplorerItem *e = explorerItem(root->child(r));
        if (e && e->type == T_Definition && e->activatable && root->child(r)->checkState() == Qt::Checked)
            args << relative(e->path);
    }

    args << extra;

    std::cout << "Launching Game: " << clonk_bin.toStdString();
    for (const auto &arg : args)
        std::cout << " " << arg.toStdString();
    std::cout << std::endl;

    stop_music();
    if (net)
        net->pause();
    auto *game_process = new QProcess(this);
    game_process->setWorkingDirectory(planet_data_path);
    hide();
    connect(game_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ClonkLauncher::onGameFinished);
    game_process->start(clonk_bin, args);
}

void ClonkLauncher::onGameFinished(int exitCode, QProcess::ExitStatus) {
    if (auto *p = qobject_cast<QProcess *>(sender()))
        p->deleteLater();
    show();
    if (net)
        net->resume();
    // exit code 1: errors during the round, show Clonk4.log (0x414b30)
    if (exitCode == 1)
        showRoundLog();
    evaluatePlayers();
    // player statistics etc. changed
    refreshTree();
    start_music();
}

// C4RankSystem with the launcher's player ranks (0x411d92: PlayerRanks, default names 51401, base 200)
QString ClonkLauncher::playerRankName(int rank) {
    if (rank < 0)
        return {};
    const QStringList defaults = LauncherRes::str(51401).split('|');
    // Init: rank names that are not in the registry yet get the default names
    for (int i = 0; i < defaults.size(); ++i) {
        const std::string key = QString("PlayerRanks\\Rank%1").arg(i + 1, 3, 10, QChar('0')).toStdString();
        if (get_cfg(key, "").empty())
            set_cfg(key, defaults[i].toStdString());
    }
    return QString::fromStdString(get_cfg(QString("PlayerRanks\\Rank%1").arg(rank + 1, 3, 10, QChar('0')).toStdString(), ""));
}

// after a round: promotion of the participants (0x424b40) with EvaluationDlg (IDD 3024)
void ClonkLauncher::evaluatePlayers() {
    for (const QString &m : splitModules(get_cfg("Explorer\\Participants", ""))) {
        const ItemPath path(QDir(planet_data_path).filePath(m));
        C4Group grp;
        if (!GroupEdit::open(path, grp))
            continue;
        C4TextDoc p(grp.getFile("Player.txt"));
        const int rank = p.getInt("Player", "Rank");
        const int score = p.getInt("Player", "Score");
        const QString next_name = playerRankName(rank + 1);
        // C4RankSystem::Experience: rank^1.5 * base
        if (next_name.isEmpty() || static_cast<int>(std::pow(rank + 1, 1.5) * 200) > score)
            continue;
        ClonkDialog dlg(LauncherRes::IDD_EVALUATION, this);
        dlg.setWindowTitle(LauncherRes::str(50516));
        QString text = LauncherRes::str(51125);
        text.replace(text.indexOf("%s"), 2, p.get("Player", "Name"));
        text.replace(text.indexOf("%i"), 2, QString::number(score));
        text.replace(text.indexOf("%s"), 2, next_name);
        dlg.get<QLabel>(2358)->setText(text);
        dlg.get<QLabel>(2358)->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        auto *comment = dlg.get<QLineEdit>(2110);
        comment->setMaxLength(100);
        comment->setText(p.get("Player", "Comment"));
        if (dlg.exec() != QDialog::Accepted)
            continue;
        p.setInt("Player", "Rank", rank + 1);
        p.set("Player", "RankName", next_name);
        p.set("Player", "Comment", comment->text());
        GroupEdit::writeFile(path, "Player.txt", p.toBytes());
    }
}

void ClonkLauncher::showRoundLog() {
    QFile f(QDir(planet_data_path).filePath("Clonk4.log"));
    if (!f.open(QIODevice::ReadOnly))
        return;
    const QString log = QString::fromLatin1(f.readAll());
    // LogDlg::OnInitDialog
    ClonkDialog dlg(LauncherRes::IDD_LOG, this);
    dlg.setWindowTitle(LauncherRes::str(50515)); // "Error"
    if (auto *text = dlg.get<QPlainTextEdit>(2145))
        text->setPlainText(log);
    dlg.exec();
}

void ClonkLauncher::editScenario(ExplorerItem *scenario) {
    // original packs are saved as a copy in the working directory, which is then selected
    ItemPath saved_to = scenario->path;
    if (editScenarioProperties(this, scenario->path, scenario->title, &saved_to)) {
        refreshTree();
        if (QStandardItem *it = findTreeItem(saved_to))
            tree->setCurrentIndex(it->index());
    }
}

void ClonkLauncher::showScenarioPresets() {
    ExplorerItem *e = selectedExplorerItem();
    if (e && e->type == T_Scenario && editScenarioDefinitions(this, e->path, e->title))
        refreshTree();
}
