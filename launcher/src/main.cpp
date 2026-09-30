#include "XPStyle.h"
#include "LauncherCompat.h"
#include <QApplication>
#include "ClonkLauncher.h"
#include "SplashWindow.h"
#include "QuickStartDlg.h"
#include "LauncherRes.h"
#include <QDir>
#include <QUrl>
#include <QLoggingCategory>
#include <QTimer>
#include <QFont>
#include <QFontDatabase>
#include <QFile>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // Fusion with the XP scrollbars of the original's screenshots
    QApplication::setStyle(new XPStyle());
    // XP menus
    app.setStyleSheet(
        "QMenuBar { background: #ece9d8; color: black; }"
        "QMenuBar::item { background: transparent; padding: 3px 6px 3px 6px; }"
        "QMenuBar::item:selected, QMenuBar::item:pressed { background: #316ac5; color: white; }"
        "QMenu { background: white; color: black; border: 1px solid #aca899; padding: 2px 0px; }"
        "QMenu::item { padding: 2px 20px 2px 28px; }"
        "QMenu::item:selected { background: #316ac5; color: white; }"
        "QMenu::item:disabled { color: #aca899; }"
        "QMenu::separator { height: 1px; background: #aca899; margin: 3px 1px; }");

    // Load custom Comic Sans MS fonts first so they are available in the font database
    QString app_dir = QCoreApplication::applicationDirPath();
    QString font_path = QDir(app_dir).filePath("Comic.ttf");
    QString bold_font_path = QDir(app_dir).filePath("Comicbd.ttf");
    
    QString font_family = "Comic Sans MS";
    if (QFile::exists(font_path)) {
        int font_id = QFontDatabase::addApplicationFont(font_path);
        if (font_id != -1) {
            font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        }
    }
    if (QFile::exists(bold_font_path)) {
        QFontDatabase::addApplicationFont(bold_font_path);
    }

    QFont app_font(font_family);
    app_font.setStyleStrategy(QFont::NoAntialias);
    app_font.setHintingPreference(QFont::PreferFullHinting);
    app_font.setPixelSize(11);
    QApplication::setFont(app_font);

    QLoggingCategory::setFilterRules("qt.multimedia.*=false\nqt.multimedia.audio*=false");

    ClonkLauncher *launcher = new ClonkLauncher();

    // Splash window: frames of Splash.c4v
    QString splash_dir = QDir(app_dir).filePath("data/splash");

    std::vector<QString> splash_frames;
    for (int i = 1; i <= 51; ++i) {
        QString frame_path = QDir(splash_dir).filePath(QString("splash_%1.png").arg(i, 3, 10, QChar('0')));
        splash_frames.push_back(frame_path);
    }

    QSoundEffect *sound_start = new QSoundEffect();
    sound_start->setSource(QUrl::fromLocalFile(LauncherRes::resPath("wave", 7008)));

    SplashWindow *splash = new SplashWindow(splash_frames, sound_start);
    QObject::connect(splash, &SplashWindow::finished, launcher, [launcher]() {
        launcher->show();
        launcher->start_music();

        // ExplorerDlg::OnInitDialog posts WM_USER+21: the quick start screen (if enabled, player view)
        QTimer::singleShot(0, launcher, [launcher] { QuickStartDlg::showAtStartup(launcher); });
    });

    splash->start();

    return app.exec();
}
