// Round trip test for launcher/src/GroupEdit: edits inside nested packed groups must keep every
// other entry, the header (maker / original flag) and the entry order intact.
#include "../launcher/src/GroupEdit.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <cstdio>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                            \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static std::vector<std::string> names(const C4Group &g) {
    std::vector<std::string> n;
    for (const auto &e : g.getEntries())
        n.push_back(e.name);
    return n;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        std::printf("usage: %s <planet_data dir>\n", argv[0]);
        return 2;
    }
    QTemporaryDir tmp;
    QString easy = tmp.filePath("Easy.c4f");
    QFile::copy(QDir(argv[1]).filePath("Easy.c4f"), easy);

    C4Group before;
    CHECK(GroupEdit::open(ItemPath(easy), before));
    C4Group gold_before;
    CHECK(GroupEdit::open(ItemPath(easy, {"Goldmine.c4s"}), gold_before));

    // write into a nested group
    std::string text = "[Head]\nTitle=Test\n";
    std::vector<uint8_t> data(text.begin(), text.end());
    CHECK(GroupEdit::writeFile(ItemPath(easy, {"Goldmine.c4s"}), "Test.txt", data));

    C4Group after;
    CHECK(GroupEdit::open(ItemPath(easy), after));
    CHECK(names(after) == names(before));
    CHECK(after.getMaker() == before.getMaker());
    CHECK(after.isOriginal() == before.isOriginal());
    CHECK(after.isPacked());
    C4Group gold_after;
    CHECK(GroupEdit::open(ItemPath(easy, {"Goldmine.c4s"}), gold_after));
    CHECK(gold_after.getFile("Test.txt") == data);
    CHECK(gold_after.getFile("Scenario.txt") == gold_before.getFile("Scenario.txt"));
    CHECK(gold_after.getEntries().size() == gold_before.getEntries().size() + 1);
    CHECK(gold_after.getMaker() == gold_before.getMaker());
    // untouched sibling group byte identical
    CHECK(after.getFile("Melee.c4s") == before.getFile("Melee.c4s"));

    // rename and remove inside nested group
    CHECK(GroupEdit::rename(ItemPath(easy, {"Goldmine.c4s", "Test.txt"}), "Renamed.txt"));
    CHECK(GroupEdit::readFile(ItemPath(easy, {"Goldmine.c4s"}), "Renamed.txt") == data);
    CHECK(GroupEdit::readFile(ItemPath(easy, {"Goldmine.c4s"}), "Test.txt").empty());
    CHECK(GroupEdit::remove(ItemPath(easy, {"Goldmine.c4s", "Renamed.txt"})));
    C4Group gold_final;
    CHECK(GroupEdit::open(ItemPath(easy, {"Goldmine.c4s"}), gold_final));
    CHECK(names(gold_final) == names(gold_before));

    // copy a nested scenario out to a directory and into another group
    CHECK(GroupEdit::copy(ItemPath(easy, {"Goldmine.c4s"}), ItemPath(tmp.path()), "Copy.c4s"));
    C4Group copy;
    CHECK(GroupEdit::open(ItemPath(tmp.filePath("Copy.c4s")), copy));
    CHECK(copy.getFile("Scenario.txt") == gold_before.getFile("Scenario.txt"));
    CHECK(GroupEdit::copy(ItemPath(tmp.filePath("Copy.c4s")), ItemPath(easy), "Copy2.c4s"));
    CHECK(GroupEdit::readFile(ItemPath(easy, {"Copy2.c4s"}), "Scenario.txt") == gold_before.getFile("Scenario.txt"));

    // new group from scratch inside a nested group
    CHECK(GroupEdit::createGroup(ItemPath(easy, {"Goldmine.c4s"}), "Sub.c4f"));
    C4Group sub;
    CHECK(GroupEdit::open(ItemPath(easy, {"Goldmine.c4s", "Sub.c4f"}), sub));

    // a directory copied into a group becomes a child group
    QDir(tmp.path()).mkpath("Folder/Inner");
    QFile inner(tmp.filePath("Folder/Inner/Note.txt"));
    CHECK(inner.open(QIODevice::WriteOnly));
    inner.write("hello");
    inner.close();
    CHECK(GroupEdit::copy(ItemPath(tmp.filePath("Folder")), ItemPath(easy, {"Goldmine.c4s"}), "Folder"));
    C4Group gold_dir;
    CHECK(GroupEdit::open(ItemPath(easy, {"Goldmine.c4s"}), gold_dir));
    CHECK(gold_dir.findEntry("Folder") && gold_dir.findEntry("Folder")->is_group);
    CHECK(GroupEdit::readFile(ItemPath(easy, {"Goldmine.c4s", "Folder", "Inner"}), "Note.txt") ==
          std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o'}));

    std::printf(failures ? "%d failures\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
