#include "LicenseDlg.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"

#include <QAbstractButton>
#include <QLineEdit>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextEdit>

namespace {
// multiline edit: the formatting rectangle holds whole lines only
class LicenseText : public QTextEdit {
public:
    using QTextEdit::QTextEdit;
    void showWholeLines(int line_height) {
        const int margin = static_cast<int>(document()->documentMargin());
        const int avail = height() - 2 * frameWidth() - 2 * margin;
        setViewportMargins(0, 0, 0, avail > line_height ? avail % line_height : 0);
    }
};
} // namespace

LicenseDlg::LicenseDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_LICENSE, parent) {
    // LicenseDlg::OnInitDialog
    setWindowTitle(LauncherRes::str(50514)); // "Developer Mode License Agreement"
    get<QAbstractButton>(1)->setText(LauncherRes::str(50000)); // "Accept"
    get<QAbstractButton>(2)->setText(LauncherRes::str(50006)); // "Cancel"

    // TEXT resource 6005 (German) or 6006 (everything else): 6006 - SEqualNoCase(Language, "DE")
    bool german = false;
    if (ClonkLauncher *l = ClonkLauncher::instance())
        german = QString::fromStdString(l->get_cfg("General\\Language", "")).compare("DE", Qt::CaseInsensitive) == 0;
    QString text = LauncherRes::text(german ? 6005 : 6006);
    text.replace("\r\n", "\n");

    // 2157: multiline read-only edit with vertical scroll bar (ES_MULTILINE | ES_READONLY | WS_VSCROLL):
    // shown as a text view with the line height of MS Sans Serif 8 (13 px) and the face color of
    // read-only edits
    QWidget *old = control(2157);
    auto *view = new LicenseText(old->parentWidget());
    view->setGeometry(old->geometry());
    view->setFont(old->font());
    view->setReadOnly(true);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameShape(QFrame::StyledPanel);
    QPalette pal = view->palette();
    pal.setColor(QPalette::Base, QColor(0xec, 0xe9, 0xd8));
    view->setPalette(pal);
    view->document()->setDocumentMargin(1);
    view->setPlainText(text);
    QTextCursor cursor(view->document());
    cursor.select(QTextCursor::Document);
    QTextBlockFormat fmt;
    fmt.setLineHeight(13, QTextBlockFormat::FixedHeight);
    cursor.mergeBlockFormat(fmt);
    view->moveCursor(QTextCursor::Start);
    view->showWholeLines(13);
    old->hide();
    text_ = view;

    // the default button has the focus, the text is not selected
    get<QAbstractButton>(1)->setFocus();
}
