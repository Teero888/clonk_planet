import struct
import pefile
import sys

def read_utf16_str(data, offset):
    s = ""
    while offset < len(data):
        val = struct.unpack_from("<H", data, offset)[0]
        offset += 2
        if val == 0:
            break
        s += chr(val)
    return s, offset

def parse_class_or_title(data, offset):
    if offset >= len(data):
        return "", offset
    val = struct.unpack_from("<H", data, offset)[0]
    if val == 0xFFFF:
        # Predefined resource/class ID
        id_val = struct.unpack_from("<H", data, offset + 2)[0]
        # Common controls
        class_names = {
            0x0080: "Button",
            0x0081: "Edit",
            0x0082: "Static",
            0x0083: "List Box",
            0x0084: "Scroll Bar",
            0x0085: "Combo Box"
        }
        name = class_names.get(id_val, f"ID:{id_val}")
        return name, offset + 4
    elif val == 0:
        return "", offset + 2
    else:
        return read_utf16_str(data, offset)

def parse_dialog(data):
    if len(data) < 18:
        return None

    # Check version
    signature, dlgVer = struct.unpack_from("<HH", data, 0)
    is_ex = (signature == 0xFFFF and dlgVer == 0x0001)

    offset = 0
    if is_ex:
        # DLGTEMPLATEEX
        helpID, exStyle, style, cDlgItems, x, y, cx, cy = struct.unpack_from("<IiiHhhhh", data, 4)
        offset = 26
    else:
        # DLGTEMPLATE
        style, exStyle, cDlgItems, x, y, cx, cy = struct.unpack_from("<Iihhhhh", data, 0)
        offset = 18
        helpID = 0

    # Menu
    menu_val = struct.unpack_from("<H", data, offset)[0]
    if menu_val == 0xFFFF:
        menu_id = struct.unpack_from("<H", data, offset + 2)[0]
        menu = f"MenuID:{menu_id}"
        offset += 4
    elif menu_val == 0:
        menu = "None"
        offset += 2
    else:
        menu, offset = read_utf16_str(data, offset)

    # Class
    cls, offset = parse_class_or_title(data, offset)

    # Title
    title, offset = read_utf16_str(data, offset)

    # Font
    pointsize = 0
    fontname = ""
    # DS_SETFONT is 0x40
    if (style & 0x40) != 0:
        pointsize = struct.unpack_from("<H", data, offset)[0]
        offset += 2
        if is_ex:
            weight, italic, charset = struct.unpack_from("<HBB", data, offset)
            offset += 4
        fontname, offset = read_utf16_str(data, offset)

    items = []
    # DLGITEMTEMPLATE starts on DWORD boundary
    offset = (offset + 3) & ~3

    for _ in range(cDlgItems):
        if offset >= len(data):
            break
        if is_ex:
            # DLGITEMTEMPLATEEX
            helpID_item, exStyle_item, style_item, x_item, y_item, cx_item, cy_item, id_item = struct.unpack_from("<IIihhhhh", data, offset)
            offset += 24
        else:
            # DLGITEMTEMPLATE
            style_item, exStyle_item, x_item, y_item, cx_item, cy_item, id_item = struct.unpack_from("<IIhhhhh", data, offset)
            offset += 18

        cls_item, offset = parse_class_or_title(data, offset)
        title_item, offset = parse_class_or_title(data, offset)

        extra_size = struct.unpack_from("<H", data, offset)[0]
        if extra_size == 0xFFFF:
            offset += 4
        elif extra_size > 0:
            offset += 2 + extra_size
        else:
            offset += 2

        # Align to DWORD
        offset = (offset + 3) & ~3

        items.append({
            "id": id_item,
            "class": cls_item,
            "title": title_item,
            "x": x_item,
            "y": y_item,
            "cx": cx_item,
            "cy": cy_item,
            "style": style_item,
            "exStyle": exStyle_item
        })

    return {
        "title": title,
        "style": style,
        "exStyle": exStyle,
        "x": x,
        "y": y,
        "cx": cx,
        "cy": cy,
        "fontname": fontname,
        "pointsize": pointsize,
        "items": items
    }

def main():
    pe = pefile.PE("nonbuilddata/Planet.exe")
    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print("No resource directory found in Planet.exe.")
        return

    # Find RT_DIALOG (ID = 5)
    dialog_type_entry = None
    for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if entry.id == 5:
            dialog_type_entry = entry
            break

    if not dialog_type_entry:
        print("No dialog resources found in Planet.exe.")
        return

    print("Found Dialog Resources:")
    for dlg_entry in dialog_type_entry.directory.entries:
        dlg_id = dlg_entry.id or dlg_entry.name
        # Get language directory
        lang_dir = dlg_entry.directory
        for lang_entry in lang_dir.entries:
            # Get resource data
            rva = lang_entry.data.struct.OffsetToData
            size = lang_entry.data.struct.Size
            data = pe.get_data(rva, size)

            try:
                dlg = parse_dialog(data)
                if dlg:
                    print(f"\n==================================================")
                    print(f"DIALOG ID: {dlg_id} (Lang: {lang_entry.id})")
                    print(f"Title: {dlg['title']}")
                    print(f"Rect: x={dlg['x']}, y={dlg['y']}, cx={dlg['cx']}, cy={dlg['cy']}")
                    print(f"Font: {dlg['fontname']} ({dlg['pointsize']}pt)")
                    print(f"Styles: style={hex(dlg['style'])}, exStyle={hex(dlg['exStyle'])}")
                    print(f"Items (Count: {len(dlg['items'])}):")
                    for item in dlg["items"]:
                        print(f"  ID {item['id']:3d}: [{item['class']}] '{item['title']}' rect=({item['x']},{item['y']},{item['cx']},{item['cy']}) style={hex(item['style'])}")
            except Exception as e:
                print(f"Error parsing dialog {dlg_id}: {e}")

if __name__ == "__main__":
    main()
