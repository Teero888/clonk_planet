import os
from launcher.c4_io import C4Group

def find_mids_recursive(grp_data, grp_name, path_acc, all_mids):
    try:
        grp = C4Group(raw_data=grp_data)
        for e in grp.entries:
            current_path = f"{path_acc}/{e['name']}"
            if e['name'].lower().endswith('.mid'):
                print(f"FOUND: {current_path}")
                all_mids.append(current_path)
            if e['is_group']:
                sub_data = grp.get_file(e['name'])
                if sub_data:
                    find_mids_recursive(sub_data, e['name'], current_path, all_mids)
    except Exception as e:
        pass

if __name__ == "__main__":
    all_mids = []
    for f in os.listdir('planet_data'):
        if f.lower().endswith(('.c4f', '.c4s', '.c4p', '.c4d', '.c4g')):
            path = os.path.join('planet_data', f)
            try:
                with open(path, 'rb') as file:
                    data = file.read()
                find_mids_recursive(data, f, f, all_mids)
            except:
                pass
    print(f"\nTotal MIDIs found: {len(all_mids)}")
