#pragma once

// The main window's tree control (ExplorerTree): own drag & drop (copy, Ctrl = move), file drops
// from outside, right click context menu and the keyboard shortcuts of TVN_KEYDOWN.

#include <QTreeView>

class ExplorerTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit ExplorerTreeView(QWidget *parent = nullptr);

    // TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT (developer view): XP dotted lines and +/- boxes
    void setShowLines(bool show);
    // label of this item is being edited (TVM_EDITLABEL)
    bool isEditing(const QModelIndex &index) const { return state() == EditingState && currentIndex() == index; }

signals:
    // item dragged onto another item (invalid target = root / working directory)
    void itemDropped(const QModelIndex &source, const QModelIndex &target, bool move);
    // files dropped from outside
    void filesDropped(const QStringList &files, const QModelIndex &target);
    void contextMenuAt(const QPoint &global_pos);
    // TVN_KEYDOWN: Space, Insert, Delete, F2, F5, F6
    void keyAction(int key);

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QPoint press_pos_;
    QPersistentModelIndex drag_index_;
    bool dragging_ = false;
    bool show_lines_ = false;
};
