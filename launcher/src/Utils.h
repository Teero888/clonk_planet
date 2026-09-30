#pragma once

#include "LauncherCompat.h"
#include <string>
#include <vector>
#include <map>
#include <QImage>
#include <QPixmap>
#include "C4Group.h"

std::string rtfToHtml(const std::string &rtf_text);

QPixmap applyClonkTransparency(const QPixmap &pix);
QImage applyClonkTransparency(const QImage &img);
