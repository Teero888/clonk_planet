#pragma once

// RegistrationDlg (IDD 3026): player name + registration code (Options > Registration...)

#include "ClonkDialog.h"

class RegistrationDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit RegistrationDlg(QWidget *parent = nullptr);

    QString name() const;
    QString code() const;
};
