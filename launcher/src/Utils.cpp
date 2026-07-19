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

std::map<std::string, std::map<std::string, std::string>> parseC4Text(const std::string &content) {
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::string current_section = "";
    
    size_t i = 0;
    while (i < content.size()) {
        size_t next_line = content.find('\n', i);
        std::string line = (next_line == std::string::npos) ? content.substr(i) : content.substr(i, next_line - i);
        i = (next_line == std::string::npos) ? content.size() : next_line + 1;
        
        while (!line.empty() && (line.back() == '\r' || std::isspace(line.back()))) line.pop_back();
        size_t ls = 0;
        while (ls < line.size() && std::isspace(line[ls])) ls++;
        line = line.substr(ls);
        
        if (line.empty() || line[0] == ';') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        
        if (!current_section.empty()) {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string k = line.substr(0, eq);
                std::string v = line.substr(eq + 1);
                while (!k.empty() && std::isspace(k.back())) k.pop_back();
                size_t ks = 0;
                while (ks < k.size() && std::isspace(k[ks])) ks++;
                k = k.substr(ks);
                
                while (!v.empty() && std::isspace(v.back())) v.pop_back();
                size_t vs = 0;
                while (vs < v.size() && std::isspace(v[vs])) vs++;
                v = v.substr(vs);
                
                sections[current_section][k] = v;
            }
        }
    }
    return sections;
}

std::string updateC4Text(const std::string &content, const std::string &section_name, const std::map<std::string, std::string> &new_values) {
    std::vector<std::string> lines;
    size_t i = 0;
    while (i < content.size()) {
        size_t next_line = content.find('\n', i);
        std::string line = (next_line == std::string::npos) ? content.substr(i) : content.substr(i, next_line - i);
        i = (next_line == std::string::npos) ? content.size() : next_line + 1;
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        lines.push_back(line);
    }

    std::vector<std::string> new_lines;
    bool in_section = false;
    std::map<std::string, std::string> new_values_lower;
    for (const auto &pair : new_values) {
        std::string k = pair.first;
        std::transform(k.begin(), k.end(), k.begin(), ::tolower);
        new_values_lower[k] = pair.first;
    }
    std::vector<std::string> found_keys;

    for (const auto &line : lines) {
        std::string s_line = line;
        while (!s_line.empty() && std::isspace(s_line.back())) s_line.pop_back();
        size_t ls = 0;
        while (ls < s_line.size() && std::isspace(s_line[ls])) ls++;
        s_line = s_line.substr(ls);

        if (s_line == "[" + section_name + "]") {
            in_section = true;
            new_lines.push_back(line);
            continue;
        }

        if (s_line.size() >= 2 && s_line[0] == '[' && s_line.back() == ']') {
            if (in_section) {
                for (const auto &pair : new_values) {
                    std::string kl = pair.first;
                    std::transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
                    if (std::find(found_keys.begin(), found_keys.end(), kl) == found_keys.end()) {
                        new_lines.push_back(pair.first + "=" + pair.second);
                    }
                }
            }
            in_section = false;
            new_lines.push_back(line);
            continue;
        }

        if (in_section && s_line.find('=') != std::string::npos) {
            size_t eq = s_line.find('=');
            std::string key = s_line.substr(0, eq);
            while (!key.empty() && std::isspace(key.back())) key.pop_back();
            size_t ks = 0;
            while (ks < key.size() && std::isspace(key[ks])) ks++;
            key = key.substr(ks);

            std::string key_lower = key;
            std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);

            auto it = new_values_lower.find(key_lower);
            if (it != new_values_lower.end()) {
                std::string orig_key = it->second;
                new_lines.push_back(orig_key + "=" + new_values.at(orig_key));
                found_keys.push_back(key_lower);
            } else {
                new_lines.push_back(line);
            }
        } else {
            new_lines.push_back(line);
        }
    }

    if (in_section) {
        for (const auto &pair : new_values) {
            std::string kl = pair.first;
            std::transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
            if (std::find(found_keys.begin(), found_keys.end(), kl) == found_keys.end()) {
                new_lines.push_back(pair.first + "=" + pair.second);
            }
        }
    }

    bool found_sec = false;
    for (const auto &line : lines) {
        std::string s_line = line;
        while (!s_line.empty() && std::isspace(s_line.back())) s_line.pop_back();
        size_t ls = 0;
        while (ls < s_line.size() && std::isspace(s_line[ls])) ls++;
        s_line = s_line.substr(ls);
        if (s_line == "[" + section_name + "]") {
            found_sec = true;
            break;
        }
    }
    if (!found_sec) {
        if (!new_lines.empty() && !new_lines.back().empty()) {
            new_lines.push_back("");
        }
        new_lines.push_back("[" + section_name + "]");
        for (const auto &pair : new_values) {
            new_lines.push_back(pair.first + "=" + pair.second);
        }
    }

    std::string result = "";
    for (const auto &line : new_lines) {
        result += line + "\r\n";
    }
    return result;
}

std::string getGroupTitle(const C4Group &grp, const std::string &language) {
    std::string title = "Unknown";
    std::vector<uint8_t> title_data = grp.getFile("Title.txt");
    if (title_data.empty()) {
        title_data = grp.getFile("Names.txt");
    }

    if (!title_data.empty()) {
        std::string content(reinterpret_cast<const char*>(title_data.data()), title_data.size());
        size_t i = 0;
        std::vector<std::string> lines;
        while (i < content.size()) {
            size_t next_line = content.find('\n', i);
            std::string line = (next_line == std::string::npos) ? content.substr(i) : content.substr(i, next_line - i);
            i = (next_line == std::string::npos) ? content.size() : next_line + 1;
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
            lines.push_back(line);
        }

        std::string pref_prefix = language + ":";
        bool found = false;
        for (const auto &line : lines) {
            if (line.rfind(pref_prefix, 0) == 0) {
                title = line.substr(pref_prefix.size());
                found = true;
                break;
            }
        }
        if (!found) {
            for (const auto &line : lines) {
                if (line.find(':') != std::string::npos) {
                    if (line.rfind("DE:", 0) != 0 && line.rfind("US:", 0) != 0) {
                        title = line.substr(line.find(':') + 1);
                        found = true;
                        break;
                    }
                } else if (!line.empty()) {
                    title = line;
                    found = true;
                    break;
                }
            }
        }
    }
    size_t dot = title.rfind('.');
    if (dot != std::string::npos) {
        title = title.substr(0, dot);
    }
    return title;
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
