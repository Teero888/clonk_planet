#include "OptJoystick.h"

#include <algorithm>
#include <cerrno>

#ifdef __linux__
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#endif

namespace {
// calibration range (dwStdGamepadMin/Max), shared like the globals of StdJoystick
unsigned g_max_x = 0, g_min_x = 0, g_max_y = 0, g_min_y = 0;
} // namespace

OptJoystick::OptJoystick() {
#ifdef __linux__
    fd_ = ::open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
#endif
}

OptJoystick::~OptJoystick() {
#ifdef __linux__
    if (fd_ >= 0)
        ::close(fd_);
#endif
}

void OptJoystick::setCalibration(int min_x, int max_x, int min_y, int max_y) {
    g_max_x = max_x;
    g_min_x = min_x;
    g_max_y = max_y;
    g_min_y = min_y;
}

void OptJoystick::getCalibration(int &min_x, int &max_x, int &min_y, int &max_y) {
    max_x = g_max_x;
    min_x = g_min_x;
    max_y = g_max_y;
    min_y = g_min_y;
}

int OptJoystick::firstSetBit(unsigned bits) {
    for (int bit = 0; bit < 32; ++bit)
        if (bits & (1u << bit))
            return bit;
    return -1;
}

bool OptJoystick::getJoyPos(unsigned &x, unsigned &y, unsigned &buttons) {
#ifdef __linux__
    if (fd_ < 0)
        return false;
    js_event ev;
    ssize_t n;
    while ((n = ::read(fd_, &ev, sizeof(ev))) == static_cast<ssize_t>(sizeof(ev))) {
        const int type = ev.type & ~JS_EVENT_INIT;
        if (type == JS_EVENT_AXIS) {
            if (ev.number == 0)
                axis_x_ = ev.value;
            else if (ev.number == 1)
                axis_y_ = ev.value;
        } else if (type == JS_EVENT_BUTTON && ev.number < 32) {
            if (ev.value)
                buttons_ |= 1u << ev.number;
            else
                buttons_ &= ~(1u << ev.number);
        }
    }
    if (n < 0 && errno != EAGAIN) { // device unplugged
        ::close(fd_);
        fd_ = -1;
        return false;
    }
    // joyGetPosEx range 0..65535
    x = static_cast<unsigned>(axis_x_ + 32768);
    y = static_cast<unsigned>(axis_y_ + 32768);
    buttons = buttons_;
    return true;
#else
    (void)x;
    (void)y;
    (void)buttons;
    return false;
#endif
}

bool OptJoystick::getGamepad(unsigned &pos, unsigned &buttons) {
    unsigned x, y;
    if (!getJoyPos(x, y, buttons))
        return false;
    // calibration range
    g_max_x = std::max(g_max_x, x);
    g_min_x = std::min(g_min_x, x);
    g_max_y = std::max(g_max_y, y);
    g_min_y = std::min(g_min_y, y);
    const unsigned cx = (g_max_x + g_min_x) / 2, cy = (g_max_y + g_min_y) / 2;
    const unsigned rx = (g_max_x - cx) / 3, ry = (g_max_y - cy) / 3;
    // evaluate
    pos = PAD_None;
    if (x < cx - rx)
        pos |= PAD_Left;
    if (x > cx + rx)
        pos |= PAD_Right;
    if (y < cy - ry)
        pos |= PAD_Up;
    if (y > cy + ry)
        pos |= PAD_Down;
    return true;
}
