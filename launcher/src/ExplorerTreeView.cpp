#include "ExplorerTreeView.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QUrl>

ExplorerTreeView::ExplorerTreeView(QWidget *parent) : QTreeView(parent) {
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
}

void ExplorerTreeView::setShowLines(bool show) {
    show_lines_ = show;
    setRootIsDecorated(show);
    viewport()->update();
}

void ExplorerTreeView::drawBranches(QPainter *p, const QRect &rect, const QModelIndex &index) const {
    if (!show_lines_)
        return;
    const QColor line(0xac, 0xa8, 0x99);
    const int indent = indentation();
    const int mid_y = rect.top() + rect.height() / 2;
    auto dotted_v = [&](int x, int y0, int y1) {
        for (int y = y0; y <= y1; ++y)
            if ((x + y) % 2 == 0)
                p->fillRect(x, y, 1, 1, line);
    };
    auto dotted_h = [&](int x0, int x1, int y) {
        for (int x = x0; x <= x1; ++x)
            if ((x + y) % 2 == 0)
                p->fillRect(x, y, 1, 1, line);
    };
    auto has_next = [&](const QModelIndex &i) { return i.sibling(i.row() + 1, 0).isValid(); };

    int x = rect.right() - indent + 1;
    const int cx = x + indent / 2;
    // this item: line to the right, up to the previous item and down to the next sibling
    dotted_h(cx, x + indent - 1, mid_y);
    dotted_v(cx, rect.top(), has_next(index) ? rect.bottom() : mid_y);
    // columns of the ancestors: vertical line if the ancestor has a following sibling
    QModelIndex a = index.parent();
    int ax = x - indent;
    while (a.isValid()) {
        if (has_next(a))
            dotted_v(ax + indent / 2, rect.top(), rect.bottom());
        a = a.parent();
        ax -= indent;
    }
    // +/- box
    if (model()->hasChildren(index)) {
        const QRect box(cx - 4, mid_y - 4, 8, 8);
        p->fillRect(box.adjusted(0, 0, 1, 1), Qt::white);
        p->setPen(line);
        p->drawRect(box);
        p->fillRect(cx - 2, mid_y, 5, 1, Qt::black);
        if (!isExpanded(index))
            p->fillRect(cx, mid_y - 2, 1, 5, Qt::black);
    }
}

void ExplorerTreeView::mousePressEvent(QMouseEvent *event) {
    // ExplorerTree::OnRButtonDown: right click selects the item under the cursor (or nothing)
    if (event->button() == Qt::RightButton) {
        const QModelIndex idx = indexAt(event->pos());
        if (idx.isValid())
            setCurrentIndex(idx);
        else
            clearSelection();
        return;
    }
    if (event->button() == Qt::LeftButton) {
        press_pos_ = event->pos();
        drag_index_ = indexAt(event->pos());
    }
    QTreeView::mousePressEvent(event);
}

void ExplorerTreeView::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && drag_index_.isValid() && !dragging_ &&
        (event->pos() - press_pos_).manhattanLength() >= QApplication::startDragDistance()) {
        // TVN_BEGINDRAG: drag image of the item
        dragging_ = true;
        auto *mime = new QMimeData;
        mime->setData("application/x-clonk-explorer-item", QByteArray());
        auto *drag = new QDrag(this);
        drag->setMimeData(mime);
        const QRect r = visualRect(drag_index_);
        drag->setPixmap(viewport()->grab(r));
        drag->setHotSpot(press_pos_ - r.topLeft());
        drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
        dragging_ = false;
        return;
    }
    QTreeView::mouseMoveEvent(event);
}

void ExplorerTreeView::mouseReleaseEvent(QMouseEvent *event) {
    drag_index_ = QPersistentModelIndex();
    QTreeView::mouseReleaseEvent(event);
}

void ExplorerTreeView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/x-clonk-explorer-item") || event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ExplorerTreeView::dragMoveEvent(QDragMoveEvent *event) {
    // ExplorerTree::OnMouseMove: copy cursor, Ctrl held = move
    const bool internal = event->mimeData()->hasFormat("application/x-clonk-explorer-item");
    event->setDropAction(internal && (event->modifiers() & Qt::ControlModifier) ? Qt::MoveAction : Qt::CopyAction);
    const QModelIndex target = indexAt(event->position().toPoint());
    if (target.isValid())
        selectionModel()->setCurrentIndex(target, QItemSelectionModel::NoUpdate);
    event->accept();
}

void ExplorerTreeView::dropEvent(QDropEvent *event) {
    const QModelIndex target = indexAt(event->position().toPoint());
    if (event->mimeData()->hasFormat("application/x-clonk-explorer-item")) {
        const bool move = event->modifiers() & Qt::ControlModifier;
        event->setDropAction(Qt::CopyAction); // the model is updated by the launcher, not by Qt
        event->accept();
        if (drag_index_.isValid() && drag_index_ != target)
            emit itemDropped(drag_index_, target, move);
        return;
    }
    // ExplorerTree::OnDropFiles
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls())
        if (url.isLocalFile())
            files << url.toLocalFile();
    event->acceptProposedAction();
    if (!files.isEmpty())
        emit filesDropped(files, target);
}

void ExplorerTreeView::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Insert:
    case Qt::Key_Delete:
    case Qt::Key_F2:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit keyAction(event->key());
        return;
    default:
        QTreeView::keyPressEvent(event);
    }
}

void ExplorerTreeView::contextMenuEvent(QContextMenuEvent *event) {
    emit contextMenuAt(event->globalPos());
}
