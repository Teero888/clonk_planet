#pragma once

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QEnterEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QCoreApplication>
#include <QCheckBox>
#include <vector>
#include "ClonkDialogs.h"
#include <utility>

class HoverLabel : public QLabel {
  Q_OBJECT
public:
  HoverLabel(int index, const QPixmap &normal, const QPixmap &hover, QMediaPlayer *hover_player, QWidget *parent = nullptr)
      : QLabel(parent), m_index(index), normal_pix(normal), hover_pix(hover), m_hoverPlayer(hover_player) {
    setPixmap(normal_pix);
  }

signals:
  void hovered(int index);
  void unhovered();

protected:
  void enterEvent(QEnterEvent *event) override {
    setPixmap(hover_pix);
    if (m_hoverPlayer) {
      m_hoverPlayer->stop();
      m_hoverPlayer->play();
    }
    emit hovered(m_index);
    QLabel::enterEvent(event);
  }

  void leaveEvent(QEvent *event) override {
    setPixmap(normal_pix);
    emit unhovered();
    QLabel::leaveEvent(event);
  }

private:
  int m_index;
  QPixmap normal_pix;
  QPixmap hover_pix;
  QMediaPlayer *m_hoverPlayer;
};

class TutorialWindow : public QWidget {
  Q_OBJECT

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() != Qt::LeftButton)
      return;

    QWidget *clicked_widget = childAt(event->position().toPoint());

    if (!clicked_widget || clicked_widget->inherits("QLabel")) {
      if (m_missPlayer) {
        m_missPlayer->stop();
        m_missPlayer->play();
      }
    }

    QWidget::mousePressEvent(event);
  }

private:
  QMediaPlayer *m_missPlayer = nullptr;
  QCheckBox *check_quick = nullptr;

