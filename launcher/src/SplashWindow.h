#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QSoundEffect>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <vector>

class SplashWindow : public QWidget {
    Q_OBJECT
public:
    explicit SplashWindow(const std::vector<QString> &frames, QSoundEffect *start_sound, QWidget *parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint), frames(frames), start_sound(start_sound) {
        setFixedSize(550, 412);
        
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeom = screen->geometry();
            move(screenGeom.center() - rect().center());
        }

        label = new QLabel(this);
        label->setGeometry(0, 0, 550, 412);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &SplashWindow::nextFrame);
    }

    void start() {
        if (start_sound) start_sound->play();
        frame_idx = 0;
        extra_ticks = 0;
        timer->start(33); // ~30 fps
        nextFrame();
        show();
    }

signals:
    void finished();

private slots:
    void nextFrame() {
        if (frame_idx < (int)frames.size()) {
            QPixmap pix(frames[frame_idx]);
            label->setPixmap(pix);
            frame_idx++;
        } else {
            extra_ticks++;
            if (extra_ticks >= 15) {
                timer->stop();
                close();
                emit finished();
                deleteLater();
            }
        }
    }

private:
    std::vector<QString> frames;
    QSoundEffect *start_sound = nullptr;
    QLabel *label = nullptr;
    QTimer *timer = nullptr;
    int frame_idx = 0;
    int extra_ticks = 0;
};
