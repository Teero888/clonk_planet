#include "GroupEdit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <cstring>

namespace {

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool readDiskFile(const QString &path, std::vector<uint8_t> &out) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QByteArray b = f.readAll();
    out.assign(b.begin(), b.end());
    return true;
}

bool writeDiskFile(const QString &path, const std::vector<uint8_t> &data) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return f.write(reinterpret_cast<const char *>(data.data()), data.size()) == static_cast<qint64>(data.size());
}

// Unpacked C4Group memory image (child groups are stored like this inside their parent)?
bool isRawGroup(const std::vector<uint8_t> &data) {
    if (data.size() < 204)
        return false;
    std::vector<uint8_t> header(data.begin(), data.begin() + 204);
    C4Group::unscramble(header.data(), header.size());
    return std::memcmp(header.data(), "RedWolf Design GrpFolder", 24) == 0;
}

// Copy of a group with one entry replaced (data == nullptr removes it, new_name renames it).
void copyGroupWith(const C4Group &grp, C4GroupWriter &writer, const std::string &name,
                   const std::vector<uint8_t> *data, const std::string &new_name = {}) {
    writer.setHeaderFrom(grp);
    bool found = false;
    for (const auto &e : grp.getEntries()) {
        C4GroupWriter::WriteEntry w;
        w.name = e.name;
        w.packed = e.packed;
        w.child_group = e.is_group;
        w.time = e.time;
        if (lower(e.name) == lower(name)) {
            found = true;
            if (!data && new_name.empty())
                continue; // removed
            if (!new_name.empty())
                w.name = new_name;
            w.data = data ? *data : grp.getFile(e.name);
            if (data || !new_name.empty())
                w.child_group = C4GroupWriter::isGroupName(w.name) || isRawGroup(w.data);
            if (data)
                w.time = static_cast<uint32_t>(time(nullptr));
        } else {
            w.data = grp.getFile(e.name);
        }
        writer.addEntry(w);
    }
    if (!found && data) {
        C4GroupWriter::WriteEntry w;
        w.name = new_name.empty() ? name : new_name;
        w.data = *data;
        w.child_group = C4GroupWriter::isGroupName(w.name) || isRawGroup(w.data);
        w.time = static_cast<uint32_t>(time(nullptr));
        writer.addEntry(w);
    }
}

// Modifies the entry `name` of the group at `group` (which is inside packed groups).
bool modifyEntry(const ItemPath &group_in, const std::string &name, const std::vector<uint8_t> *data,
                 const std::string &new_name = {}) {
    ItemPath group = group_in.normalized();
    if (group.subs.isEmpty() && GroupEdit::isDirectory(group)) {
        // entry of a directory on disk
        QString target = QDir(group.disk).filePath(QString::fromStdString(name));
        if (!new_name.empty())
            return QFile::rename(target, QDir(group.disk).filePath(QString::fromStdString(new_name)));
        if (!data) {
            QFileInfo fi(target);
            if (fi.isDir())
                return QDir(target).removeRecursively();
            return QFile::remove(target);
        }
        return writeDiskFile(target, *data);
    }

    C4Group grp;
    if (!GroupEdit::open(group, grp))
        return false;
    C4GroupWriter writer;
    copyGroupWith(grp, writer, name, data, new_name);
    return GroupEdit::replaceGroup(group, writer);
}

} // namespace

ItemPath ItemPath::child(const QString &name) const {
    ItemPath n = normalized();
    if (n.subs.isEmpty() && QFileInfo(n.disk).isDir())
        return ItemPath(QDir(n.disk).filePath(name));
    ItemPath c = n;
    c.subs << name;
    return c;
}

ItemPath ItemPath::parent() const {
    ItemPath p = *this;
    if (!p.subs.isEmpty()) {
        p.subs.removeLast();
        return p;
    }
    QFileInfo fi(disk);
    return ItemPath(fi.absolutePath());
}

QString ItemPath::name() const {
    if (!subs.isEmpty())
        return subs.last();
    return QFileInfo(disk).fileName();
}

ItemPath ItemPath::normalized() const {
    ItemPath p = *this;
    while (!p.subs.isEmpty() && QFileInfo(p.disk).isDir()) {
        p.disk = QDir(p.disk).filePath(p.subs.first());
        p.subs.removeFirst();
    }
    return p;
}

QString ItemPath::toString() const {
    QString s = disk;
    for (const QString &sub : subs)
        s += "/" + sub;
    return s;
}

