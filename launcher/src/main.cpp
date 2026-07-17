#include "LauncherCompat.h"
#include <QApplication>
#include "ClonkLauncher.h"
#include "SplashWindow.h"
#include "TutorialWindow.h"
#include <QDir>
#include <QUrl>
#include <QLoggingCategory>
#include <QTimer>
#include <vector>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");
    QLoggingCategory::setFilterRules("qt.multimedia.*=false\nqt.multimedia.audio*=false");

    ClonkLauncher *launcher = new ClonkLauncher();

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
