#pragma once

// Registration of the original Planet.exe: the player name / registration code check
// (C4Config::IsRegistered, 0x404e10), the main window title and Options > Registration...
//
// The code is stored in General\Name and General\Code (C4Config::General.Name / .Code), the same
// keys the engine reads. Codes of the previous version (General\UserName / RegCode, "old" key) are
// recognised by the registration dialog but not accepted.

#include <QString>

class QWidget;

// C4Config::Registered (0x404f80): validates the stored General\Name / General\Code
bool isRegistered();

// ExplorerDlg title (0x413e10): "Clonk Planet" or "Clonk Planet - NOT REGISTERED" (localised)
QString mainWindowTitle();

// Options > Registration... (ExplorerDlg::On4006Clicked). Stores and saves name/code when valid.
// Returns true if the registration was accepted (the caller then updates the window title and,
// like the original, refreshes the tree because the player name may have changed).
bool runRegistrationDialog(QWidget *parent);

// the algorithm
namespace Registration {

// C4Config::IsRegistered(szName, szCode, szSecKey, fOld) (0x404e10) with the key of this version
// (fOld = false) or of the previous version (fOld = true)
bool isValid(const QString &name, const QString &code, bool old_version = false);

} // namespace Registration
