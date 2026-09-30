#pragma once

// QuickStartDlg (IDD 3025): the quick start screen of the original launcher, shown after the start
// (ExplorerDlg::OnInitDialog posts WM_USER+21 -> ExplorerDlg::OnUserMsg21 0x415fe0 -> DoModal) if
// Explorer\ShowQuickStart is set and the main window is in player view.
//
// Main page: three pictures (bitmaps 1024-1029) that open the scenario folders Tutorial.c4f\Mouse.c4f,
// Tutorial.c4f\Keyboard.c4f and Easy.c4f; hovering a picture shows its highlighted bitmap and a title
// bitmap below. Folder page: the scenarios (*.c4s) of the folder, three at a time (Title.bmp, "<" / ">"
// scroll by one, "^" goes back), hovering one shows its description, clicking starts it.
//
// Ported from Planet.exe 0x424e60-0x426c30 (QuickStartDlg__*, FUN_00425930 open folder,
// FUN_004263e0 / 0x4261f0 description, FUN_004264a0 start).

#include "ExplorerModel.h"

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <functional>
#include <vector>

class QAbstractButton;
class QCheckBox;
class QPainter;
class QWidget;
class ClonkLauncher;

class QuickStartDlg : public QDialog {
    Q_OBJECT
public:
    explicit QuickStartDlg(QWidget *parent = nullptr);
    ~QuickStartDlg() override;

    // ExplorerDlg::OnInitDialog (0x412e10): Explorer\ShowQuickStart set and player view
    // (Explorer\Mode 0 or developer mode off).
    static bool enabled(ClonkLauncher *launcher);
    // Shows the quick start screen window modal on top of the main window if enabled() (non blocking,
    // deletes itself when closed). Call once after the main window is shown.
    static void showAtStartup(ClonkLauncher *launcher);

    // The "?" button (id 9, SharedDlg::Handler_414200: CWinApp::WinHelp(0, HELP_FINDER) = help
    // contents). Default: ClonkLauncher::showHelp(). Set this to open the help viewer instead.
    static std::function<void(QWidget *parent)> helpHandler;

    // FUN_00425930: opens a scenario folder ("Tutorial.c4f\\Mouse.c4f", relative to the data
    // directory) as the folder page. Returns false if the folder can't be opened.
    bool openFolder(const QString &folder);
    // simulates the mouse over position p (dialog coordinates), as WM_MOUSEMOVE
    void hoverAt(const QPoint &p);
    // left mouse button released at p (WM_LBUTTONUP): opens a folder / starts a scenario
    bool clickAt(const QPoint &p);
    // "<" (-1) / ">" (+1)
    void scroll(int delta);
    // "^"
    void back();

    // keyboard cues (focus rectangles): hidden until the keyboard is used (WM_CHANGEUISTATE)
    bool focusCues() const { return focus_cues_; }
    void setFocusCues(bool on);

    int itemCount() const { return static_cast<int>(items_.size()); }

    int exec() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void done(int result) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Entry {
        ExplorerItem item;
        ExplorerInfo info;
        QByteArray desc; // Desc*.rtf
    };

    void updateButtons();     // FUN_00425fd0
    void updateDescription(); // FUN_004263e0
    void startScenario(int index); // FUN_004264a0
    const Entry *entryAt(int index) const; // FUN_00426460
    int hitTest(const QPoint &p, bool folder_rects) const;
    void showHelp();
    void setFrames(int hover); // FUN_00425ec0
    void repaintSlots();
    QImage slotBase(int slot) const;
    void drawStaticFrame(QPainter &p, int slot) const;

    // bitmaps (OnInitDialog: 0x3fd background, 0x40b logo, 0x400-0x405 pictures, texts)
    QPixmap background_;
    QPixmap logo_;
    QPixmap pictures_[6];
    QPixmap texts_[3];

    // rects of the pictures on the main page (0x1e8) and on the folder page (0x218)
    QRect main_rects_[3];
    QRect folder_rects_[3];

    std::vector<Entry> items_; // list at 0x248, count at 0x264 (folder page if not empty)
    int first_ = 0;            // 0x268: index of the item in the first slot
    int hover_ = 0;            // 0x508: hovered picture 1-3, 0 none

    QAbstractButton *btn_help_ = nullptr;  // 9
    QAbstractButton *btn_close_ = nullptr; // 2
    QAbstractButton *btn_up_ = nullptr;    // 2034 "^"
    QAbstractButton *btn_next_ = nullptr;  // 2061 ">"
    QAbstractButton *btn_prev_ = nullptr;  // 2053 "<"
    QCheckBox *dont_show_ = nullptr;       // 2081
    QWidget *desc_frame_ = nullptr;        // 2221 rich edit (RichEditView)

    // statics 2310-2312: frame (inset: 0 none as in the template, 2 client edge, 3 modal frame),
    // visible (folder page), screen contents of their rects
    int frames_[3] = {0, 0, 0};
    bool statics_visible_ = false;
    QImage slot_images_[3];

    bool in_exec_ = false;
    bool focus_cues_ = false;
    QWidget *hidden_focus_ = nullptr; // focused button that was hidden (keeps the focus, see updateButtons)
};
