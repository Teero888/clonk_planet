// Developer mode tools of the tree: external editors (0x410d10 / 0x411180, rescan 0x410df0 on
// activation and every 2 seconds with Developer\AutoEditScan), Duplicate (0x430b60),
// Pack / Unpack (0x4312d0 / 0x4314e0) and Explode (0x431280).

#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "GroupEdit.h"
#include "LauncherRes.h"

#include <QDesktopServices>
#include <QEvent>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

namespace {

QString fmt1(int id, const QString &a) {
    QString s = LauncherRes::str(id);
    const int i = s.indexOf("%s");
    if (i >= 0)
        s.replace(i, 2, a);
    return s;
}

// external editor kind (type table) -> config key of the Extern page
const char *editorKey(int kind) {
    switch (kind) {
    case 1: return "Explorer\\EditorText";
    case 2: return "Explorer\\EditorBitmap";
    case 3: return "Explorer\\EditorSound";
    case 4: return "Explorer\\EditorRichText";
    case 5: return "Explorer\\EditorMusic";
    case 6: return "Explorer\\EditorScript";
    case 7: return "Explorer\\EditorZip";
    case 8: return "Explorer\\EditorDefinition";
    case 10: return "Explorer\\EditorHtml";
    default: return nullptr;
    }
}

} // namespace

// ExplorerTree 0x42d7b0: open the selected item in its external editor
bool ClonkLauncher::openInEditor() {
    QStandardItem *it = selectedTreeItem();
    ExplorerItem *e = explorerItem(it);
    if (!e)
        return false;
    const int kind = explorerType(e->type).editor;
    const char *key = editorKey(kind);
    if (!key)
        return false;
    logStatus(LauncherRes::str(51802)); // "Preparing edit..."

    // already being edited: just bring the editor up again (0x411700 / 0x411740)
    QString file;
    for (const EditedItem &ed : edited_items)
        if (ed.path == e->path)
            file = ed.temp_file;
    if (file.isEmpty()) {
        const ItemPath path = e->path.normalized();
        if (path.subs.isEmpty()) {
            file = path.disk; // plain file on disk: edit in place
        } else {
            // extract from the group into a temporary directory
            const QString dir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("ClonkPlanetEdit");
            QDir().mkpath(dir);
            file = QDir(dir).filePath(e->filename);
            std::vector<uint8_t> data;
            if (!GroupEdit::readItem(path, data)) {
                clonkMessage(this, LauncherRes::str(50602)); // "Edit failure."
                return false;
            }
            QFile f(file);
            if (!f.open(QIODevice::WriteOnly) ||
                f.write(reinterpret_cast<const char *>(data.data()), data.size()) != static_cast<qint64>(data.size())) {
                clonkMessage(this, LauncherRes::str(50602));
                return false;
            }
        }
        edited_items.push_back({e->path, file, QFileInfo(file).lastModified(), isOriginalItem(it)});
    }

    bool ok;
    const std::string editor = get_cfg(key, "");
    if (get_cfg("Explorer\\EditorUseShell", "1") == "1" || editor.empty())
        ok = QDesktopServices::openUrl(QUrl::fromLocalFile(file));
    else
        ok = QProcess::startDetached(QString::fromStdString(editor), {file});
    if (!ok)
        clonkMessage(this, LauncherRes::str(50602));
    logStatus(QString());
    return ok;
}

// 0x410df0: write modified files back into their groups
void ClonkLauncher::scanEditedItems() {
    bool changed = false;
    for (EditedItem &ed : edited_items) {
        const QDateTime mtime = QFileInfo(ed.temp_file).lastModified();
        if (!mtime.isValid() || mtime == ed.mtime)
            continue;
        ed.mtime = mtime;
        const ItemPath path = ed.path.normalized();
        if (path.subs.isEmpty())
            continue; // edited in place
        QFile f(ed.temp_file);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        const QByteArray b = f.readAll();
        const std::vector<uint8_t> data(b.begin(), b.end());
        if (ed.original) {
            // "... may not be saved at it's former location. Save modification to working directory?"
            if (!clonkMessage(this, fmt1(51130, ed.path.name()), MsgButtons::OKCancel))
                continue;
            GroupEdit::copy(ItemPath(ed.temp_file), ItemPath(planet_data_path), ed.path.name());
        } else {
            logStatus(fmt1(51803, ed.path.name())); // "Loading %s..."
            if (!GroupEdit::writeFile(path.parent(), path.name().toStdString(), data))
                clonkMessage(this, LauncherRes::str(50606)); // "File modification failure."
        }
        changed = true;
    }
    if (changed)
        refreshTree();
}

