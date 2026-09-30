#pragma once

// Items of the main window tree (the original's ExplorerTree, 0x42b000-0x432000).
//
// Every file or group entry becomes an ExplorerItem. Its type comes from the extension table of
// Planet.exe (0x47cbb0, 25 entries of name / extension / icons / flags / editor / "new" template),
// title, icon and flags from the file contents (Item::Init 0x4319a0 and 0x431ea0).

#include "GroupEdit.h"

#include <QPixmap>
#include <QString>
#include <vector>

enum ExplorerTypeId {
    T_Directory = 0,
    T_Player = 1,         // .c4p
    T_ScenarioFolder = 2, // .c4f
    T_Scenario = 3,       // .c4s
    T_Group = 4,          // .c4g
    T_Material = 5,       // .c4m
    T_Help = 6,           // .hlp
    T_Log = 7,            // .log
    T_Text = 8,           // .txt
    T_RichText = 9,       // .rtf
    T_Bitmap = 10,        // .bmp
    T_Script = 11,        // .c
    T_Definition = 12,    // .c4d (object definition or object folder)
    T_ObjectFolder = 13,  // .c4d, only used by "New"
    T_CrewMember = 14,    // .c4i
    T_Sound = 15,         // .wav
    T_Music = 16,         // .mid
    T_Video = 17,         // .avi
    T_Binary = 18,        // .c4b
    T_Executable = 19,    // .exe
    T_Engine = 20,        // .c4x
    T_Zip = 21,           // .zip
    T_Animation = 22,     // .c4v
    T_Hypertext = 23,     // .html
    T_Unknown = 24,
    T_Count = 25
};

struct ExplorerType {
    int name_id;      // string id of the type name ("Scenario folder")
    const char *ext;  // extension without dot
    int icon;         // index in the tree icon strip (bitmap 1015)
    int icon_open;
    bool group;       // can contain items (loaded when expanded)
    bool player_view; // shown in player view
    int editor;       // external editor kind in developer mode (0 none)
    int newable;      // 0: no, 1: developer mode only, 2: always (New dialog)
    int template_id;  // BINARY resource with the contents of a new item
};

const ExplorerType &explorerType(int type);
// type by file name (extension); directories are T_Directory
int explorerTypeFor(const QString &filename, bool is_directory);

struct ExplorerItem {
    int type = T_Unknown;
    int icon = 0;
    int icon_open = 0;
    QPixmap custom_icon; // Icon.bmp of the group (player view), magenta is transparent
    ItemPath path;
    QString filename;
    QString title; // player view text
    QString maker; // group header: author line
    bool original = false;
    int creation = 0;
    bool group_packed = false;

    bool activatable = false;    // checkbox (players and definition packs in the root folder)
    bool activated = false;
    bool locked = false;         // checkbox state forced by the selected scenario
    bool has_properties = false; // players, scenarios with properties access
    bool bold = false;
    bool expandable = false;     // group with (possibly) children

    // scenario
    bool scenario_access = false; // [Head] Access=1: scenario properties for unregistered users
    int scenario_icon = 0;

    // generated description (players: rank + name / statistics); empty: from the item's files
    QString info_title;
    QString info_text;
    // sort key for tutorials (icons 27-35 sort by icon), definitions by category
    int sort_icon = 0;
};

struct ExplorerContext {
    QString data_dir;         // planet data directory (root of the tree)
    bool developer_view = false;
    bool registered = false;
    QString language = "US";
    QStringList participants; // Explorer\Participants
    QStringList definitions;  // Explorer\Definitions
};

// Evaluates one item. Returns false if it is not shown in the current view.
bool initExplorerItem(ExplorerItem &item, const ItemPath &path, const ExplorerContext &ctx);

// Items of a group / directory (unsorted).
std::vector<ExplorerItem> listExplorerItems(const ItemPath &group, const ExplorerContext &ctx);

// ExplorerTree::FindInsertPos (0x42e130): index at which `item` is inserted among `siblings`
// (in their current order). Items are inserted one by one in group order, which gives the
// original's order: numbered tutorial icons (27-35) after smaller icons, everything else after
// smaller types and (case sensitive) smaller titles.
size_t explorerInsertPos(const std::vector<ExplorerItem> &siblings, const ExplorerItem &item);

// Sorts a list of items like inserting them one by one into the tree.
void sortExplorerItems(std::vector<ExplorerItem> &items);

// Description pane contents (ExplorerDlg::ShowItemInfo 0x4131d0, item loaders 0x431c50 / 0x432fa0)
struct ExplorerInfo {
    QString html;         // description (rich text)
    bool frontend_font = true; // the description uses the frontend font (not for plain text files)
    QPixmap picture;
    bool transparent = false;  // definition graphics: drawn centered with shadow instead of stretched
};
ExplorerInfo loadExplorerInfo(const ExplorerItem &item, const ExplorerContext &ctx);

// Title of a group in the current language (Title.txt / Names.txt / DefCore.txt / Player.txt)
QString groupTitle(const C4Group &grp, const QString &filename, const QString &language);

// SIsModule / SAddModule / SRemoveModule on ';' separated lists (case insensitive)
bool isModule(const QStringList &list, const QString &name);
