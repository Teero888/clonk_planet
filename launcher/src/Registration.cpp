#include "Registration.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "RegistrationDlg.h"
#include "ClonkDialog.h"

#include <QByteArray>
#include <QStringList>
#include <cstdlib>
#include <string>

namespace Registration {

namespace {

// ---------------------------------------------------------------------------------------------
// Security strings of Planet.exe (C4Config fields after the registry values, filled by 0x404cc0).
// They are stored obfuscated in the binary and decoded with the string table decoder (0x404990,
// key at 0x4791bc); 0x404a20 skips the 0xA3 marker. Stored here exactly as in the binary.

const char OBF_KEY[] = "_D.lp/8f3_ 3 ] =h16%2"; // 0x4791bc

// port of DecodeResStr (0x404990)
QByteArray decode(const QByteArray &in) {
    QByteArray s = in;
    int ki = 0, skip = 0;
    const int key_len = sizeof(OBF_KEY) - 1;
    for (int i = 0; i < s.size(); ++i) {
        const int c = static_cast<signed char>(s[i]);
        if (c == '\\') {
            skip = 2;
        } else if (skip == 0) {
            int v = c;
            if (v == -8) // 0xf8 stands for a backslash
                v = s[i] = '\\';
            if (v > 0x2f && v < 0x7b) {
                int k = OBF_KEY[ki];
                k = k < 0x30 ? 0x30 : (k > 0x7a ? 0x7a : k);
                int d = v - k + 0x30;
                if (d < 0x30)
                    d = v - k + 0x7b;
                s[i] = static_cast<char>(d);
                if (++ki >= key_len)
                    ki = 0;
            }
        }
        if (skip)
            --skip;
    }
    return s.startsWith('\xa3') ? s.mid(1) : s; // 0x404a20 returns the string after the marker
}

QByteArray secKey() { return decode("\xa3-a..Hndl.-dnP./mOsom]q3Zojfg"); }            // +0x8dd4 (0x479290)
QByteArray secKeyOld() { return decode("\xa3" "18ndiezLqKrlfH"); }                      // +0x91d5 (0x479280)
QByteArray masterValueName() { return decode("\xa3" "1useZrLPyIlrpGrP\xf8" "ek"); }    // +0x99d7 (0x479258)
QByteArray masterValue() { return decode("\xa3gF7pv1?h;b"); }                            // +0x9dd8 (0x47924c)
QByteArray masterNames() {                                                               // +0xa1d9 (0x47921c)
    return decode("\xa3" "1ute]e0 xhRdhrhMn_l0s YMwhZiitW>6egWQls 1fyiiR");
}
QByteArray blockedNames() {                                                              // +0xa5da (0x4791e8)
    return decode("\xa3" "6yd HdlnqMEmhs pl1[lAMcH hu]^pC8dKif MKkrsYgx JE7");
}
QByteArray forbiddenChars() { return decode("\xa3!\xa7$%&/()lS{[N}#*+"); }              // +0xa9db (0x4791d4)

// ---------------------------------------------------------------------------------------------

// 0x403dc0: sum of char * index (signed chars as in the original)
int nameChecksum(const QByteArray &name) {
    int sum = 0;
    for (int i = 0; i < name.size(); ++i)
        sum += static_cast<signed char>(name[i]) * i;
    return sum;
}

// the common tail of 0x403df0 / 0x403cc0: xor with the key, reduce to digits
QByteArray finishCode(unsigned char buf[10], const QByteArray &key, char zero_replacement) {
    for (int i = 0; i < 10; ++i)
        buf[i] ^= static_cast<unsigned char>(key[i % key.size()]);
    QByteArray out(10, '0');
    for (int i = 0; i < 10; ++i)
        out[i] = static_cast<char>(std::abs(static_cast<int>(static_cast<signed char>(buf[i]))) % 10 + '0');
    if (out[0] == '0')
        out[0] = zero_replacement;
    return out;
}

// 0x403df0
QByteArray code(const QByteArray &name, const QByteArray &key) {
    if (name.isEmpty() || key.isEmpty())
        return {};
    unsigned char buf[10];
    // buf[9..0] = name[(k + 5) % len]
    for (int k = 0; k < 10; ++k)
        buf[9 - k] = static_cast<unsigned char>(name[(k + 5) % name.size()]);
    unsigned char b = static_cast<unsigned char>(nameChecksum(name));
    for (int i = 0; i < 10; ++i) {
        buf[i] ^= b;
        b = static_cast<unsigned char>(b + 0x2c);
    }
    return finishCode(buf, key, '3');
}

// 0x403cc0 (code of the previous version)
QByteArray codeOld(const QByteArray &name, const QByteArray &key) {
    if (name.isEmpty() || key.isEmpty())
        return {};
    unsigned char sum = 0;
    for (char c : name)
        sum = static_cast<unsigned char>(sum + static_cast<unsigned char>(c));
    unsigned char buf[10];
    for (int k = 0; k < 10; ++k)
        buf[9 - k] = static_cast<unsigned char>(name[k % name.size()]);
    for (int i = 0; i < 10; ++i)
        buf[i] ^= static_cast<unsigned char>(i * '!' + sum);
    return finishCode(buf, key, '5');
}

// SEqualNoCase (0x434350 with length -1)
bool equalNoCase(const QByteArray &a, const QByteArray &b) {
    return QString::fromLatin1(a).compare(QString::fromLatin1(b), Qt::CaseInsensitive) == 0;
}

// SCopySegment loop over a ';' separated list
QList<QByteArray> segments(const QByteArray &list) { return list.split(';'); }

QByteArray latin1(const std::string &s) { return QString::fromStdString(s).toLatin1(); }

// C4Config::IsRegistered (0x404e10)
bool isRegisteredImpl(const QByteArray &name, const QByteArray &code_in, bool old_version) {
    const QByteArray key = old_version ? secKeyOld() : secKey();
    if (name.isEmpty() || code_in.isEmpty() || key.isEmpty())
        return false;
    const QByteArray expected = old_version ? codeOld(name, key) : code(name, key);
    if (code_in != expected)
        return false;
    // the name has to consist of at least two words
    if (!name.contains(' '))
        return false;
    // and must not contain any of the forbidden characters
    for (char c : forbiddenChars())
        if (name.contains(c))
            return false;
    // blocked names
    for (const QByteArray &n : segments(blockedNames()))
        if (equalNoCase(name, n))
            return false;
    // names of the authors are only valid with the developer code in the registry
    // (HKCU\Software\RedWolf Design\Clonk 4\Developer\MasterDeveloperCode)
    const std::string master = ClonkLauncher::instance()
                                   ? ClonkLauncher::instance()->get_cfg(
                                         "Developer\\" + masterValueName().toStdString(), "")
                                   : std::string();
    for (const QByteArray &n : segments(masterNames()))
        if (equalNoCase(name, n) && latin1(master) != masterValue())
            return false;
    return true;
}

} // namespace

bool isValid(const QString &name, const QString &code_str, bool old_version) {
    return isRegisteredImpl(name.toLatin1(), code_str.toLatin1(), old_version);
}

} // namespace Registration

