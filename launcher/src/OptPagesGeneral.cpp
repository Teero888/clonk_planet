// Options pages Program, Graphics and Sound

#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "LicenseDlg.h"
#include "OptPages.h"
#include "Registration.h"
#include "Win3DWidgets.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFontDatabase>
#include <QProcess>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>

namespace {

// CBS_DROPDOWN | CBS_SORT combo box that enumerates the installed fonts when it is dropped down
// with an empty list (PageProgram::On2105 CBN_DROPDOWN: EnumFontFamilies)
class OptFontCombo : public QComboBox {
public:
    using QComboBox::QComboBox;
    void showPopup() override {
        if (count() == 0) {
            const QString text = currentText();
            QStringList families = QFontDatabase::families();
            families.sort(Qt::CaseInsensitive);
            addItems(families);
            setCurrentIndex(findText(text));
            setEditText(text);
        }
        QComboBox::showPopup();
    }
};

bool sameText(const std::string &a, const char *b) {
    return QString::fromStdString(a).compare(QString::fromLatin1(b), Qt::CaseInsensitive) == 0;
}

} // namespace

// ============================================================================ PageProgram

OptProgramPage::OptProgramPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_PROGRAM, parent) {
    // FUN_00424410: texts
    german_ = get<QRadioButton>(2207);
    english_ = get<QRadioButton>(2208);
    german_->setText(LauncherRes::str(50185));
    english_->setText(LauncherRes::str(50173));
    get<QCheckBox>(2080)->setText(LauncherRes::str(50160));
    get<QCheckBox>(2098)->setText(LauncherRes::str(50319));

    // FUN_00424530 (ctor): language radio index 0 = "DE", 1 = "US", none otherwise
    const std::string lang = cfg("General\\Language", "");
    if (sameText(lang, "DE"))
        german_->setChecked(true);
    else if (sameText(lang, "US"))
        english_->setChecked(true);

    // font: DDX_CBString(FEFontName), the list is filled on drop down
    auto *combo = new OptFontCombo(this);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    replaceControl(2105, combo);
    font_ = combo;
    font_->setEditText(QString::fromStdString(cfg("General\\FEFontName", "Comic Sans MS")));

    // DDX_Text + DDV_MinMaxInt(8, 16), UDM_SETRANGE(8, 16)
    fe_size_ = get<QSpinBox>(2155);
    rx_size_ = get<QSpinBox>(2156);
    fe_size_->setRange(8, 16);
    rx_size_->setRange(8, 16);
    fe_size_->setValue(cfgInt("General\\FEFontSize", 9));
    rx_size_->setValue(cfgInt("General\\RXFontSize", 10));

    quick_start_ = get<QCheckBox>(2098);
    quick_start_->setChecked(cfgInt("Explorer\\ShowQuickStart", 1) != 0);
    developer_ = get<QCheckBox>(2080);
    developer_->setChecked(cfgInt("Developer\\Active", 0) != 0);
    connect(developer_, &QCheckBox::clicked, this, [this]() { onDeveloperActiveClicked(); });

    // PageProgram::OnInitDialog: the developer mode is only available to registered players
    if (!isRegistered()) {
        control(2324)->hide();
        developer_->hide();
    }
}

// PageProgram::OnDeveloperActiveClicked
void OptProgramPage::onDeveloperActiveClicked() {
    if (!developer_->isChecked())
        return;
    LicenseDlg license(this);
    if (license.exec() == QDialog::Accepted) {
        QString password;
        if (clonkPrompt(this, LauncherRes::str(51105), password, true) && password == LauncherRes::str(51601))
            return; // "Enter password:" / "Siedlerclonk"
    }
    developer_->setChecked(false);
}

// FUN_004245f0
void OptProgramPage::apply() {
    const char *lang = german_->isChecked() ? "DE" : english_->isChecked() ? "US" : nullptr;
    if (lang) {
        setCfg("General\\Language", lang);
        if (ClonkLauncher *l = ClonkLauncher::instance())
            l->setLanguage(lang); // saveConfig() writes the launcher's language
    }
    const std::string font = font_->currentText().toStdString();
    if (!font.empty()) {
        setCfg("General\\FEFontName", font);
        setCfg("General\\RXFontName", font);
    }
    setCfgInt("General\\FEFontSize", fe_size_->value());
    setCfgInt("General\\RXFontSize", rx_size_->value());
    setCfgInt("Developer\\Active", developer_->isChecked() ? 1 : 0);
    setCfgInt("Explorer\\ShowQuickStart", quick_start_->isChecked() ? 1 : 0);
}

