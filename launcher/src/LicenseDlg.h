#pragma once

// LicenseDlg (IDD 3006): developer mode license agreement (Accept / Cancel), shown by the Program
// options page when "Enable developer mode" is checked.

#include "ClonkDialog.h"

class LicenseDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit LicenseDlg(QWidget *parent = nullptr);

private:
    QWidget *text_ = nullptr;
};
