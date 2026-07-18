#include "LauncherCompat.h"
#include <QApplication>
#include "ClonkLauncher.h"
#include "SplashWindow.h"
#include "TutorialWindow.h"
#include "OptionsDialog.h"
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
    QApplication::setStyle("Fusion");

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

    // Check for screenshot arguments
    bool take_screenshot_main = false;
    QString screenshot_main_path;
    bool take_screenshot_options = false;
    QString screenshot_options_path;

    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--screenshot-main" && i + 1 < argc) {
            take_screenshot_main = true;
            screenshot_main_path = argv[i+1];
        } else if (QString(argv[i]) == "--screenshot-options" && i + 1 < argc) {
            take_screenshot_options = true;
            screenshot_options_path = argv[i+1];
        }
    }

    if (take_screenshot_main) {
        launcher->show();
        QTimer::singleShot(200, [launcher]() {
            launcher->selectFirstItemAndExpand();
        });
        QTimer::singleShot(1000, [launcher, screenshot_main_path]() {
            QPixmap pix = launcher->centralWidget()->grab();
            pix.save(screenshot_main_path);
            QCoreApplication::quit();
        });
        return app.exec();
    }

    if (take_screenshot_options) {
        OptionsDialog *dlg = new OptionsDialog(launcher);
        dlg->show();
        QTimer::singleShot(1000, [dlg, screenshot_options_path]() {
            QPixmap pix = dlg->grab();
            pix.save(screenshot_options_path);
            QCoreApplication::quit();
        });
        return app.exec();
    }

    // Setup Splash Window
    QString base_path = QDir(QCoreApplication::applicationDirPath()).filePath("..");
    QString splash_dir = QDir(base_path).filePath("launcher/res_splash");

    std::vector<QString> splash_frames;
    for (int i = 1; i <= 51; ++i) {
        QString frame_path = QDir(splash_dir).filePath(QString("splash_%1.png").arg(i, 3, 10, QChar('0')));
        splash_frames.push_back(frame_path);
    }

    QSoundEffect *sound_start = new QSoundEffect();
    sound_start->setSource(QUrl::fromLocalFile(QDir(QCoreApplication::applicationDirPath()).filePath("res_wav/sound_7008.wav")));

    SplashWindow *splash = new SplashWindow(splash_frames, sound_start);
    QObject::connect(splash, &SplashWindow::finished, launcher, [launcher]() {
        launcher->show();
        launcher->start_music();

        // Launch the tutorial window
        TutorialWindow *tut = new TutorialWindow(launcher->getDumpPath(), launcher);
        tut->show();

        // Position it directly on top of the main launcher window
        QTimer::singleShot(100, [launcher, tut]() {
            if (launcher && tut) {
                QPoint center = launcher->geometry().center();
                tut->move(center - tut->rect().center());
            }
        });
    });

    splash->start();

    return app.exec();
}
