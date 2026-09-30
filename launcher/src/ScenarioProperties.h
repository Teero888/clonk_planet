#pragma once

// Entry points of the scenario editing dialogs (Planet.exe ExplorerDlg::OnPropertiesClicked,
// 0x414150 -> 0x42e210, and the object definitions dialog).

#include "GroupEdit.h"

#include <QString>

class QWidget;

// Scenario Properties (IDD 3027). Returns true if the scenario was saved.
// title: scenario title for the caption ("Scenario Properties - <title>"); empty = the localised
// title of the scenario (Title.txt / [Head] Title) like the original.
bool editScenarioProperties(QWidget *parent, const ItemPath &scenario, const QString &title);

// Same; saved_to receives the path that was written. Scenarios inside original packs are saved as a
// copy in the game directory after asking (ScenarioPropertiesDlg::OnOK, result 0x3039 of the
// original) - the caller has to add that copy to the tree then.
bool editScenarioProperties(QWidget *parent, const ItemPath &scenario, const QString &title, ItemPath *saved_to);

// Object definitions of a scenario (IDD 3028, [Definitions] LocalOnly / Definition1..10).
// Returns true if saved.
bool editScenarioDefinitions(QWidget *parent, const ItemPath &scenario, const QString &title);

// Is the item part of an original pack (any enclosing group has the original flag, FUN_00430210)?
bool scenarioIsOriginal(const ItemPath &item);
