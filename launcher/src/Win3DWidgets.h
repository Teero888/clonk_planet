#pragma once

#include <QWidget>
#include <QPushButton>
#include <QFrame>
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
    explicit Win3DFrame(QWidget *parent = nullptr, const std::vector<std::string> &colors = {}, const std::string &bg_color = "#f5f5f5");
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::vector<std::string> colors;
    std::string bg_color;
};

class Win3DButton : public QPushButton {
    Q_OBJECT
public:
    explicit Win3DButton(QWidget *parent = nullptr, const std::vector<std::string> &raised = {}, const std::vector<std::string> &sunken = {}, const std::string &bg = "#f5f5f5", const std::string &arrow = "");
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

class Win3DTabWidget : public QWidget {
    Q_OBJECT
public:
    explicit Win3DTabWidget(QWidget *parent = nullptr, const std::vector<std::string> &colors = {});
    void addTab(QWidget *widget, const QString &text, const QIcon &icon = QIcon());
    void setActiveIndex(int index);
    int activeIndex() const { return active_index; }
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    struct TabData {
        QString text;
        QIcon icon;
        QWidget *widget;
    };
    std::vector<std::string> colors;
    std::vector<TabData> tabs;
    int active_index = 0;
    QStackedWidget *stacked = nullptr;
    QVBoxLayout *main_layout = nullptr;
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

class ClonkTextArea : public QFrame {
    Q_OBJECT
public:
    explicit ClonkTextArea(QWidget *parent = nullptr, const std::string &bg_color = "");
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::string bg_color;
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
