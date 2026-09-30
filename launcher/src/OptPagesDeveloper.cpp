// Options pages of the developer mode: Extern (PageEditor, IDD 3011) and Developer (PageDeveloper,
// IDD 3010)

#include "LauncherRes.h"
#include "OptPages.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>

// ============================================================================ PageEditor

namespace {
struct EditorRow {
    int edit, browse, label, label_text;
    const char *key, *def; // C4Config Explorer.Editor* (engine/src/C4Config.cpp)
};
const EditorRow EDITOR_ROWS[] = {
    {2129, 2047, 2358, 50339, "Explorer\\EditorText", "Notepad.exe"},
    {2126, 2044, 2340, 50292, "Explorer\\EditorRichText", "Wordpad.exe"},
    {2127, 2045, 2343, 50297, "Explorer\\EditorScript", "Notepad.exe"},
    {2124, 2042, 2268, 50113, "Explorer\\EditorBitmap", "MSPaint.exe"},
    {2128, 2046, 2354, 50327, "Explorer\\EditorSound", "MPlayer.exe"},
    {2125, 2043, 2325, 50240, "Explorer\\EditorMusic", "MPlayer.exe"},
    {2130, 2048, 2378, 50374, "Explorer\\EditorZip", "WinZip32.exe"},
};
constexpr int IDC_USESHELL = 2102;

// MFC filter "text|pattern|...||" -> Qt filter list
QString qtFilter(const QString &mfc) {
    const QStringList parts = mfc.split('|');
    QStringList filters;
    for (int i = 0; i + 1 < parts.size(); i += 2)
        if (!parts[i].isEmpty())
            filters << QString("%1 (%2)").arg(parts[i], QString(parts[i + 1]).replace(';', ' '));
#ifndef _WIN32
    filters << "* (*)"; // programs have no extension here
#endif
    return filters.join(";;");
}
} // namespace

OptEditorPage::OptEditorPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_EDITOR, parent) {
    // PageEditor::OnInitDialog: texts
    get<QCheckBox>(IDC_USESHELL)->setText(LauncherRes::str(50351));
    for (const auto &r : EDITOR_ROWS) {
        get<QLabel>(r.label)->setText(LauncherRes::str(r.label_text));
        get<QPushButton>(r.browse)->setText(LauncherRes::str(50004)); // "Browse..."
        // FUN_00410630 (ctor)
        get<QLineEdit>(r.edit)->setText(QString::fromStdString(cfg(r.key, r.def)));
        const int edit = r.edit;
        connect(get<QPushButton>(r.browse), &QPushButton::clicked, this, [this, edit]() { browse(edit); });
    }
    auto *shell = get<QCheckBox>(IDC_USESHELL);
    shell->setChecked(cfgInt("Explorer\\EditorUseShell", 1) != 0);
    // PageEditor::OnUseshellClicked
    connect(shell, &QCheckBox::clicked, this, [this, shell]() { enableEditors(!shell->isChecked()); });
    enableEditors(!shell->isChecked());
}

// FUN_00410b40
void OptEditorPage::enableEditors(bool enable) {
    for (const auto &r : EDITOR_ROWS)
        for (int id : {r.edit, r.browse, r.label})
            control(id)->setEnabled(enable);
}

// PageEditor::OnBrowseClicked..OnBrowseClicked_7
void OptEditorPage::browse(int edit_id) {
    const QString file = QFileDialog::getOpenFileName(this, QString(), QString(), qtFilter(LauncherRes::str(50701)));
    if (!file.isEmpty())
        get<QLineEdit>(edit_id)->setText(file);
}

// FUN_00410530
void OptEditorPage::apply() {
    for (const auto &r : EDITOR_ROWS)
        setCfg(r.key, get<QLineEdit>(r.edit)->text().toStdString());
    setCfgInt("Explorer\\EditorUseShell", get<QCheckBox>(IDC_USESHELL)->isChecked() ? 1 : 0);
}

// ============================================================================ PageDeveloper

namespace {
const std::pair<int, const char *> DEVELOPER_CHECKS[] = {
    {2079, "General\\DebugMode"},             // "Enable debug mode"
    {2103, "Graphics\\VerboseObjectLoading"}, // "Show object definition overload"
    {2096, "Developer\\SendDefReload"},       // "Send runtime object updates to engine"
    {2076, "Developer\\AutoEditScan"},        // "Background update edited objects"
};
}

OptDeveloperPage::OptDeveloperPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_DEVELOPER, parent) {
    // FUN_00420460 (ctor)
    for (const auto &c : DEVELOPER_CHECKS)
        get<QCheckBox>(c.first)->setChecked(cfgInt(c.second, 0) != 0);
}

// FUN_004204b0
void OptDeveloperPage::apply() {
    for (const auto &c : DEVELOPER_CHECKS)
        setCfgInt(c.second, get<QCheckBox>(c.first)->isChecked() ? 1 : 0);
}
