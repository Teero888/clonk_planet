#include "C4TextDoc.h"

void C4TextDoc::parse(const std::string &content) {
    lines_.clear();
    const QString text = QString::fromLatin1(content.data(), static_cast<int>(content.size()));
    crlf_ = text.contains("\r\n") || text.isEmpty();
    QString section;
    const QStringList raw = text.split('\n');
    for (int i = 0; i < raw.size(); ++i) {
        QString l = raw[i];
        if (l.endsWith('\r'))
            l.chop(1);
        if (i == raw.size() - 1 && l.isEmpty())
            break; // trailing newline
        Line line;
        line.text = l;
        const QString t = l.trimmed();
        if (t.startsWith('[') && t.endsWith(']')) {
            section = t.mid(1, t.size() - 2);
        } else if (!t.isEmpty() && !t.startsWith(';') && !t.startsWith('#') && t.contains('=')) {
            const int eq = t.indexOf('=');
            line.key = t.left(eq).trimmed();
            line.value = t.mid(eq + 1).trimmed();
        }
        line.section = section;
        lines_.push_back(line);
    }
}

std::string C4TextDoc::toString() const {
    const QString nl = crlf_ ? "\r\n" : "\n";
    QString out;
    for (const Line &l : lines_) {
        out += (l.key.isEmpty() || !l.dirty) ? l.text : l.key + "=" + l.value;
        out += nl;
    }
    const QByteArray b = out.toLatin1();
    return std::string(b.constData(), b.size());
}

std::vector<uint8_t> C4TextDoc::toBytes() const {
    const std::string s = toString();
    return std::vector<uint8_t>(s.begin(), s.end());
}

int C4TextDoc::find(const QString &section, const QString &key) const {
    for (size_t i = 0; i < lines_.size(); ++i)
        if (lines_[i].section == section && lines_[i].key == key)
            return static_cast<int>(i);
    return -1;
}

bool C4TextDoc::has(const QString &section, const QString &key) const {
    return find(section, key) >= 0;
}

QString C4TextDoc::get(const QString &section, const QString &key, const QString &def) const {
    const int i = find(section, key);
    return i < 0 ? def : lines_[i].value;
}

int C4TextDoc::getInt(const QString &section, const QString &key, int def) const {
    const int i = find(section, key);
    if (i < 0)
        return def;
    bool ok = false;
    const int v = lines_[i].value.section(',', 0, 0).trimmed().toInt(&ok);
    return ok ? v : def;
}

std::vector<int> C4TextDoc::getInts(const QString &section, const QString &key) const {
    std::vector<int> out;
    const int i = find(section, key);
    if (i < 0)
        return out;
    for (const QString &p : lines_[i].value.split(',', Qt::SkipEmptyParts))
        out.push_back(p.trimmed().toInt());
    return out;
}

void C4TextDoc::set(const QString &section, const QString &key, const QString &value) {
    const int i = find(section, key);
    if (i >= 0) {
        if (lines_[i].value != value) {
            lines_[i].value = value;
            lines_[i].dirty = true;
        }
        return;
    }
    // append after the last line of the section (before trailing blank lines)
    int last = -1, header = -1;
    for (size_t j = 0; j < lines_.size(); ++j) {
        if (lines_[j].section != section)
            continue;
        if (header < 0)
            header = static_cast<int>(j);
        if (!lines_[j].text.trimmed().isEmpty() || !lines_[j].key.isEmpty())
            last = static_cast<int>(j);
    }
    Line line;
    line.section = section;
    line.key = key;
    line.value = value;
    line.dirty = true;
    if (header < 0) {
        if (!lines_.empty() && !lines_.back().text.trimmed().isEmpty())
            lines_.push_back(Line{QString(), lines_.back().section, {}, {}});
        lines_.push_back(Line{"[" + section + "]", section, {}, {}});
        lines_.push_back(line);
        return;
    }
    lines_.insert(lines_.begin() + last + 1, line);
}

void C4TextDoc::setInt(const QString &section, const QString &key, int value) {
    set(section, key, QString::number(value));
}

void C4TextDoc::remove(const QString &section, const QString &key) {
    const int i = find(section, key);
    if (i >= 0)
        lines_.erase(lines_.begin() + i);
}

QStringList C4TextDoc::sections() const {
    QStringList out;
    for (const Line &l : lines_)
        if (!l.section.isEmpty() && !out.contains(l.section))
            out << l.section;
    return out;
}

QStringList C4TextDoc::keys(const QString &section) const {
    QStringList out;
    for (const Line &l : lines_)
        if (l.section == section && !l.key.isEmpty())
            out << l.key;
    return out;
}

