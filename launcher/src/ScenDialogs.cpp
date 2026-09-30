#include "ScenDialogs.h"
#include "ClonkLauncher.h"
#include "HelpViewer.h"
#include "IDListCtrl.h"
#include "LauncherRes.h"
#include "ScenarioProperties.h"
#include "ScenStyle.h"
#include "Win3DWidgets.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRadioButton>
#include <QRegularExpression>
#include <QTreeWidget>

namespace {

constexpr int IDC_MAINTAB = 2380;
constexpr int SoundTab = 7009; // 0x1b61, ScenarioPropertiesDlg::ctor

void status(int str_id) {
    // MainPostStatusMsg
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->logStatus(LauncherRes::str(str_id));
}

bool registered() {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l && l->isRegistered();
}

// Title of a scenario: Title.txt in the launcher language, else [Head] Title (FUN_004047f0/FUN_004048f0)
QString scenarioTitle(const ItemPath &path, const Scen::Core &core) {
    const std::vector<uint8_t> t = GroupEdit::readFile(path, "Title.txt");
    if (!t.empty()) {
        const QString s = Scen::localizedText(t, Scen::language());
        if (!s.isEmpty())
            return s;
    }
    return core.title;
}

bool readCore(const ItemPath &path, Scen::Core &core, QString *maker = nullptr) {
    C4Group grp;
    if (!GroupEdit::open(path, grp) || !grp.hasEntry("Scenario.txt"))
        return false;
    core.load(C4TextDoc(grp.getFile("Scenario.txt")));
    if (maker)
        *maker = QString::fromStdString(grp.getMaker());
    return true;
}

bool writeCore(const ItemPath &path, const Scen::Core &core) {
    // FUN_0040d160: Scenario.txt with the decompiled core
    C4TextDoc doc(GroupEdit::readFile(path, "Scenario.txt"));
    core.save(doc);
    return GroupEdit::writeFile(path, "Scenario.txt", doc.toBytes());
}

} // namespace

// ======================================================================================= tab control

ScenTabCtrl::ScenTabCtrl(QWidget *placeholder) : QWidget(placeholder->parentWidget()) {
    setGeometry(placeholder->geometry());
    setObjectName(placeholder->objectName());
    placeholder->hide();
    setFont(LauncherRes::feFont()); // WM_SETFONT with the frontend font
    setFocusPolicy(Qt::StrongFocus);
}

void ScenTabCtrl::addTab(const QString &text, int param) {
    tabs_.push_back({text, param});
    update();
}

void ScenTabCtrl::setCurrent(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size()) || index == current_)
        return;
    current_ = index;
    update();
    emit changed(index);
}

int ScenTabCtrl::tabHeight() const {
    return QFontMetrics(font()).height() + 6;
}

QRect ScenTabCtrl::tabRect(int index) const {
    // text width + padding, at least the minimum tab width of the tab control
    const QFontMetrics fm(font());
    auto width = [&](int i) { return std::max(fm.horizontalAdvance(tabs_[i].text) + 12, 48); };
    int x = 2;
    for (int i = 0; i < index; ++i)
        x += width(i);
    return QRect(x, 2, width(index), tabHeight());
}

QRect ScenTabCtrl::pageRect() const {
    // display rect of the tab control, widened by 2 to the left, right and bottom (FUN_00428b20);
    // measured on the screenshots: the page starts 2 pixels below the pane edge
    const int top = 2 + tabHeight() + 2;
    return QRect(x() + 2, y() + top, width() - 4, height() - 2 - top);
}

