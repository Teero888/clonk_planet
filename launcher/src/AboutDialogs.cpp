// AboutDlg (IDD 3000, bitmap 1000 + version line) and CreditsDlg (IDD 3001, bitmap 1010): borderless
// pictures sized to their bitmap, closed by a click.

#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "GroupEdit.h"
#include "HelpViewer.h"
#include "Registration.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QKeyEvent>

namespace {

class PictureDialog : public QDialog {
public:
    PictureDialog(QWidget *parent, int bitmap_id, const QString &text)
        : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint), pix_(LauncherRes::bitmap(bitmap_id)), text_(text) {
        setFixedSize(pix_.size());
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.drawPixmap(0, 0, pix_);
        if (text_.isEmpty())
            return;
        // AboutDlg::OnPaint: version line centered in the bottom 30 pixels, black shadow + white text
        QFont f = LauncherRes::sysFont();
        f.setBold(true);
        f.setPixelSize(13);
        p.setFont(f);
        const QRect r(0, height() - 30, width(), 30);
        p.setPen(Qt::black);
        p.drawText(r.translated(2, 2), Qt::AlignHCenter | Qt::AlignTop, text_);
        p.setPen(Qt::white);
        p.drawText(r, Qt::AlignHCenter | Qt::AlignTop, text_);
    }
    void mousePressEvent(QMouseEvent *) override { accept(); }

private:
    QPixmap pix_;
    QString text_;
};

} // namespace

QDialog *makeAboutDialog(QWidget *parent) {
    // AboutDlg::OnInitDialog: FileVersion of Planet.exe and the engine (both 4.65)
    QString text = LauncherRes::str(50360);
    text.replace(text.indexOf("%s"), 2, "4.65");
    text.replace(text.indexOf("%s"), 2, "4.65");
    return new PictureDialog(parent, 1000, text);
}

QDialog *makeCreditsDialog(QWidget *parent) {
    return new PictureDialog(parent, 1010, QString());
}

void ClonkLauncher::showAbout() {
    QDialog *dlg = makeAboutDialog(this);
    dlg->exec();
    delete dlg;
}

void ClonkLauncher::showCredits() {
    QDialog *dlg = makeCreditsDialog(this);
    dlg->exec();
    delete dlg;
}

void ClonkLauncher::showHelp() {
    showHelpContents(this);
}

void ClonkLauncher::showRegistration() {
    if (::runRegistrationDialog(this)) {
        // registered now: title, maker, properties access and the tree change (F5)
        applyRegistration();
        refreshTree();
    }
}

// title and C4Group maker of the (un)registered user (C4Application: C4Group_SetMaker)
void ClonkLauncher::applyRegistration() {
    setWindowTitle(windowTitleText());
    const std::string name = get_cfg("General\\Name", "");
    GroupEdit::setMaker(isRegistered() && !name.empty() ? QString::fromStdString(name) : LauncherRes::str(51603));
}

bool ClonkLauncher::isRegistered() const {
    return ::isRegistered();
}

QString ClonkLauncher::windowTitleText() const {
    return ::mainWindowTitle();
}

void ClonkLauncher::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_F1) {
        // control ids of the ExplorerDlg template (Planet.hlp popups t1-t16)
        const std::pair<QWidget *, int> controls[] = {
            {tree, 2381}, {preview_frame, 2016}, {desc_frame, 2221}, {status_frame, 2357}, {btn_new, 2055},
            {btn_activate, 2025}, {btn_rename, 2058}, {btn_delete, 2039}, {btn_props, 2057},
            {radio_player, 2219}, {radio_developer, 2220}, {btn_start, 2070}, {btn_quit, 2049},
            {view_label, 2368}, {author_label, 2267}};
        QWidget *w = QApplication::focusWidget();
        QWidget *under = QApplication::widgetAt(QCursor::pos());
        for (const auto &c : controls) {
            if (c.first && ((w && (w == c.first || c.first->isAncestorOf(w))) ||
                            (under && (under == c.first || c.first->isAncestorOf(under))))) {
                showHelpPopup(this, c.second, QCursor::pos());
                return;
            }
        }
        showHelpContents(this);
        return;
    }
    QMainWindow::keyPressEvent(event);
}
