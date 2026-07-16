#pragma once

#include "LauncherCompat.h"
#include <string>
#include <vector>
#include <map>
#include <QImage>
#include <QPixmap>
#include "C4Group.h"

std::string rtfToHtml(const std::string &rtf_text);

std::map<std::string, std::map<std::string, std::string>> parseC4Text(const std::string &content);

std::string updateC4Text(const std::string &content, const std::string &section_name, const std::map<std::string, std::string> &new_values);

std::string getGroupTitle(const C4Group &grp, const std::string &language = "US");

QPixmap applyClonkTransparency(const QPixmap &pix);
QImage applyClonkTransparency(const QImage &img);
