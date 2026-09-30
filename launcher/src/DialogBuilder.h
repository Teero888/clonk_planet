#pragma once

// Creates the controls of an original dialog template (data/dialogs.json) on a widget, with the
// exact pixel geometry and localised texts of Planet.exe. Code then binds behaviour by control id,
// the same way the original MFC classes did (DDX_Control / message maps).

#include <QWidget>
#include <map>

class DialogBuilder {
public:
    // Skinned: the marble look of the original's frontend dialogs (ClonkButton push buttons,
    // frontend font). Standard: plain Windows controls with the system dialog font.
    enum Skin { Standard, Skinned };

    DialogBuilder(int idd, QWidget *parent, Skin skin = Standard);

    // Widget created for a control id (an edit + its up-down buddy share one QSpinBox,
    // ES_MULTILINE edits are QPlainTextEdit, others QLineEdit).
    QWidget *widget(int ctrl_id) const;

    template <class T> T *get(int ctrl_id) const { return qobject_cast<T *>(widget(ctrl_id)); }

    int idd() const { return idd_; }
    const std::map<int, QWidget *> &widgets() const { return widgets_; }

private:
    int idd_;
    std::map<int, QWidget *> widgets_;
};
