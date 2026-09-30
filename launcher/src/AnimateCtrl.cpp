#include "AnimateCtrl.h"
#include "LauncherRes.h"

#include <QImage>
#include <QMouseEvent>

AnimateCtrl::AnimateCtrl(QWidget *parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter); // ACS_CENTER
    connect(&timer_, &QTimer::timeout, this, [this]() {
        if (frame_ + 1 >= frames_) {
            if (!loop_) {
                timer_.stop();
                return;
            }
            showFrame(0);
        } else {
            showFrame(frame_ + 1);
        }
    });
}

void AnimateCtrl::play(int avi_id, bool loop) {
    timer_.stop();
    clip_ = avi_id;
    loop_ = loop;
    QImage img(LauncherRes::resPath("avi", avi_id));
    if (img.isNull()) {
        clear();
        return;
    }
    frames_ = qMax(1, img.text("frames").toInt());
    frame_width_ = img.text("frame_width").toInt();
    if (frame_width_ <= 0)
        frame_width_ = img.width() / frames_;
    sheet_ = QPixmap::fromImage(img);
    showFrame(0);
    if (frames_ > 1)
        timer_.start(qMax(10, img.text("frame_delay_ms").toInt()));
}

void AnimateCtrl::stop() {
    timer_.stop();
}

void AnimateCtrl::showFrame(int frame) {
    frame_ = frame;
    setPixmap(sheet_.copy(frame * frame_width_, 0, frame_width_, sheet_.height()));
}

void AnimateCtrl::mouseReleaseEvent(QMouseEvent *event) {
    emit clicked(event->button());
    QLabel::mouseReleaseEvent(event);
}
