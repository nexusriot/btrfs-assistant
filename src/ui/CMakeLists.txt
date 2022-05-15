set(UI_SRC
    ui/MainWindow.ui ui/MainWindow.h ui/MainWindow.cpp
    ui/Cli.h ui/Cli.cpp
    ui/DiffViewer.ui ui/DiffViewer.h ui/DiffViewer.cpp
    ui/FileBrowser.ui ui/FileBrowser.h ui/FileBrowser.cpp
    ui/SnapshotSubvolumeDialog.ui ui/SnapshotSubvolumeDialog.h ui/SnapshotSubvolumeDialog.cpp
)

file(GLOB_RECURSE UI_FILES "*.ui")
source_group("Ui Files" FILES ${UI_FILES})
