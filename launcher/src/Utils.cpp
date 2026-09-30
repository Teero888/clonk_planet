#include "Utils.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cctype>

std::string rtfToHtml(const std::string &text) {
    if (text.rfind("{\\rtf", 0) != 0) {
        return text;
    }
    
    int skip_level = 0;
    std::string html = "";
    size_t i = 0;
    bool bold = false;
    bool italic = false;
    bool strike = false;
    bool underline = false;
    bool curr_size_set = false;
    int curr_size = 0;

    while (i < text.size()) {
        char c = text[i];
        if (c == '{') {
            i++;
            if (skip_level > 0) {
                skip_level++;
            } else {
                size_t j = i;
                while (j < text.size() && (text[j] == '\n' || text[j] == '\r' || text[j] == ' ')) j++;
                if (j < text.size() && text[j] == '\\') {
                    j++;
                    std::string tag = "";
                    if (j < text.size() && text[j] == '*') {
                        tag = "*";
                    } else {
                        while (j < text.size() && std::isalpha(static_cast<unsigned char>(text[j]))) {
                            tag += text[j];
                            j++;
                        }
                    }
                    if (tag == "fonttbl" || tag == "colortbl" || tag == "stylesheet" || tag == "info" || tag == "*") {
                        skip_level = 1;
                    }
                }
            }
        } else if (c == '}') {
            i++;
            if (skip_level > 0) skip_level--;
        } else if (c == '\\') {
            i++;
            std::string tag = "";
            if (i < text.size() && !std::isalpha(static_cast<unsigned char>(text[i]))) {
                tag = text[i];
                i++;
                if (tag == "'") {
                    if (i + 2 <= text.size()) {
                        if (skip_level == 0) {
                            try {
                                std::string hex = text.substr(i, 2);
                                int code = std::stoi(hex, nullptr, 16);
                                html += static_cast<char>(code);
                            } catch (...) {}
                        }
                        i += 2;
                    }
                    continue;
                }
            } else {
                while (i < text.size() && std::isalpha(static_cast<unsigned char>(text[i]))) {
                    tag += text[i];
                    i++;
                }
                std::string arg = "";
                if (i < text.size() && text[i] == '-') {
                    arg += '-';
                    i++;
                }
                while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
                    arg += text[i];
                    i++;
                }
                if (i < text.size() && text[i] == ' ') i++;
                
                if (skip_level == 0) {
                    if (tag == "par" || tag == "line") {
                        html += "<br>";
                    } else if (tag == "tab") {
                        html += "&nbsp;&nbsp;&nbsp;&nbsp;";
                    } else if (tag == "b") {
                        if (arg == "0") {
                            if (bold) { html += "</b>"; bold = false; }
                        } else {
                            if (!bold) { html += "<b>"; bold = true; }
                        }
                    } else if (tag == "i") {
                        if (arg == "0") {
                            if (italic) { html += "</i>"; italic = false; }
                        } else {
                            if (!italic) { html += "<i>"; italic = true; }
                        }
                    } else if (tag == "strike") {
                        if (arg == "0") {
                            if (strike) { html += "</s>"; strike = false; }
                        } else {
                            if (!strike) { html += "<s>"; strike = true; }
                        }
                    } else if (tag == "ul" || tag == "ulnone") {
                        if (tag == "ulnone" || arg == "0") {
                            if (underline) { html += "</u>"; underline = false; }
                        } else {
                            if (!underline) { html += "<u>"; underline = true; }
                        }
                    } else if (tag == "fs" && !arg.empty()) {
                        try {
                            int size_pt = std::stoi(arg) / 2;
                            if (curr_size_set) {
                                html += "</span>";
                            }
                            html += "<span style=\"font-size: " + std::to_string(size_pt) + "pt\">";
                            curr_size = size_pt;
                            curr_size_set = true;
                        } catch (...) {}
                    }
                }
            }
        } else if (c == '\r' || c == '\n') {
            i++;
        } else {
            if (skip_level == 0) {
                if (c == '<') html += "&lt;";
                else if (c == '>') html += "&gt;";
                else html += c;
            }
            i++;
        }
    }
    
    if (curr_size_set) html += "</span>";
    if (bold) html += "</b>";
    if (italic) html += "</i>";
    if (strike) html += "</s>";
    if (underline) html += "</u>";
    
    return html;
}

QImage applyClonkTransparency(const QImage &img) {
    if (img.isNull()) return img;
    QImage out = img.convertToFormat(QImage::Format_ARGB32);
    
    uint32_t magic_colors[5] = {
        0xff00ff, // Magenta
        0xc0c4fc, // Clonk Blue
        0x000000, // Black
        0x008080, // Teal
        0xffff00  // Yellow
    };

    uint32_t bg_rgb = out.pixel(0, 0) & 0xffffff;

    std::vector<uint32_t> to_mask;
    to_mask.push_back(magic_colors[0]);
    to_mask.push_back(magic_colors[1]);

    for (int i = 0; i < 5; ++i) {
        if (bg_rgb == magic_colors[i]) {
            if (bg_rgb != magic_colors[0] && bg_rgb != magic_colors[1]) {
                to_mask.push_back(bg_rgb);
            }
            break;
        }
    }

    for (int y = 0; y < out.height(); ++y) {
        for (int x = 0; x < out.width(); ++x) {
            uint32_t rgb = out.pixel(x, y) & 0xffffff;
            bool should_mask = false;
            for (uint32_t m : to_mask) {
                if (rgb == m) {
                    should_mask = true;
                    break;
                }
            }
            if (should_mask) {
                out.setPixel(x, y, 0);
            }
        }
    }
    return out;
}

QPixmap applyClonkTransparency(const QPixmap &pix) {
    if (pix.isNull()) return pix;
    return QPixmap::fromImage(applyClonkTransparency(pix.toImage()));
}
