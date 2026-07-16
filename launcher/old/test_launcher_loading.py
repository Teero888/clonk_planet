import sys
import os
from PyQt5.QtWidgets import QApplication
from launcher import ClonkLauncher

app = QApplication(sys.argv)
try:
    launcher = ClonkLauncher()
    print("Launcher initialized.")
    # The refresh_resources is called in __init__
    root_item = launcher.tree_model.invisibleRootItem()
    print(f"Root item has {root_item.rowCount()} children.")
    for i in range(root_item.rowCount()):
        print(f"  Child {i}: {root_item.child(i).text()}")
except Exception as e:
    import traceback
    traceback.print_exc()
sys.exit(0)