void ScenTabCtrl::paintEvent(QPaintEvent *) {
    QPainter p(this);
    const int pane_top = 2 + tabHeight();
    const int w = width(), h = height();
    // pane (raised soft edge)
    p.setPen(Qt::white);
    p.drawLine(0, pane_top, w - 2, pane_top);
    p.drawLine(0, pane_top, 0, h - 2);
    p.setPen(SysColor::light());
    p.drawLine(1, pane_top + 1, w - 3, pane_top + 1);
    p.drawLine(1, pane_top + 1, 1, h - 3);
    p.setPen(SysColor::dkShadow());
    p.drawLine(0, h - 1, w - 1, h - 1);
    p.drawLine(w - 1, pane_top, w - 1, h - 1);
    p.setPen(SysColor::btnShadow());
    p.drawLine(1, h - 2, w - 2, h - 2);
    p.drawLine(w - 2, pane_top + 1, w - 2, h - 2);

    const QFontMetrics fm(font());
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        const bool sel = i == current_;
        QRect r = tabRect(i);
        if (sel)
            r.adjust(-2, -2, 2, 0);
        const int l = r.left(), t = r.top(), rr = r.right();
        const int b = sel ? pane_top + 1 : pane_top - 1;
        // TCS_OWNERDRAWFIXED: the item is filled with the page marble (bitmap 1019); the selected
        // item covers the pane edge
        p.drawTiledPixmap(QRect(l + 2, t + 2, r.width() - 4, b - t - 1), LauncherRes::bitmap(1019), QPoint(l + 2, t + 2));
        p.setPen(Qt::white);
        p.drawLine(l, t + 2, l, b);
        p.drawPoint(l + 1, t + 1);
        p.drawLine(l + 2, t, rr - 2, t);
        p.setPen(SysColor::light());
        p.drawLine(l + 1, t + 2, l + 1, b);
        p.drawLine(l + 2, t + 1, rr - 2, t + 1);
        p.setPen(SysColor::dkShadow());
        p.drawPoint(rr - 1, t + 1);
        p.drawLine(rr, t + 2, rr, b);
        p.setPen(SysColor::btnShadow());
        p.drawLine(rr - 1, t + 2, rr - 1, b);
        // text with a white shadow
        p.setFont(font());
        const QRect text_rect = sel ? r.adjusted(0, 0, 0, -1) : r.adjusted(0, 1, 0, 0);
        p.setPen(Qt::white);
        p.drawText(text_rect.translated(1, 1), Qt::AlignCenter, tabs_[i].text);
        p.setPen(Qt::black);
        p.drawText(text_rect, Qt::AlignCenter, tabs_[i].text);
        if (sel && hasFocus())
            drawFocusRect(p, r.adjusted(3, 3, -4, -3));
    }
}

void ScenTabCtrl::mousePressEvent(QMouseEvent *e) {
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i)
        if (tabRect(i).contains(e->pos())) {
            setFocus(Qt::MouseFocusReason);
            setCurrent(i);
            return;
        }
}

void ScenTabCtrl::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Left)
        setCurrent(current_ - 1);
    else if (e->key() == Qt::Key_Right)
        setCurrent(current_ + 1);
    else
        QWidget::keyPressEvent(e);
}

// ======================================================================================= properties

ScenarioPropertiesDlg::ScenarioPropertiesDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_SCENARIO_PROPERTIES, parent) {
    // ScenarioPropertiesDlg::OnErasebkgnd: the dark marble (bitmap 1020, app + 0xaed8)
    if (auto *tex = qobject_cast<ClonkTexturedWidget *>(body()))
        tex->setTexture(LauncherRes::resPath("bitmap", 1020));
    tabs_ = new ScenTabCtrl(control(IDC_MAINTAB));
    // ScenarioPropertiesDlg::OnInitDialog: tab texts and the page of each tab (lParam)
    tabs_->addTab(LauncherRes::str(50508), 0);
    tabs_->addTab(LauncherRes::str(50522), 1);
    tabs_->addTab(LauncherRes::str(50513), 2);
    tabs_->addTab(LauncherRes::str(50506), 4);
    tabs_->addTab(LauncherRes::str(50528), 3);
    connect(tabs_, &ScenTabCtrl::changed, this, [this](int index) {
        // ScenarioPropertiesDlg::OnNotifyMaintab
        Scen::playSound(SoundTab);
        showPage(index);
    });
    // FUN_00428980
    get<QAbstractButton>(1)->setText(LauncherRes::str(50026));
    get<QAbstractButton>(2)->setText(LauncherRes::str(50006));
    tabs_->setFocus();
    // WM_HELP (0x423dd0): popup help of the control, "?" in the title bar
    setWindowFlag(Qt::WindowContextHelpButtonHint, true);
    installContextHelp(this);
}

ScenarioPropertiesDlg::~ScenarioPropertiesDlg() = default;

