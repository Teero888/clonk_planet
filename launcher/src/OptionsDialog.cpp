#include "OptionsDialog.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "OptPages.h"
#include "Win3DWidgets.h"

#include <QBitmap>

namespace {
// property sheet layout (comctl32 PropertySheet with MS Sans Serif 8, measured on the originals)
constexpr int SHEET_MARGIN_X = 6;   // tab control left / right
constexpr int SHEET_MARGIN_TOP = 7; // tab control top
constexpr int BUTTON_GAP_Y = 6;     // tab control bottom to the buttons
constexpr int BUTTON_W = 75, BUTTON_H = 23, BUTTON_SPACING = 6, SHEET_MARGIN_BOTTOM = 7;

// image list of the sheet: bitmap 1037, 16 px, mask color 0xff00ff (FUN_0041fc60)
QIcon tabIcon(int index) {
    static QPixmap strip;
    if (strip.isNull()) {
        strip = LauncherRes::bitmap(1037);
        if (!strip.isNull())
            strip.setMask(strip.createMaskFromColor(QColor(255, 0, 255)));
    }
    if (strip.isNull())
        return {};
    return QIcon(strip.copy(index * 16, 0, 16, 16));
}
} // namespace

OptionsDialog::OptionsDialog(ClonkLauncher *parent, bool show_network) : QDialog(parent), launcher(parent) {
    setWindowTitle(LauncherRes::str(50520)); // "Options"
    setFont(LauncherRes::sysFont());
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xec, 0xe9, 0xd8));
    setPalette(pal);
    setWindowFlag(Qt::WindowContextHelpButtonHint, true); // WS_EX_CONTEXTHELP (FUN_0041ff00)

    tab_widget = new Win3DTabWidget(this);
    tab_widget->setFont(LauncherRes::sysFont());

    // FUN_0041ff00: pages, captions and icons
    struct PageInfo {
        OptPage *page;
        int caption, icon;
    };
    const bool developer = launcher && launcher->developerMode(); // app +0x4024 (Developer.Active)
    std::vector<PageInfo> infos = {
        {new OptProgramPage(tab_widget), 50523, 6},  // "Program"
        {new OptGraphicsPage(tab_widget), 50510, 2}, // "Graphics"
        {new OptSoundPage(tab_widget), 50526, 5},    // "Sound"
        {new OptKeyboardPage(tab_widget), 50512, 10}, // "Keyboard"
        {new OptGamepadPage(tab_widget), 50509, 12},  // "Game Pad"
        {new OptNetworkPage(tab_widget), 50518, 13},  // "Network"
    };
    if (developer) {
        infos.push_back({new OptEditorPage(tab_widget), 50505, 14});    // "Extern"
        infos.push_back({new OptDeveloperPage(tab_widget), 50504, 14}); // "Developer"
    }

    // the sheet is sized for the largest page template
    QSize page_size(0, 0);
    for (const auto &info : infos) {
        page_size = page_size.expandedTo(LauncherRes::dialogSize(info.page->idd()));
        tab_widget->addTab(info.page, LauncherRes::str(info.caption), tabIcon(info.icon));
        pages_.push_back(info.page);
    }
    const QSize tab_size = tab_widget->sizeForPageSize(page_size);
    tab_widget->setGeometry(SHEET_MARGIN_X, SHEET_MARGIN_TOP, tab_size.width(), tab_size.height());

    // OK / Cancel right aligned below the tab control (no Apply: PSH_NOAPPLYNOW, no Help)
    const int width = tab_size.width() + 2 * SHEET_MARGIN_X;
    const int buttons_y = SHEET_MARGIN_TOP + tab_size.height() + BUTTON_GAP_Y;
    auto *btn_cancel = new Win3DButton(LauncherRes::str(50006), this);
    btn_cancel->setFont(LauncherRes::sysFont());
    btn_cancel->setGeometry(width - SHEET_MARGIN_X - BUTTON_W, buttons_y, BUTTON_W, BUTTON_H);
    auto *btn_ok = new Win3DButton(LauncherRes::str(50026), this);
    btn_ok->setFont(LauncherRes::sysFont());
    btn_ok->setGeometry(btn_cancel->x() - BUTTON_SPACING - BUTTON_W, buttons_y, BUTTON_W, BUTTON_H);
    btn_ok->setDefault(true);
    connect(btn_ok, &QPushButton::clicked, this, &OptionsDialog::onOK);
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);
    setFixedSize(width, buttons_y + BUTTON_H + SHEET_MARGIN_BOTTOM);

    // SetActivePage(DAT_0048fea0 ? 5 : 0)
    tab_widget->setActiveIndex(show_network ? 5 : 0);
}

void OptionsDialog::setActiveTab(int index) { tab_widget->setActiveIndex(index); }


OptPage *OptionsDialog::page(int index) const {
    return index >= 0 && index < static_cast<int>(pages_.size()) ? pages_[index] : nullptr;
}

void OptionsDialog::onOK() {
    // CPropertySheet OK: OnOK of the pages (PageKeyboard stores the shown block), then the values of
    // all pages go into the config (FUN_00420030) and the config is saved (ExplorerDlg::On4005Clicked)
    for (OptPage *p : pages_)
        p->onOK();
    for (OptPage *p : pages_)
        p->apply();
    if (launcher)
        launcher->saveConfig();
    accept();
}
