#pragma once

#include <QWidget>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QSoundEffect>
#include <QIcon>
#include <vector>
#include <string>

class Win3DFrame : public QFrame {
    Q_OBJECT
public:
    explicit Win3DFrame(QWidget *parent = nullptr, const std::vector<std::string> &colors = {}, const std::string &bg_color = "#ece9d8");
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::vector<std::string> colors;
    std::string bg_color;
};

class Win3DButton : public QPushButton {
    Q_OBJECT
public:
    explicit Win3DButton(QWidget *parent = nullptr, const std::vector<std::string> &raised = {}, const std::vector<std::string> &sunken = {}, const std::string &bg = "#ece9d8", const std::string &arrow = "");
    Win3DButton(const QString &text, QWidget *parent = nullptr);
    static QSoundEffect *click_sound;
    static QString font_family;
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void playClick();
private:
    std::vector<std::string> raised_colors;
    std::vector<std::string> sunken_colors;
    std::string bg_color;
    std::string arrow;
};

class Win3DGroupBox : public QWidget {
    Q_OBJECT
public:
    explicit Win3DGroupBox(const QString &title, QWidget *parent = nullptr, const std::vector<std::string> &colors = {});
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QString m_title;
    std::vector<std::string> colors;
};

// Tab control with the classic Windows look (comctl32 SysTabControl32 as used by the property
// sheets of the original, XP colors): tabs get their natural width (icon + text), rows are
// balanced and justified to the full width, the row of the selected tab is moved next to the page
// frame. Pages are children placed at pageRect(); only the active one is visible.
class Win3DTabWidget : public QWidget {
    Q_OBJECT
public:
    explicit Win3DTabWidget(QWidget *parent = nullptr, const std::vector<std::string> &colors = {});
    void addTab(QWidget *widget, const QString &text, const QIcon &icon = QIcon());
    void setActiveIndex(int index);
    int count() const { return static_cast<int>(tabs.size()); }
    QWidget *page(int index) const;

    // number of tab rows at the given control width
    int rowCount(int width) const;
    // control size that holds pages of the given size (tab rows + frame)
    QSize sizeForPageSize(const QSize &page) const;
    // area of the pages inside the frame
    QRect pageRect() const;
    // (inclusive) rectangle of a tab as drawn, without the enlargement of the selected tab
    QRect tabRect(int index) const;

    static constexpr int ROW_HEIGHT = 19;
    static constexpr int SELECTED_OFFSET = 2;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct TabData {
        QString text;
        QIcon icon;
        QWidget *widget;
        int row = 0;           // logical row
        int left = 0, right = 0; // x range (inclusive) inside the row
    };
    int naturalWidth(const TabData &t) const;
    void layoutTabs();
    int frameTop() const;
    int visualRow(int logical_row) const;
    void drawTab(QPainter &p, int index, bool selected);

    std::vector<std::string> colors;
    std::vector<TabData> tabs;
    std::vector<int> row_order; // row_order[logical row] = distance from the frame (0 = next to it)
    int num_rows = 1;
    int active_index = -1;
};

class ClonkArea : public QFrame {
    Q_OBJECT
public:
    explicit ClonkArea(QWidget *parent = nullptr, const std::string &bg_color = "", const std::vector<std::string> &borders = {});
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::string bg_color;
    std::vector<std::string> border_colors;
};

class ClonkPreviewLabel : public QLabel {
    Q_OBJECT
public:
    explicit ClonkPreviewLabel(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};

class ClonkTextArea : public QFrame {
    Q_OBJECT
public:
    explicit ClonkTextArea(QWidget *parent = nullptr, const std::string &bg_color = "");
    void setText(const QString &text) { text_content = text; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::string bg_color;
    QString text_content;
};

class ClonkButton : public QPushButton {
    Q_OBJECT
public:
    explicit ClonkButton(const QString &text, QWidget *parent = nullptr, const QString &bg_path = "", const QPoint &bg_offset = QPoint(0, 0), const QSize &size = QSize(86, 20));
    static QSoundEffect *click_sound;
    static QString font_family;
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void playClick();
private:
    QPixmap bg_pix;
    QPoint bg_offset;
};

class ClonkAtlasWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClonkAtlasWidget(QWidget *parent = nullptr, const QString &bg_path = "", const QSize &sub_size = QSize(0, 0), int index = 0);
    void setIndex(int index);
    int getIndex() const { return index; }
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QPixmap bg_pix;
    QSize sub_size;
    int index = 0;
};

class ClonkTexturedWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClonkTexturedWidget(QWidget *parent = nullptr, const QString &bg_path = "");
    void setTexture(const QString &bg_path);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap bg_pix;
};