bool ScenarioPropertiesDlg::load(const ItemPath &scenario, bool original, const QString &title) {
    // FUN_00428570
    path_ = scenario;
    original_ = original;
    ctx_.scenario = scenario;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    status(51804); // "Loading objects definitions..."
    Scen::Core probe;
    const bool readable = readCore(scenario, probe, &maker_);
    ctx_.defs.clear();
    if (ctx_.defs.loadForScenario(scenario, probe) == 0) {
        QApplication::restoreOverrideCursor();
        clonkMessage(this, LauncherRes::str(51114)); // "No object definitions available!"
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
    if (!readable) {
        QApplication::restoreOverrideCursor();
        return false;
    }
    core_ = probe;
    // C4Group::OpenAsFolder: unpacked groups have the maker "Open directory"
    if (GroupEdit::isDirectory(scenario.normalized()))
        maker_ = "Open directory";
    title_ = title.isEmpty() ? scenarioTitle(scenario, core_) : title;
    status(51805); // "Loading map..."
    // pages (created on demand in the original, data in before OnInitDialog)
    const QRect pr = tabs_->pageRect();
    pages_[0] = new ScenPageGame(&ctx_, body());
    pages_[1] = new ScenPageEquipment(&ctx_, body());
    pages_[2] = new ScenPageLandscape(&ctx_, body());
    pages_[3] = new ScenPageWeather(&ctx_, body());
    pages_[4] = new ScenPageEnvironment(&ctx_, body());
    for (ScenPage *p : pages_) {
        p->setGeometry(pr);
        p->load(core_);
        p->hide();
    }
    // FUN_00428980
    setWindowTitle(QString("%1 - %2").arg(LauncherRes::str(50525), title_));
    showPage(0);
    QApplication::restoreOverrideCursor();
    return true;
}

void ScenarioPropertiesDlg::showPage(int tab) {
    // FUN_00428b20: show the page of the tab, hide the others
    tabs_->setCurrent(tab);
    const int param = tabs_->param(tabs_->current());
    for (int i = 0; i < 5; ++i)
        if (pages_[i])
            pages_[i]->setVisible(i == param);
    tabs_->update();
}

void ScenarioPropertiesDlg::onOK() {
    // ScenarioPropertiesDlg::OnOK
    if (!registered()) {
        clonkMessage(this, LauncherRes::str(51122));
        return;
    }
    if (original_) {
        if (!clonkMessage(this, LauncherRes::str(51130).replace("%s", title_), MsgButtons::OKCancel))
            return;
        // a free file name in the working directory: Goldmine.c4s, Goldmine2.c4s, ...
        const QString game_dir = DefinitionDB::gameDir();
        const QString name = path_.name();
        QString target = name;
        const int dot = name.lastIndexOf('.');
        for (int n = 2; QFileInfo::exists(QDir(game_dir).filePath(target)); ++n)
            target = dot < 0 ? name + QString::number(n) : name.left(dot) + QString::number(n) + name.mid(dot);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const bool ok = GroupEdit::copy(path_, ItemPath(game_dir), target);
        QApplication::restoreOverrideCursor();
        if (!ok) {
            clonkMessage(this, LauncherRes::str(50606)); // "File modification failure."
            return;
        }
        path_ = ItemPath(QDir(game_dir).filePath(target));
    }
    storePages();
    accept();
}

void ScenarioPropertiesDlg::storePages() {
    // FUN_00428c40: Game, Equipment, Landscape, Weather, Environment
    pages_[0]->store(core_);
    pages_[1]->store(core_);
    pages_[2]->store(core_);
    pages_[3]->store(core_);
    pages_[4]->store(core_);
}

bool ScenarioPropertiesDlg::save() {
    // FUN_00428d40
    if (!writeCore(path_, core_))
        return false;
    // scenarios of other authors lose their description and title picture
    ClonkLauncher *l = ClonkLauncher::instance();
    const QString user = l ? QString::fromStdString(l->get_cfg("General\\Name", "")) : QString();
    if (maker_.compare("Open directory", Qt::CaseInsensitive) != 0 && maker_.compare(user, Qt::CaseInsensitive) != 0) {
        C4Group grp;
        if (GroupEdit::open(path_, grp)) {
            const QRegularExpression desc("^Desc.*\\.rtf$", QRegularExpression::CaseInsensitiveOption);
            const QRegularExpression pic("^Title.*\\.bmp$", QRegularExpression::CaseInsensitiveOption);
            for (const C4GroupEntry &e : grp.getEntries()) {
                const QString n = QString::fromStdString(e.name);
                if (desc.match(n).hasMatch() || pic.match(n).hasMatch())
                    GroupEdit::remove(path_.child(n));
            }
        }
    }
    return true;
}

// ======================================================================================= definitions

namespace {
enum DefIds { D_USES = 2342, D_ACTIVATED = 2205, D_LOCAL = 2209, D_SPECIFIED = 2214, D_LIST = 2202, D_USE_ACTIVATED = 2072 };
}

ScenarioDefinitionsDlg::ScenarioDefinitionsDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_SCENARIO_DEFINITIONS, parent) {
    // ScenarioDefinitionsDlg::OnInitDialog: report list view without header, state images of bitmap 1016
    QWidget *placeholder = control(D_LIST);
    list_ = new QTreeWidget(body());
    list_->setGeometry(placeholder->geometry());
    list_->setEnabled(placeholder->isEnabled());
    placeholder->hide();
    list_->setHeaderHidden(true);
    list_->setColumnCount(1);
    list_->setRootIsDecorated(false);
    list_->setIconSize(QSize(16, 16));
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setFont(LauncherRes::sysFont());
    list_->setFrameShape(QFrame::StyledPanel);
    list_->setFrameShadow(QFrame::Sunken);
    list_->setStyleSheet("QTreeWidget { background: white; } QTreeWidget:disabled { background: rgb(236, 233, 216); }");
    for (int id : {D_ACTIVATED, D_LOCAL, D_SPECIFIED}) {
        get<QAbstractButton>(id)->setAutoExclusive(true);
        connect(get<QAbstractButton>(id), &QAbstractButton::clicked, this, [this]() { updateEnable(); });
    }
    connect(get<QAbstractButton>(D_USE_ACTIVATED), &QAbstractButton::clicked, this, [this]() {
        // ScenarioDefinitionsDlg::OnUseActivatedClicked
        list_->clear();
        for (const QString &m : DefinitionDB::activatedModules())
            addItem(m);
    });
    // FUN_00427490 (texts)
    get<QAbstractButton>(D_USE_ACTIVATED)->setText(LauncherRes::str(50036));
    get<QAbstractButton>(1)->setText(LauncherRes::str(50026));
    get<QAbstractButton>(2)->setText(LauncherRes::str(50006));
    get<QLabel>(D_USES)->setText(LauncherRes::str(50296));
    applyClassicButtons(body());
    // WM_HELP (0x423dd0)
    setWindowFlag(Qt::WindowContextHelpButtonHint, true);
    installContextHelp(this);
    get<QAbstractButton>(D_ACTIVATED)->setText(LauncherRes::str(50156));
    get<QAbstractButton>(D_LOCAL)->setText(LauncherRes::str(50219));
    get<QAbstractButton>(D_SPECIFIED)->setText(LauncherRes::str(50332));
}

