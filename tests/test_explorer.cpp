// Checks the tree order of the launcher's item model against the original's screenshots.
#include "../launcher/src/ExplorerModel.h"
#include "../launcher/src/LauncherRes.h"

#include <QApplication>
#include <QDir>
#include <cstdio>

static int failures = 0;

static QStringList titles(const ItemPath &group, const ExplorerContext &ctx) {
    std::vector<ExplorerItem> items = listExplorerItems(group, ctx);
    sortExplorerItems(items);
    QStringList out;
    for (const auto &i : items)
        out << i.title;
    return out;
}

static void expect(const char *what, const QStringList &got, const QStringList &want) {
    if (got != want) {
        std::printf("FAIL %s\n  got:  %s\n  want: %s\n", what, qPrintable(got.join(", ")), qPrintable(want.join(", ")));
        ++failures;
    }
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc < 3) {
        std::printf("usage: %s <planet_data dir> <launcher data dir>\n", argv[0]);
        return 2;
    }
    const QString data = QDir(argv[1]).absolutePath();
    LauncherRes::load(argv[2]);
    ExplorerContext ctx;
    ctx.data_dir = data;

    // MainPage.png: players, scenario folders, definition packs
    QStringList root = titles(ItemPath(data), ctx);
    root.removeAll("Savegames"); // not on the original CD
    expect("root", root, {"Joki", "Twonky", "Easy Worlds", "Far Worlds", "Hazard", "Knights", "Missions", "More Maps",
                          "Tutorial", "Worlds", "Hazard", "Knights", "Objects"});
    // MenuScenarioCastleDunkelfelsGreyedOutObjects.png: Easy Worlds sorted by title
    expect("easy", titles(ItemPath(data + "/Easy.c4f"), ctx),
           {"Gold Mine", "Minor Melee", "Monsterkill", "Settlement", "Shark Lake", "The Castle"});
    // tutorials: numbered icons keep their order
    expect("keyboard", titles(ItemPath(data + "/Tutorial.c4f", {"Keyboard.c4f"}), ctx),
           {"A Clonk", "More Clonks", "Air Travel", "Production Line", "Gold Mine", "Underground", "Acid Lake",
            "Wipf Rescue", "Arctic Ocean", "Volcanic"});
    expect("mouse", titles(ItemPath(data + "/Tutorial.c4f", {"Mouse.c4f"}), ctx),
           {"Settlement", "Goldmine", "Production", "Objects"});
    // MenuObjectHazard.png
    expect("hazard defs", titles(ItemPath(data + "/Hazard.c4d"), ctx),
           {"Aliens", "Control points", "Guard drone", "HazardClonk", "Items", "Vehicles", "Weapons"});

    std::printf(failures ? "%d failures\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
