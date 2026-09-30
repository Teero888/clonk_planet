#include "ClonkDialog.h"
#include "LauncherRes.h"
#include "Win3DWidgets.h"
#include "HelpViewer.h"

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <set>

namespace {

constexpr uint32_t WS_CAPTION = 0x00C00000;

// fill color of the standard dialogs (COLOR_BTNFACE of the XP theme in the screenshots)
const char *STANDARD_BG = "#ece9d8";

} // namespace

bool ClonkDialog::isSkinned(int idd) {
    // dialog classes of the original with a WM_ERASEBKGND handler
    static const std::set<int> skinned = {3000, 3001, 3002, 3003, 3005, 3007, 3008, 3009, 3012, 3013,
                                          3017, 3019, 3022, 3023, 3024, 3025, 3026, 3027, 3028, 3029};
    return skinned.count(idd) != 0;
}

ClonkDialog::ClonkDialog(int idd, QWidget *parent)
    : QDialog(parent), idd_(idd), skinned_(isSkinned(idd)) {
    const QSize size = LauncherRes::dialogSize(idd);
    const QJsonObject tmpl = LauncherRes::dialogTemplate(idd);
    const uint32_t style = static_cast<uint32_t>(tmpl.value("style").toDouble());
    // without a caption the window manager draws nothing: paint the DS_MODALFRAME border ourselves
    int border = 0;
    if (!(style & WS_CAPTION)) {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        border = 3;
        auto *frame = new Win3DFrame(this, {}, STANDARD_BG);
        frame->setGeometry(0, 0, size.width() + 2 * border, size.height() + 2 * border);
    }
    setFixedSize(size.width() + 2 * border, size.height() + 2 * border);

    if (skinned_) {
        body_ = new ClonkTexturedWidget(this, LauncherRes::resPath("bitmap", 1019));
        // labels are transparent on the marble; disabled texts in the XP gray
        body_->setStyleSheet("QLabel { background: transparent; color: black; } QLabel:disabled { color: #aca899; }");
    } else {
        body_ = new QWidget(this);
        body_->setAutoFillBackground(true);
        QPalette pal = body_->palette();
        pal.setColor(QPalette::Window, QColor(STANDARD_BG));
        body_->setPalette(pal);
    }
    body_->setGeometry(border, border, size.width(), size.height());
    setFont(skinned_ ? LauncherRes::feFont() : LauncherRes::sysFont());

    builder_ = std::make_unique<DialogBuilder>(idd, body_, skinned_ ? DialogBuilder::Skinned : DialogBuilder::Standard);

    if (auto *ok = qobject_cast<QAbstractButton *>(builder_->widgets().count(1) ? builder_->widget(1) : nullptr)) {
        connect(ok, &QAbstractButton::clicked, this, [this]() { onOK(); });
        if (auto *pb = qobject_cast<QPushButton *>(ok))
            pb->setDefault(true);
    }
    if (auto *cancel = qobject_cast<QAbstractButton *>(builder_->widgets().count(2) ? builder_->widget(2) : nullptr))
        connect(cancel, &QAbstractButton::clicked, this, [this]() { onCancel(); });

    // WM_HELP: F1 shows the help popup of the focused control (Planet.hlp context popups by control id)
    installContextHelp(this);
}

ClonkDialog::~ClonkDialog() = default;

void ClonkDialog::reject() {
    // Escape / window close behave like the cancel button
    if (in_cancel_) {
        QDialog::reject();
        return;
    }
    in_cancel_ = true;
    onCancel();
    in_cancel_ = false;
}

// ---------------------------------------------------------------------------------- MessageDlg

bool clonkMessage(QWidget *parent, const QString &text, MsgButtons buttons) {
    ClonkDialog dlg(LauncherRes::IDD_MESSAGE, parent);
    dlg.setWindowTitle(LauncherRes::str(50516)); // "Clonk Planet"
    dlg.get<QLabel>(2358)->setText(text);
    dlg.get<QLabel>(2358)->setWordWrap(true);
    const bool yes_no = buttons == MsgButtons::YesNo;
    // MessageDlg::OnInitDialog: OK/Cancel or Yes/No, cancel hidden for plain messages
    dlg.get<QAbstractButton>(1)->setText(LauncherRes::str(yes_no ? 50038 : 50026));
    dlg.get<QAbstractButton>(2)->setText(LauncherRes::str(yes_no ? 50024 : 50006));
    dlg.control(2)->setVisible(buttons != MsgButtons::OK);
    return dlg.exec() == QDialog::Accepted;
}

// ---------------------------------------------------------------------------------- PromptDlg

bool clonkPrompt(QWidget *parent, const QString &prompt, QString &value, bool password) {
    ClonkDialog dlg(LauncherRes::IDD_PROMPT, parent);
    dlg.get<QLabel>(2332)->setText(prompt);
    auto *edit = dlg.get<QLineEdit>(2143);
    edit->setText(value);
    edit->selectAll();
    if (password)
        edit->setEchoMode(QLineEdit::Password);
    dlg.get<QAbstractButton>(1)->setText(LauncherRes::str(50026));
    dlg.get<QAbstractButton>(2)->setText(LauncherRes::str(50006));
    edit->setFocus();
    if (dlg.exec() != QDialog::Accepted)
        return false;
    value = edit->text();
    return true;
}