bool ScenarioDefinitionsDlg::load(const ItemPath &scenario, bool original, const QString &title) {
    // FUN_00427580
    path_ = scenario;
    original_ = original;
    if (!readCore(scenario, core_))
        return false;
    title_ = title.isEmpty() ? scenarioTitle(scenario, core_) : title;
    // FUN_004277c0
    int mode = 0;
    list_->clear();
    for (const QString &d : core_.definitions)
        if (!d.isEmpty()) {
            addItem(d);
            mode = 2;
        }
    if (core_.localOnly)
        mode = 1;
    get<QAbstractButton>(D_ACTIVATED)->setChecked(mode == 0);
    get<QAbstractButton>(D_LOCAL)->setChecked(mode == 1);
    get<QAbstractButton>(D_SPECIFIED)->setChecked(mode == 2);
    updateEnable();
    setWindowTitle(LauncherRes::str(50503) + " - " + title_);
    return true;
}

void ScenarioDefinitionsDlg::addItem(const QString &module) {
    // FUN_00427b30: state image 7 (check) if the module exists, 6 (cross) if not
    if (module.isEmpty())
        return;
    const ItemPath p = DefinitionDB::modulePath(module);
    C4Group grp;
    const bool exists = QFileInfo::exists(p.disk) && (p.subs.isEmpty() || GroupEdit::open(p, grp));
    const QPixmap strip = LauncherRes::bitmap(1016);
    QImage img = strip.copy((exists ? 6 : 5) * 16, 0, 16, 16).toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            if ((img.pixel(x, y) & 0xffffff) == 0xff00ff)
                img.setPixel(x, y, qRgba(0, 0, 0, 0));
    auto *item = new QTreeWidgetItem({module});
    item->setIcon(0, QIcon(QPixmap::fromImage(img)));
    list_->addTopLevelItem(item);
}

