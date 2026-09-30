#pragma once

// Base class for dialogs recreated from the original dialog templates.
//
// The dialog gets the template's client size and all of its controls (DialogBuilder). Dialogs whose
// original class paints the marble background (they handle WM_ERASEBKGND) are skinned: marble
// texture, frontend font and ClonkButton push buttons. The others use the standard Windows look.
// Control 1 (IDOK) calls onOK(), control 2 (IDCANCEL) calls onCancel().

#include "DialogBuilder.h"

#include <QDialog>
#include <memory>

class ClonkDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClonkDialog(int idd, QWidget *parent = nullptr);
    ~ClonkDialog() override;

    QWidget *control(int id) const { return builder_->widget(id); }
    template <class T> T *get(int id) const { return builder_->get<T>(id); }

    // widget that holds the controls (paint custom things on it or add children)
    QWidget *body() const { return body_; }
    bool skinned() const { return skinned_; }
    int idd() const { return idd_; }

    // Does the original class of this dialog template paint the marble background?
    static bool isSkinned(int idd);

protected:
    virtual void onOK() { accept(); }
    virtual void onCancel() { reject(); }
    void reject() override;

private:
    int idd_;
    bool skinned_;
    bool in_cancel_ = false;
    QWidget *body_ = nullptr;
    std::unique_ptr<DialogBuilder> builder_;
};

// MessageDlg (IDD 3008): the launcher's message box.
enum class MsgButtons { OK, OKCancel, YesNo };
bool clonkMessage(QWidget *parent, const QString &text, MsgButtons buttons = MsgButtons::OK);

// PromptDlg (IDD 3004): single line text input without caption. Returns false on cancel.
bool clonkPrompt(QWidget *parent, const QString &prompt, QString &value, bool password = false);