// ============================================================================ PageGraphics

namespace {
const int GRAPHICS_CHECKS[][2] = {
    // control, index into GRAPHICS_KEYS
    {2077, 0}, {2091, 1}, {2094, 2}, {2095, 3}, {2104, 4}, {2101, 5},
};
const char *GRAPHICS_KEYS[] = {"Graphics\\ColorAnimation", "Graphics\\ShowCommands",
                               "Graphics\\ShowPlayerInfoAlways", "Graphics\\ShowPortraits",
                               "Graphics\\SplitscreenDividers", "Graphics\\ShowStartupMessages"};
} // namespace

OptGraphicsPage::OptGraphicsPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_GRAPHICS, parent) {
    // FUN_00418dc0 (ctor) / PageGraphics::OnInitDialog
    const int res = cfgInt("Graphics\\Resolution", 1) - 1; // DDX_Radio: 640x480, 800x600, 1024x768
    const int radios[] = {2215, 2216, 2217};
    for (int i = 0; i < 3; ++i)
        get<QRadioButton>(radios[i])->setChecked(i == res);
    for (const auto &c : GRAPHICS_CHECKS)
        get<QCheckBox>(c[0])->setChecked(cfgInt(GRAPHICS_KEYS[c[1]], 1) != 0);
    // "DirectDraw software emulation" = !DDrawAccel
    get<QCheckBox>(2078)->setChecked(cfgInt("Graphics\\DDrawAccel", 1) == 0);
    // CSliderCtrl::SetRange(0, 300), TBM_SETPOS(SmokeLevel)
    auto *smoke = get<QSlider>(2247);
    smoke->setRange(0, 300);
    smoke->setValue(cfgInt("Graphics\\SmokeLevel", 200));
}

// FUN_00418d10
void OptGraphicsPage::apply() {
    const int radios[] = {2215, 2216, 2217};
    for (int i = 0; i < 3; ++i)
        if (get<QRadioButton>(radios[i])->isChecked())
            setCfgInt("Graphics\\Resolution", i + 1);
    for (const auto &c : GRAPHICS_CHECKS)
        setCfgInt(GRAPHICS_KEYS[c[1]], get<QCheckBox>(c[0])->isChecked() ? 1 : 0);
    setCfgInt("Graphics\\SmokeLevel", get<QSlider>(2247)->value());
    setCfgInt("Graphics\\DDrawAccel", get<QCheckBox>(2078)->isChecked() ? 0 : 1);
}

// ============================================================================ PageSound

namespace {
const std::pair<int, const char *> SOUND_CHECKS[] = {
    {2084, "Sound\\RXSound"}, {2085, "Sound\\RXSoundLoops"}, {2086, "Sound\\RXMusic"},
    {2082, "Sound\\FESamples"}, {2083, "Sound\\FEMusic"},
};
}

OptSoundPage::OptSoundPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_SOUND, parent) {
    // FUN_00429b30 / PageSound::OnInitDialog
    for (const auto &c : SOUND_CHECKS)
        get<QCheckBox>(c.first)->setChecked(cfgInt(c.second, 1) != 0);

    // PageSound::OnSoundfxClicked: sound loops only with sound effects
    auto *sfx = get<QCheckBox>(2084);
    auto update_loops = [this, sfx]() { control(2085)->setEnabled(sfx->isChecked()); };
    connect(sfx, &QCheckBox::clicked, this, update_loops);
    update_loops();

    // PageSound::OnVolumeClicked runs sndvol32.exe: open the platform's mixer instead
    connect(get<QPushButton>(2073), &QPushButton::clicked, this, []() {
#if defined(_WIN32)
        QProcess::startDetached("sndvol.exe", {});
#elif defined(__APPLE__)
        QProcess::startDetached("open", {"x-apple.systempreferences:com.apple.preference.sound"});
#else
        if (!QProcess::startDetached("pavucontrol", {}))
            QProcess::startDetached("pwvucontrol", {});
#endif
    });
}

// FUN_00429a70
void OptSoundPage::apply() {
    for (const auto &c : SOUND_CHECKS)
        setCfgInt(c.second, get<QCheckBox>(c.first)->isChecked() ? 1 : 0);
}
