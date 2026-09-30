#pragma once

// SysAnimate32 replacement: plays the AVI clips of Planet.exe (extracted as sprite sheets to
// data/res/avi/<id>.png with frame count / width / delay in PNG text chunks).

#include <QLabel>
#include <QPixmap>
#include <QTimer>

class AnimateCtrl : public QLabel {
    Q_OBJECT
public:
    explicit AnimateCtrl(QWidget *parent = nullptr);

    // Animate_Open + Animate_Play(from 0, to end, repeat): loop plays forever, else once and stops on the last frame
    void play(int avi_id, bool loop = true);
    void stop();
    int clip() const { return clip_; }

signals:
    void clicked(Qt::MouseButton button);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void showFrame(int frame);

    QTimer timer_;
    QPixmap sheet_;
    int clip_ = 0;
    int frames_ = 0;
    int frame_width_ = 0;
    int frame_ = 0;
    bool loop_ = true;
};
