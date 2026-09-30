// Geometry of the main window's controls. In developer view the window is resizable: every control
// has anchor flags (ExplorerDlg 0x415cb0 / 0x415ec0) and the window can't get smaller than the
// dialog template (OnGetMinMaxInfo). The window placement of the developer view is stored on close
// (0x4362a0 "Window").

#include "ClonkLauncher.h"
#include "LauncherRes.h"

#include <QCloseEvent>
#include <QMenuBar>
#include <QResizeEvent>

namespace {

// 0x415ec0 flags: 8 move right, 2 move down, 0x20 grow width, 0x80 grow height, 0x10 keep left
QRect anchored(int ctrl_id, int flags, int dx, int dy) {
    QRect r = LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, ctrl_id);
    if (flags & 8)
        r.translate(dx, 0);
    if (flags & 2)
        r.translate(0, dy);
    if (flags & 0x10)
        r.setLeft(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, ctrl_id).left());
    if (flags & 0x20)
        r.setWidth(r.width() + dx);
    if (flags & 0x80)
        r.setHeight(r.height() + dy);
    return r;
}

} // namespace

void ClonkLauncher::layoutControls() {
    if (!ui_container || !tree_frame)
        return;
    const QSize base = LauncherRes::dialogSize(LauncherRes::IDD_EXPLORER);
    const int dx = qMax(0, ui_container->width() - base.width());
    const int dy = qMax(0, ui_container->height() - base.height());

    separator->setGeometry(anchored(2290, 0x30, dx, dy));
    tree_frame->setGeometry(anchored(2381, 0xa0, dx, dy));
    tree->setGeometry(tree_frame->rect().adjusted(2, 2, -2, -2));
    preview_frame->setGeometry(anchored(2016, 8, dx, dy));
    preview->setGeometry(preview_frame->rect());
    desc_frame->setGeometry(anchored(desc_full_area ? 2189 : 2221, 0x88, dx, dy));
    desc->setGeometry(desc_frame->rect().adjusted(2, 2, -2, -2));
    for (auto [btn, id] : {std::pair<QWidget *, int>{btn_new, 2055}, {btn_activate, 2025}, {btn_rename, 2058},
                           {btn_delete, 2039}, {btn_props, 2057}})
        btn->move(anchored(id, 8, dx, dy).topLeft());
    btn_start->move(anchored(2070, 0xa, dx, dy).topLeft());
    btn_quit->move(anchored(2049, 0xa, dx, dy).topLeft());
    view_label->setGeometry(anchored(2368, 8, dx, dy));
    radio_player->setGeometry(anchored(2219, 8, dx, dy));
    radio_developer->setGeometry(anchored(2220, 8, dx, dy));
    author_label->setGeometry(anchored(2267, 0xa, dx, dy));
    anim_frame->setGeometry(anchored(2000, 6, dx, dy));
    animation->setGeometry(anim_frame->rect().adjusted(2, 2, -2, -2));
    status_frame->setGeometry(anchored(2357, 0x26, dx, dy));
    status_bar->setGeometry(status_frame->rect().adjusted(2, 2, -2, -2));
}

// ExplorerDlg 0x414ce0: player view has the dialog's fixed size, developer view is resizable
void ClonkLauncher::applyViewMode() {
    const QSize base = LauncherRes::dialogSize(LauncherRes::IDD_EXPLORER);
    const int menu_h = menuBar() ? menuBar()->sizeHint().height() : 0;
    if (developer_view) {
        central_widget->setMinimumSize(base);
        central_widget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setMinimumSize(base.width(), base.height() + menu_h);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        // stored placement of the developer view
        const QStringList w = QString::fromStdString(get_cfg("Window", "")).split(',');
        if (w.size() == 4)
            setGeometry(w[0].toInt(), w[1].toInt(), qMax(w[2].toInt(), base.width()), qMax(w[3].toInt(), base.height() + menu_h));
    } else {
        central_widget->setFixedSize(base);
        setFixedSize(base.width(), base.height() + menu_h);
    }
    ui_container->resize(central_widget->size());
    layoutControls();
}

void ClonkLauncher::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (ui_container && central_widget) {
        ui_container->resize(central_widget->size());
        layoutControls();
    }
}

void ClonkLauncher::closeEvent(QCloseEvent *event) {
    // ExplorerDlg::OnClose: remember the window of the developer view
    if (developer_view) {
        const QRect g = geometry();
        set_cfg("Window", QString("%1,%2,%3,%4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()).toStdString());
        saveConfig();
    }
    QMainWindow::closeEvent(event);
}