public:
  explicit TutorialWindow(const QString &dump_path, QWidget *parent = nullptr) : QWidget(parent, Qt::Window) {
    setWindowTitle("Tutorial");
    setAttribute(Qt::WA_DeleteOnClose);

    // Setup Background
    QString path = QDir(dump_path).filePath("Planet_fixed.bin_2_1021_1031.bmp");
    QLabel *bg_label = new QLabel(this);
    QPixmap pix(path);
    if (!pix.isNull()) {
      bg_label->setPixmap(pix);
      setFixedSize(pix.size());
      bg_label->setGeometry(0, 0, pix.width(), pix.height());
    } else {
      setFixedSize(550, 412);
      bg_label->setGeometry(0, 0, 550, 412);
      bg_label->setText("Tutorial background image not found:\n" + path);
      bg_label->setAlignment(Qt::AlignCenter);
    }

    // Setup Hover Sound (Player + Output)
    QMediaPlayer *hover_player = new QMediaPlayer(this);
    QAudioOutput *hover_output = new QAudioOutput(this);
    hover_player->setAudioOutput(hover_output);
    QString sound_file = QDir(QCoreApplication::applicationDirPath()).filePath("res_wav/sound_7009.wav");
    hover_player->setSource(QUrl::fromLocalFile(sound_file));

    // Setup Miss Sound (Player + Output)
    m_missPlayer = new QMediaPlayer(this);
    QAudioOutput *miss_output = new QAudioOutput(this);
    m_missPlayer->setAudioOutput(miss_output);
    QString miss_sound_file = QDir(QCoreApplication::applicationDirPath()).filePath("res_wav/sound_7005.wav");
    m_missPlayer->setSource(QUrl::fromLocalFile(miss_sound_file));

    // Create the single, persistent dynamic text/description label at the bottom
    QLabel *description_label = new QLabel(this);
    description_label->setGeometry(110, 175, 330, 40);
    description_label->hide();

    // Setup Image Pairs and Connect Signals
    std::vector<std::pair<QString, QString>> img_pairs = {{"Planet_fixed.bin_2_1024_1031.bmp", "Planet_fixed.bin_2_1025_1031.bmp"},
                                                          {"Planet_fixed.bin_2_1026_1031.bmp", "Planet_fixed.bin_2_1027_1031.bmp"},
                                                          {"Planet_fixed.bin_2_1028_1031.bmp", "Planet_fixed.bin_2_1029_1031.bmp"}};

    int current_x = 45;
    int top_margin = 60;
    int horizontal_gap = 5;
    int index = 0;

    for (const auto &pair : img_pairs) {
      QString normal_path = QDir(dump_path).filePath(pair.first);
      QString hover_path = QDir(dump_path).filePath(pair.second);
      QPixmap normal_pix(normal_path);
      QPixmap hover_pix(hover_path);

      if (!normal_pix.isNull() && !hover_pix.isNull()) {
        HoverLabel *img_label = new HoverLabel(index, normal_pix, hover_pix, hover_player, this);
        img_label->setGeometry(current_x, top_margin, normal_pix.width(), normal_pix.height());
        img_label->raise();

        connect(img_label, &HoverLabel::hovered, this, [this, dump_path, description_label](int idx) {
          QString desc_image;
          switch (idx) {
          case 0:
            desc_image = "Planet_fixed.bin_2_1033_1031.bmp";
            break;
          case 1:
            desc_image = "Planet_fixed.bin_2_1031_1031.bmp";
            break;
          case 2:
            desc_image = "Planet_fixed.bin_2_1023_1031.bmp";
            break;
          }

          QPixmap pix(QDir(dump_path).filePath(desc_image));
          if (!pix.isNull()) {
            description_label->setPixmap(pix);
            description_label->setFixedSize(pix.size());
            description_label->show();
            description_label->raise();
          }
        });

        connect(img_label, &HoverLabel::unhovered, this, [description_label]() { description_label->hide(); });

        current_x += normal_pix.width() + horizontal_gap;
        index++;
      }
    }

    // Welcome Title Image
    {
      QString path = QDir(dump_path).filePath("Planet_fixed.bin_2_1035_1031.bmp");
      QLabel *label = new QLabel(this);
      QPixmap pix(path);
      if (!pix.isNull()) {
        label->setPixmap(pix);
        label->setGeometry(110, 10, pix.width(), pix.height());
      }
    }

    // Bottom Checkbox Setup
    {
      QMediaPlayer *cb_player0 = new QMediaPlayer(this);
      QAudioOutput *cb_output0 = new QAudioOutput(this);
      cb_player0->setAudioOutput(cb_output0);
      QString cbsound = QDir(QCoreApplication::applicationDirPath()).filePath("res_wav/fixed/sound_7000_cleaned.wav");
      cb_player0->setSource(QUrl::fromLocalFile(cbsound));

      QMediaPlayer *cb_player1 = new QMediaPlayer(this);
      QAudioOutput *cb_output1 = new QAudioOutput(this);
      cb_player1->setAudioOutput(cb_output1);
      cbsound = QDir(QCoreApplication::applicationDirPath()).filePath("res_wav/fixed/sound_7001_cleaned.wav");
      cb_player1->setSource(QUrl::fromLocalFile(cbsound));

      check_quick = new QCheckBox("Don't display this screen in the future.", this);
      check_quick->setGeometry(144, 372, 240, 15);
      check_quick->setStyleSheet("color: black;");
      QString comic_family = CD::GetComicFontFamily(parent);
      QFont check_font(comic_family, 9);
      check_font.setStyleStrategy(QFont::NoAntialias);
      check_quick->setFont(check_font);
      // set_cfg("Explorer\\ShowQuickStart", 1 if self.check_quick.isChecked() else 0)

      connect(check_quick, &QCheckBox::clicked, this, [cb_player0, cb_player1](bool checked) {
        if (checked) {
          cb_player1->stop();
          cb_player1->play();
        } else {
          cb_player0->stop();
          cb_player0->play();
        }
      });
    }
  }
};