// 0x430b60: copy with the first free number before the extension, into the same group
void ClonkLauncher::duplicateItem() {
    QStandardItem *it = selectedTreeItem();
    ExplorerItem *e = explorerItem(it);
    if (!e)
        return;
    const ItemPath parent = e->path.parent();
    C4Group grp;
    const bool is_dir = GroupEdit::isDirectory(parent);
    if (!is_dir && !GroupEdit::open(parent, grp))
        return;
    auto exists = [&](const QString &n) {
        return is_dir ? QFileInfo::exists(QDir(parent.normalized().disk).filePath(n)) : grp.hasEntry(n.toStdString());
    };
    const int dot = e->filename.lastIndexOf('.');
    QString name;
    for (int n = 2;; ++n) {
        name = dot < 0 ? e->filename + QString::number(n)
                       : e->filename.left(dot) + QString::number(n) + e->filename.mid(dot);
        if (!exists(name))
            break;
    }
    logStatus(LauncherRes::str(51800).replace("%s", e->title)); // "Copying %s to %s..."
    if (!GroupEdit::copy(e->path, parent, name)) {
        clonkMessage(this, LauncherRes::str(50606));
        return;
    }
    refreshTree();
    if (QStandardItem *n = findTreeItem(parent.child(name)))
        tree->setCurrentIndex(n->index());
}

namespace {

// C4Group_UnpackDirectory: a packed group file becomes a directory with its entries
bool unpackTo(const C4Group &grp, const QString &dir, bool recursive) {
    if (!QDir().mkpath(dir))
        return false;
    for (const auto &e : grp.getEntries()) {
        const QString target = QDir(dir).filePath(QString::fromStdString(e.name));
        auto data = grp.getFile(e.name);
        if (recursive && e.is_group) {
            C4Group child;
            if (child.loadFromMemory(data)) {
                if (!unpackTo(child, target, true))
                    return false;
                continue;
            }
        }
        if (e.is_group) {
            // child groups stay packed files
            C4Group child;
            if (child.loadFromMemory(data)) {
                C4GroupWriter w;
                w.addFromGroup(child);
                w.setHeaderFrom(child);
                if (!w.writeToFile(target.toStdString(), true))
                    return false;
                continue;
            }
        }
        QFile f(target);
        if (!f.open(QIODevice::WriteOnly) || f.write(reinterpret_cast<const char *>(data.data()), data.size()) != static_cast<qint64>(data.size()))
            return false;
    }
    return true;
}

} // namespace

void ClonkLauncher::packItem(bool pack, bool recursive) {
    QStandardItem *it = selectedTreeItem();
    ExplorerItem *e = explorerItem(it);
    if (!e || !canModify(it, false))
        return;
    const ItemPath path = e->path.normalized();
    if (!path.subs.isEmpty())
        return; // only groups on disk
    const QString tmp = path.disk + ".tmp";
    bool ok = false;
    if (pack) {
        if (!GroupEdit::isDirectory(path))
            return;
        logStatus(fmt1(51807, e->title)); // "Packing %s..."
        C4Group grp;
        if (grp.loadFromDirectory(path.disk.toStdString())) {
            C4GroupWriter w;
            w.addFromGroup(grp);
            ok = w.writeToFile(tmp.toStdString(), true) && QDir(path.disk).removeRecursively() && QFile::rename(tmp, path.disk);
        }
    } else {
        if (GroupEdit::isDirectory(path))
            return;
        logStatus(fmt1(51811, e->title)); // "Unpacking %s..."
        C4Group grp;
        if (grp.loadFromFile(path.disk.toStdString())) {
            ok = unpackTo(grp, tmp, recursive) && QFile::remove(path.disk) && QFile::rename(tmp, path.disk);
            if (!ok)
                QDir(tmp).removeRecursively();
        }
    }
    if (!ok)
        clonkMessage(this, LauncherRes::str(pack ? 50609 : 50605));
    refreshTree();
    if (QStandardItem *n = findTreeItem(path))
        tree->setCurrentIndex(n->index());
}

void ClonkLauncher::changeEvent(QEvent *event) {
    // ExplorerDlg::OnActivateApp (0x414ef0)
    if (event->type() == QEvent::ActivationChange && isActiveWindow() && !edited_items.empty())
        scanEditedItems();
    QMainWindow::changeEvent(event);
}
