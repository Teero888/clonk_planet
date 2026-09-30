#pragma once

// Game pad access for the Game Pad options page: the launcher's copy of StdJoystick
// (standard/src/StdJoystick.cpp: StdGetGamepad with the calibration range that grows with the
// positions seen). Reads the first Linux joystick device (/dev/input/js0); elsewhere no game pad.

#include <cstdint>

class OptJoystick {
public:
    OptJoystick();
    ~OptJoystick();

    // PAD_Up = 1, PAD_Down = 2, PAD_Left = 4, PAD_Right = 8
    enum { PAD_None = 0, PAD_Up = 1, PAD_Down = 2, PAD_Left = 4, PAD_Right = 8 };

    // StdGetGamepad: false if there is no game pad
    bool getGamepad(unsigned &pos, unsigned &buttons);

    // StdSetGamepadCalibration / StdGetGamepadCalibration
    static void setCalibration(int min_x, int max_x, int min_y, int max_y);
    static void getCalibration(int &min_x, int &max_x, int &min_y, int &max_y);

    // GetFirstSetBit
    static int firstSetBit(unsigned bits);

private:
    // StdGetJoyPos: raw position 0..65535 and buttons
    bool getJoyPos(unsigned &x, unsigned &y, unsigned &buttons);
    int fd_ = -1;
    int axis_x_ = 0, axis_y_ = 0;
    unsigned buttons_ = 0;
};
