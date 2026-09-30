#pragma once

// Clonk component text files (Scenario.txt, Player.txt, DefCore.txt, Title.txt, ...):
//   [Section]
//   Key=Value
// Editing keeps comments, unknown keys and the order of everything; new keys are appended to their
// section, new sections to the end. Keys and sections are matched case sensitive like C4Compiler.

#include <QString>
#include <QStringList>
#include <string>
#include <vector>

class C4TextDoc {
public:
    C4TextDoc() = default;
    explicit C4TextDoc(const std::string &content) { parse(content); }
    explicit C4TextDoc(const std::vector<uint8_t> &content) { parse(std::string(content.begin(), content.end())); }

    void parse(const std::string &content);
    std::string toString() const;
    std::vector<uint8_t> toBytes() const;

    bool has(const QString &section, const QString &key) const;
    QString get(const QString &section, const QString &key, const QString &def = {}) const;
    int getInt(const QString &section, const QString &key, int def = 0) const;
    // comma separated integers ("Climate=50,10,0,100")
    std::vector<int> getInts(const QString &section, const QString &key) const;

    void set(const QString &section, const QString &key, const QString &value);
    void setInt(const QString &section, const QString &key, int value);
    void remove(const QString &section, const QString &key);

    QStringList sections() const;
    QStringList keys(const QString &section) const;

private:
    struct Line {
        QString text;    // original text (comments, blank lines, section headers)
        QString section; // section the line belongs to
        QString key;     // empty for non key lines
        QString value;
        bool dirty = false; // written as key=value instead of the original text
    };
    std::vector<Line> lines_;
    bool crlf_ = true;

    int find(const QString &section, const QString &key) const;
};