int ScenarioDefinitionsDlg::mode() const {
    if (get<QAbstractButton>(D_LOCAL)->isChecked())
        return 1;
    if (get<QAbstractButton>(D_SPECIFIED)->isChecked())
        return 2;
    return 0;
}

void ScenarioDefinitionsDlg::updateEnable() {
    // FUN_00427890 / OnLocalAndUserActivatedDefsClicked
    list_->setEnabled(mode() == 2);
    get<QAbstractButton>(D_USE_ACTIVATED)->setEnabled(mode() == 2);
}

void ScenarioDefinitionsDlg::onOK() {
    // ScenarioDefinitionsDlg::OnOK
    if (!registered()) {
        clonkMessage(this, LauncherRes::str(51122));
        return;
    }
    if (original_) {
        clonkMessage(this, LauncherRes::str(51108).replace("%s", title_));
        return;
    }
    // FUN_00427820
    switch (mode()) {
    case 0:
        core_.localOnly = 0;
        core_.definitions.fill(QString());
        break;
    case 1:
        core_.localOnly = 1;
        break;
    case 2:
        core_.localOnly = 0;
        core_.definitions.fill(QString());
        for (int i = 0; i < 10 && i < list_->topLevelItemCount(); ++i)
            core_.definitions[i] = list_->topLevelItem(i)->text(0).left(260);
        break;
    }
    accept();
}

bool ScenarioDefinitionsDlg::save() {
    // FUN_004278d0: the whole (converted) core is decompiled like in the properties dialog
    return writeCore(path_, core_);
}

// ======================================================================================= entry points

bool scenarioIsOriginal(const ItemPath &item_in) {
    // FUN_00430210: the item or any enclosing group carries the original flag
    const ItemPath item = item_in.normalized();
    ItemPath p(item.disk);
    for (int i = 0; i <= item.subs.size(); ++i) {
        if (i > 0)
            p = p.child(item.subs[i - 1]);
        if (GroupEdit::isDirectory(p))
            continue;
        C4Group grp;
        if (GroupEdit::open(p, grp) && grp.isOriginal())
            return true;
    }
    return false;
}

bool editScenarioProperties(QWidget *parent, const ItemPath &scenario, const QString &title) {
    return editScenarioProperties(parent, scenario, title, nullptr);
}

bool editScenarioProperties(QWidget *parent, const ItemPath &scenario, const QString &title, ItemPath *saved_to) {
    // ExplorerDlg properties handler (0x42e210) for scenarios
    ScenarioPropertiesDlg dlg(parent);
    if (!dlg.load(scenario, scenarioIsOriginal(scenario), title)) {
        clonkMessage(parent, LauncherRes::str(50606)); // "File modification failure."
        return false;
    }
    // FUN_00428ca0
    if (dlg.exec() != QDialog::Accepted)
        return false;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    status(51813); // "Updating scenario..."
    const bool ok = dlg.save();
    QApplication::restoreOverrideCursor();
    if (!ok)
        clonkMessage(parent, LauncherRes::str(50615)); // "Save failure."
    if (saved_to)
        *saved_to = dlg.path();
    return ok;
}

bool editScenarioDefinitions(QWidget *parent, const ItemPath &scenario, const QString &title) {
    ScenarioDefinitionsDlg dlg(parent);
    if (!dlg.load(scenario, scenarioIsOriginal(scenario), title)) {
        clonkMessage(parent, LauncherRes::str(50606));
        return false;
    }
    // FUN_00427730
    if (dlg.exec() != QDialog::Accepted)
        return false;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    status(51813);
    const bool ok = dlg.save();
    QApplication::restoreOverrideCursor();
    if (!ok)
        clonkMessage(parent, LauncherRes::str(50615));
    return ok;
}
