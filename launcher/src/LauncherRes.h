#pragma once

// Access to the resources extracted from the original Planet.exe (launcher/data):
//   data/strings.json - the launcher string table (German/English)
//   data/dialogs.json - the dialog templates (control geometry in dialog units, text string ids)

#include <QFont>
#include <QJsonArray>
#include <QPixmap>
#include <QJsonObject>
#include <QRect>
#include <QString>
#include <string>
#include <vector>

namespace LauncherRes {

// Dialog template ids of the original Planet.exe
enum DialogId {
    IDD_EXPLORER = 3002, // main window
    IDD_IDSELECT = 3003,
    IDD_PROMPT = 3004,
    IDD_LICENSE = 3006,
    IDD_LOG = 3007,
    IDD_MESSAGE = 3008,
    IDD_NETSTART = 3009,
    IDD_PAGE_DEVELOPER = 3010,
    IDD_PAGE_EDITOR = 3011,
    IDD_SCEN_ENVIRONMENT = 3012,
    IDD_SCEN_GAME = 3013,
    IDD_PAGE_GAMEPAD = 3014,
    IDD_PAGE_GRAPHICS = 3015,
    IDD_PAGE_KEYBOARD = 3016,
    IDD_SCEN_LANDSCAPE = 3017,
    IDD_PAGE_NETWORK = 3018,
    IDD_SCEN_EQUIPMENT = 3019,
    IDD_PAGE_PROGRAM = 3020,
    IDD_PAGE_SOUND = 3021,
    IDD_SCEN_WEATHER = 3022,
    IDD_PLAYER_PROPERTIES = 3023,
    IDD_EVALUATION = 3024,
    IDD_QUICKSTART = 3025,
    IDD_REGISTRATION = 3026,
    IDD_SCENARIO_PROPERTIES = 3027,
    IDD_SCENARIO_DEFINITIONS = 3028,
    IDD_NEW = 3029,
};

// Loads data/strings.json and data/dialogs.json from the given directory.
bool load(const QString &data_dir);

// "US" or "DE"
void setLanguage(const std::string &lang);

// Equivalent of the original LoadResStr: ids are the German ids (50000-54999),
// the English text is looked up automatically when the language is "US".
QString str(int id);

// Size of a dialog template in pixels.
QSize dialogSize(int idd);

// Geometry of a dialog control in pixels (same conversion as MapDialogRect with MS Sans Serif 8).
QRect controlRect(int idd, int ctrl_id);

// Localised text of a dialog control: the string the original code writes into it, or the
// template text if it is not localised. Empty if the control does not exist.
QString controlText(int idd, int ctrl_id);

// Raw template of a dialog (style, x, y, w, h, title, font, controls, handlers, ...)
QJsonObject dialogTemplate(int idd);

// Raw template data of all controls of a dialog, in template order
QJsonArray dialogControls(int idd);

// Raw template data of one control (id, cls, key, x, y, w, h, style, exstyle, text_id)
QJsonObject control(int idd, int ctrl_id);

// Original resources extracted to data/res (bitmap/<id>.bmp, wave/<id>.wav, binary/<id>.c4g|bin,
// text/<id>.txt, avi/<id>.png sprite sheets)
QString resPath(const QString &type, int id);
QPixmap bitmap(int id);
std::vector<uint8_t> binary(int id);
QString text(int id);

// Frontend sound effects (WAVE resources 7000-7009), only if enabled (Sound\FESamples)
void setSoundsEnabled(bool enabled);
void playSound(int wave_id);

// Fonts: the frontend font (FEFontName/FEFontSize, used by the skinned dialogs) and the system
// dialog font (MS Sans Serif 8)
void setFonts(const QString &fe_family, int fe_point_size, const QString &sys_family);
QFont feFont();
QFont sysFont();

// Dialog units -> pixels
int dluToPxX(int x);
int dluToPxY(int y);

} // namespace LauncherRes
