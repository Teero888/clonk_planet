#pragma once

// Reading and modifying files inside (nested) C4Groups.
//
// An ItemPath locates an item the way the launcher tree does: a file or directory on disk plus a
// chain of entry names inside packed groups, e.g. {"/data/Easy.c4f", {"Goldmine.c4s"}}.
// Directories on disk are treated like unpacked groups. Modifying an entry inside nested packed
// groups rewrites every enclosing group (preserving maker, original flag, entry order and times).

#include "C4Group.h"

#include <QString>
#include <QStringList>
#include <string>
#include <vector>

struct ItemPath {
    QString disk;     // file or directory on disk
    QStringList subs; // entries inside packed groups, outermost first

    ItemPath() = default;
    ItemPath(const QString &d, const QStringList &s = {}) : disk(d), subs(s) {}

    // Path of an entry inside this group
    ItemPath child(const QString &name) const;
    // Enclosing group (empty disk path for top level items)
    ItemPath parent() const;
    // File name of the item itself
    QString name() const;
    bool isEmpty() const { return disk.isEmpty(); }
    // Moves directory components of subs to disk (a group that is a directory on disk)
    ItemPath normalized() const;
    // Readable form for logs: disk/sub1/sub2
    QString toString() const;

    bool operator==(const ItemPath &o) const { return disk == o.disk && subs == o.subs; }
    bool operator!=(const ItemPath &o) const { return !(*this == o); }
};

namespace GroupEdit {

// Is the item a directory on disk (unpacked group or plain folder)?
bool isDirectory(const ItemPath &path);

// Loads the group at path. Returns false if it is no group.
bool open(const ItemPath &path, C4Group &out);

// Raw contents of a (non group) item, or the packed memory image of a group item.
bool readItem(const ItemPath &path, std::vector<uint8_t> &out);

// File of a group; empty vector if missing.
std::vector<uint8_t> readFile(const ItemPath &group, const std::string &name);

// Adds or replaces a file inside a group.
bool writeFile(const ItemPath &group, const std::string &name, const std::vector<uint8_t> &data);

// Removes an entry (file, group or directory).
bool remove(const ItemPath &path);

// Renames an entry within its group / directory.
bool rename(const ItemPath &path, const QString &new_name);

// Copies an item (with all contents) into a group or directory under the given name.
bool copy(const ItemPath &src, const ItemPath &dst_group, const QString &new_name);

// Creates an empty group (packed file or, inside a directory, a packed file too - like the original).
bool createGroup(const ItemPath &parent_group, const QString &name, const std::vector<uint8_t> &template_raw = {});

// Writes a modified group back to its location, rewriting all enclosing groups.
bool replaceGroup(const ItemPath &path, C4GroupWriter &writer);

// C4Group_SetMaker: like the engine's C4Group::Save, every group that is written gets this maker
// (the registered user name, else "Unregistered user") and loses the original flag.
// Empty (default): headers are kept as they are.
void setMaker(const QString &maker);
QString maker();

} // namespace GroupEdit
