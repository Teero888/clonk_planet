#include "DialogBuilder.h"
#include "LauncherRes.h"
#include "Win3DWidgets.h"
#include "ScenStyle.h"

#include <QCheckBox>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QTreeWidget>
#include <iostream>

namespace {

// window / control styles (winuser.h)
constexpr uint32_t WS_VISIBLE = 0x10000000;
constexpr uint32_t WS_DISABLED = 0x08000000;
constexpr uint32_t BS_TYPEMASK = 0x0F;
constexpr uint32_t SS_TYPEMASK = 0x1F;
constexpr uint32_t UDS_AUTOBUDDY = 0x10;
constexpr uint32_t TBS_VERT = 0x02;
constexpr uint32_t ES_MULTILINE = 0x0004;
constexpr uint32_t ES_PASSWORD = 0x0020;
constexpr uint32_t ES_AUTOHSCROLL = 0x0080;
constexpr uint32_t ES_READONLY = 0x0800;
constexpr uint32_t WS_HSCROLL = 0x00100000;
constexpr uint32_t WS_VSCROLL = 0x00200000;

// height of a combo box edit field with MS Sans Serif 8 (the template height includes the drop list)
constexpr int COMBO_HEIGHT = 21;

QRect pxRect(const QJsonObject &c) {
    return {LauncherRes::dluToPxX(c.value("x").toInt()), LauncherRes::dluToPxY(c.value("y").toInt()),
            LauncherRes::dluToPxX(c.value("w").toInt()), LauncherRes::dluToPxY(c.value("h").toInt())};
}

QString controlText(const QJsonObject &c) {
    if (c.contains("text_id"))
        return LauncherRes::str(c.value("text_id").toInt());
    return c.value("key").toString();
}

QWidget *createButton(const QJsonObject &c, QWidget *parent, DialogBuilder::Skin skin) {
    const uint32_t style = static_cast<uint32_t>(c.value("style").toDouble());
    const QString text = controlText(c);
    switch (style & BS_TYPEMASK) {
    case 2: case 3: case 5: case 6: // BS_CHECKBOX, BS_AUTOCHECKBOX, BS_3STATE, BS_AUTO3STATE
        return new QCheckBox(text, parent);
    case 4: case 9: // BS_RADIOBUTTON, BS_AUTORADIOBUTTON
        return new QRadioButton(text, parent);
    case 7: // BS_GROUPBOX
        return new Win3DGroupBox(text, parent);
    default:
        if (skin == DialogBuilder::Skinned) {
            // ClonkButton tiles bitmap 1006 centered on the button
            const QRect r = pxRect(c);
            const QPixmap tex = LauncherRes::bitmap(1006);
            const QPoint offset((tex.width() - r.width()) / 2, (tex.height() - r.height()) / 2);
            return new ClonkButton(text, parent, LauncherRes::resPath("bitmap", 1006), offset, r.size());
        }
        return new Win3DButton(text, parent);
    }
}

QWidget *createStatic(const QJsonObject &c, QWidget *parent) {
    const uint32_t style = static_cast<uint32_t>(c.value("style").toDouble());
    const uint32_t type = style & SS_TYPEMASK;
    if (type >= 4 && type <= 9) { // SS_BLACKRECT .. SS_WHITEFRAME: plain frames
        auto *f = new QFrame(parent);
        f->setFrameShape(QFrame::Box);
        return f;
    }
    if (type == 0x10) { // SS_ETCHEDHORZ
        auto *f = new QFrame(parent);
        f->setFrameShape(QFrame::HLine);
        f->setFrameShadow(QFrame::Sunken);
        return f;
    }
    auto *l = new QLabel(controlText(c), parent);
    l->setAlignment(type == 1 ? Qt::AlignHCenter | Qt::AlignTop : type == 2 ? Qt::AlignRight | Qt::AlignTop
                                                                           : Qt::AlignLeft | Qt::AlignTop);
    l->setWordWrap(type == 0 || type == 1 || type == 2);
    return l;
}

QWidget *createControl(const QJsonObject &c, QWidget *parent, DialogBuilder::Skin skin) {
    const QString cls = c.value("cls").toString();
    const uint32_t style = static_cast<uint32_t>(c.value("style").toDouble());
    if (cls == "Button")
        return createButton(c, parent, skin);
    if (cls == "Static")
        return createStatic(c, parent);
    if (cls == "Edit") {
        if (style & ES_MULTILINE) {
            auto *e = new QPlainTextEdit(parent);
            e->setReadOnly(style & ES_READONLY);
            e->setHorizontalScrollBarPolicy((style & WS_HSCROLL) ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
            e->setVerticalScrollBarPolicy((style & WS_VSCROLL) ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
            e->setLineWrapMode((style & ES_AUTOHSCROLL) ? QPlainTextEdit::NoWrap : QPlainTextEdit::WidgetWidth);
            return e;
        }
        auto *e = new QLineEdit(parent);
        e->setReadOnly(style & ES_READONLY);
        if (style & ES_PASSWORD)
            e->setEchoMode(QLineEdit::Password);
        return e;
    }
    if (cls == "ComboBox")
        return new QComboBox(parent);
    if (cls == "ListBox" || cls == "SysListView32")
        return new QListWidget(parent);
    if (cls == "SysTreeView32")
        return new QTreeWidget(parent);
    if (cls == "msctls_trackbar32")
        return new QSlider(style & TBS_VERT ? Qt::Vertical : Qt::Horizontal, parent);
    if (cls == "RICHEDIT" || cls.startsWith("RichEdit")) {
        auto *t = new QTextEdit(parent);
        t->setReadOnly(true);
        return t;
    }
    if (cls == "msctls_hotkey32")
        return new QLineEdit(parent);
    // SysAnimate32, SysTabControl32, custom classes: placeholder the caller replaces or paints on
    return new QWidget(parent);
}

} // namespace

DialogBuilder::DialogBuilder(int idd, QWidget *parent, Skin skin) : idd_(idd) {
    const QFont font = skin == Skinned ? LauncherRes::feFont() : LauncherRes::sysFont();
    // walk the template in order to keep the original z-order
    const QJsonArray controls = LauncherRes::dialogControls(idd);
    if (controls.isEmpty())
        std::cerr << "DialogBuilder: no template for dialog " << idd << std::endl;

    for (int i = 0; i < controls.size(); ++i) {
        const QJsonObject c = controls.at(i).toObject();
        const int id = c.value("id").toInt();
        const QString cls = c.value("cls").toString();
        const uint32_t style = static_cast<uint32_t>(c.value("style").toDouble());
        QRect rect = pxRect(c);
        QWidget *w = nullptr;

        if (cls == "msctls_updown32" && (style & UDS_AUTOBUDDY) && i > 0) {
            // auto buddy: the preceding edit control becomes a spin box including the arrows
            const int buddy_id = controls.at(i - 1).toObject().value("id").toInt();
            auto it = widgets_.find(buddy_id);
            if (it != widgets_.end()) {
                QWidget *edit = it->second;
                auto *spin = new QSpinBox(parent);
                spin->setRange(0, 100);
                spin->setGeometry(edit->geometry().united(rect));
                spin->setFont(font);
                spin->setEnabled(edit->isEnabled());
                spin->setVisible(edit->isVisibleTo(parent));
                delete edit;
                it->second = spin;
                widgets_[id] = spin;
                continue;
            }
        }

        w = createControl(c, parent, skin);
        // skinned check boxes / radio buttons play sounds (control class 0x402a00, 0x402bc0)
        if (skin == Skinned) {
            if (auto *cb = qobject_cast<QCheckBox *>(w))
                QObject::connect(cb, &QCheckBox::clicked, cb, [](bool on) { LauncherRes::playSound(on ? 7000 : 7001); });
            else if (auto *rb = qobject_cast<QRadioButton *>(w))
                QObject::connect(rb, &QRadioButton::clicked, rb, []() { LauncherRes::playSound(7005); });
        }
        if (qobject_cast<QLineEdit *>(w) || qobject_cast<QPlainTextEdit *>(w) || qobject_cast<QSpinBox *>(w) ||
            qobject_cast<QComboBox *>(w)) {
            // disabled edit fields of the XP theme
            QPalette pal = w->palette();
            pal.setColor(QPalette::Disabled, QPalette::Base, QColor(0xec, 0xe9, 0xd8));
            pal.setColor(QPalette::Disabled, QPalette::Text, QColor(0xac, 0xa8, 0x99));
            w->setPalette(pal);
        }
        if (!qobject_cast<ClonkButton *>(w))
            w->setFont(font);
        if (cls == "ComboBox")
            rect.setHeight(COMBO_HEIGHT);
        w->setGeometry(rect);
        w->setObjectName(QString("ctrl_%1").arg(id));
        w->setEnabled(!(style & WS_DISABLED));
        w->setVisible(style & WS_VISIBLE);
        widgets_[id] = w;
    }
    // classic (unthemed) check boxes, radio buttons, combo boxes and up-down edits like Planet.exe on XP
    applyClassicButtons(parent);
}

QWidget *DialogBuilder::widget(int ctrl_id) const {
    auto it = widgets_.find(ctrl_id);
    if (it == widgets_.end()) {
        std::cerr << "DialogBuilder: dialog " << idd_ << " has no control " << ctrl_id << std::endl;
        return nullptr;
    }
    return it->second;
}
