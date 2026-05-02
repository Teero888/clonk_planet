import sys
import os
from PyQt5.QtWidgets import QApplication
from launcher import ClonkLauncher

app = QApplication(sys.argv)
try:
    launcher = ClonkLauncher()
    print("Launcher initialized.")
    root_item = launcher.tree_model.invisibleRootItem()
    print(f"Root item has {root_item.rowCount()} children.")
    for i in range(root_item.rowCount()):
        item = root_item.child(i)
        print(f"  Root {i}: {item.text()}")
        for j in range(item.rowCount()):
            print(f"    Child {j}: {item.child(j).text()}")
except Exception as e:
    import traceback
    traceback.print_exc()
sys.exit(0)
