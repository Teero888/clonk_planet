import os
from launcher.c4_io import C4Group

def find_mids(path, indent=0):
    try:
        grp = C4Group(path)
        for e in grp.entries:
            if e['name'].lower().endswith('.mid'):
                print("  " * indent + f"FOUND: {e['name']} in {path}")
            if e['is_group']:
                sub_data = grp.get_file(e['name'])
                if sub_data:
                    # Temporary save or process in memory
                    s_grp = C4Group(raw_data=sub_data)
                    for se in s_grp.entries:
                        if se['name'].lower().endswith('.mid'):
                            print("  " * (indent+1) + f"FOUND: {se['name']} in {path}/{e['name']}")
    except Exception as e:
        pass

if __name__ == "__main__":
    for f in os.listdir('planet_data'):
        if f.lower().endswith(('.c4f', '.c4s', '.c4p', '.c4d', '.c4g')):
            find_mids(os.path.join('planet_data', f))