namespace GroupEdit {

bool isDirectory(const ItemPath &path) {
    ItemPath p = path.normalized();
    return p.subs.isEmpty() && QFileInfo(p.disk).isDir();
}

bool open(const ItemPath &path_in, C4Group &out) {
    ItemPath path = path_in.normalized();
    C4Group grp;
    if (!grp.loadFromFile(path.disk.toStdString()))
        return false;
    for (const QString &sub : path.subs) {
        auto raw = grp.getFile(sub.toStdString());
        if (raw.empty())
            return false;
        C4Group child;
        if (!child.loadFromMemory(raw))
            return false;
        grp = child;
    }
    out = grp;
    return true;
}

bool readItem(const ItemPath &path_in, std::vector<uint8_t> &out) {
    ItemPath path = path_in.normalized();
    if (path.subs.isEmpty())
        return readDiskFile(path.disk, out);
    C4Group grp;
    if (!open(path.parent(), grp))
        return false;
    const C4GroupEntry *e = grp.findEntry(path.subs.last().toStdString());
    if (!e)
        return false;
    out = grp.getFile(e->name);
    return true;
}

std::vector<uint8_t> readFile(const ItemPath &group, const std::string &name) {
    std::vector<uint8_t> out;
    readItem(group.child(QString::fromStdString(name)), out);
    return out;
}

bool writeFile(const ItemPath &group, const std::string &name, const std::vector<uint8_t> &data) {
    return modifyEntry(group, name, &data);
}

bool remove(const ItemPath &path_in) {
    ItemPath path = path_in.normalized();
    if (path.subs.isEmpty()) {
        QFileInfo fi(path.disk);
        if (fi.isDir())
            return QDir(path.disk).removeRecursively();
        return QFile::remove(path.disk);
    }
    return modifyEntry(path.parent(), path.subs.last().toStdString(), nullptr);
}

bool rename(const ItemPath &path_in, const QString &new_name) {
    ItemPath path = path_in.normalized();
    if (path.subs.isEmpty()) {
        QFileInfo fi(path.disk);
        return QFile::rename(path.disk, QDir(fi.absolutePath()).filePath(new_name));
    }
    return modifyEntry(path.parent(), path.subs.last().toStdString(), nullptr, new_name.toStdString());
}

bool copy(const ItemPath &src_in, const ItemPath &dst_group, const QString &new_name) {
    ItemPath src = src_in.normalized();
    std::vector<uint8_t> data;
    if (isDirectory(src)) {
        // pack the directory into a group
        C4Group grp;
        if (!grp.loadFromDirectory(src.disk.toStdString()))
            return false;
        data = grp.getRawData();
    } else {
        if (!readItem(src, data))
            return false;
        // a packed root group on disk is gzip compressed; groups inside groups are stored plain
        if (src.subs.isEmpty() && data.size() > 2 &&
            ((data[0] == 0x1e && data[1] == 0x8c) || (data[0] == 0x1f && data[1] == 0x8b))) {
            C4Group grp;
            if (grp.loadFromMemory(data))
                data = grp.getRawData();
        }
    }
    ItemPath dst = dst_group.normalized();
    if (dst.subs.isEmpty() && isDirectory(dst)) {
        // the copy becomes a packed file on disk
        QString target = QDir(dst.disk).filePath(new_name);
        if (C4GroupWriter::isGroupName(new_name.toStdString())) {
            C4Group grp;
            if (grp.loadFromMemory(data)) {
                C4GroupWriter w;
                w.addFromGroup(grp);
                w.setHeaderFrom(grp);
                return w.writeToFile(target.toStdString(), true);
            }
        }
        return writeDiskFile(target, data);
    }
    return writeFile(dst, new_name.toStdString(), data);
}

bool createGroup(const ItemPath &parent_group, const QString &name, const std::vector<uint8_t> &template_raw) {
    C4GroupWriter w;
    C4Group tmpl;
    if (!template_raw.empty() && tmpl.loadFromMemory(template_raw)) {
        w.addFromGroup(tmpl);
    }
    ItemPath dst = parent_group.normalized();
    if (dst.subs.isEmpty() && isDirectory(dst))
        return w.writeToFile(QDir(dst.disk).filePath(name).toStdString(), true);
    return writeFile(dst, name.toStdString(), w.makeMemoryBlob());
}

static QString group_maker;

void setMaker(const QString &m) {
    group_maker = m;
}

QString maker() {
    return group_maker;
}

bool replaceGroup(const ItemPath &path_in, C4GroupWriter &writer) {
    ItemPath path = path_in.normalized();
    // C4Group::Save: maker of the saving user, original flag only kept by MakeOriginal
    if (!group_maker.isEmpty()) {
        writer.setMaker(group_maker.toStdString());
        writer.setOriginal(false);
    }
    if (path.subs.isEmpty()) {
        // root group file on disk: keep its compression
        C4Group old;
        bool compress = true;
        if (old.loadFromFile(path.disk.toStdString()))
            compress = old.isPacked();
        QString tmp = path.disk + ".tmp";
        if (!writer.writeToFile(tmp.toStdString(), compress)) {
            QFile::remove(tmp);
            return false;
        }
        QFile::remove(path.disk);
        return QFile::rename(tmp, path.disk);
    }
    // nested: the new group image replaces the entry in its parent
    std::vector<uint8_t> raw = writer.makeMemoryBlob();
    return modifyEntry(path.parent(), path.subs.last().toStdString(), &raw);
}

} // namespace GroupEdit