bool isRegistered() {
    ClonkLauncher *l = ClonkLauncher::instance();
    if (!l)
        return false;
    // C4Config::Registered: IsRegistered(General.Name, General.Code, SecKey, FALSE)
    return Registration::isRegisteredImpl(Registration::latin1(l->get_cfg("General\\Name", "")),
                                          Registration::latin1(l->get_cfg("General\\Code", "")), false);
}

QString mainWindowTitle() {
    QString title = LauncherRes::str(50507); // "Clonk Planet"
    if (!isRegistered())
        title += LauncherRes::str(50349); // " - NOT REGISTERED"
    return title;
}

bool runRegistrationDialog(QWidget *parent) {
    // ExplorerDlg::On4006Clicked + FUN_00426f40 (evaluation after DoModal == IDOK)
    ClonkLauncher *l = ClonkLauncher::instance();
    RegistrationDlg dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    const QString name = dlg.name(), code = dlg.code();
    if (code.isEmpty()) // returns IDCANCEL: nothing is saved
        return false;
    bool registered = false;
    if (Registration::isValid(name, code, false)) {
        if (l) {
            l->set_cfg("General\\Name", name.toStdString());
            l->set_cfg("General\\Code", code.toStdString());
        }
        registered = true;
        clonkMessage(parent, LauncherRes::str(50288)); // "Thank you for your registration."
    } else if (Registration::isValid(name, code, true)) {
        clonkMessage(parent, LauncherRes::str(51132)); // "... valid for an older version ..."
    } else {
        clonkMessage(parent, LauncherRes::str(50252)); // "The registration code is invalid."
    }
    // IDOK in all three cases: the config is saved (FUN_00404b00) and the main window updates its
    // title (0x413e10); a changed player name refreshes the tree (VK_F5)
    if (l)
        l->saveConfig();
    return registered;
}
