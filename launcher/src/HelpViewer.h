#pragma once

// Replacement of WinHelp for the help file of the original launcher (planet_data/Planet.hlp).
//
// The original calls CWinApp::WinHelp (vtable slot 0x98) with
//   HELP_FINDER (0x0b)          Help > Contents... (ExplorerDlg::On4001Clicked 0x414200) and the help
//                               button (id 9) of the quick start dialog: the "Help Topics" dialog
//   HELP_CONTEXTPOPUP (8)       WM_HELP of ExplorerDlg (ExplorerDlg::OnHelp 0x415570) and of the dialogs
//                               3009, 3023, 3027, 3028 (SharedDlg::Handler_423dd0) with the control id
//                               (HELPINFO::iCtrlId): the popup of that control
//   HELP_CONTEXT (1)            RegistrationDlg::OnHelpClicked (0x20bd6 = HID_BASE_RESOURCE + 3030) and
//                               MFC's F1 handling (HID_BASE_RESOURCE + IDD, e.g. 0x20bca = options page
//                               "Network" -> topic "Networking")
//
// Planet.hlp is stored converted to HTML in data/help/<lang>/ (topics as
// t<n>.html, contents.json from Planet.cnt, context.json with the [MAP] numbers of |CTXOMAP).

#include <QPoint>
#include <QString>

class QDialog;
class QWidget;

// Help > Contents...: the "Help Topics" dialog (Contents / Index tab)
void showHelpContents(QWidget *parent);

// WinHelp(context, HELP_CONTEXT): shows the topic of a help context id ([MAP] number) in the help
// window. Popup topics (the control help) are shown as popup at the mouse position. Unknown ids
// fall back to the "Help Topics" dialog.
void showHelpContext(QWidget *parent, int context);

// WinHelp(control id, HELP_CONTEXTPOPUP) of the WM_HELP handlers: the popup of a control at the
// given global position. Ids without help show the "Subject not available" popup of the help file.
void showHelpPopup(QWidget *parent, int context, const QPoint &global_pos);

// Context help of a dialog built from a template (controls named "ctrl_<id>" by DialogBuilder):
// F1 shows the popup of the focused control, the "What's This?" mode (title bar "?" button, needs
// Qt::WindowContextHelpButtonHint) the popup of the clicked control. Port of the WM_HELP handlers.
void installContextHelp(QWidget *dialog);
