set(UI_SRC
    ui/btrfs-assistant.ui ui/BtrfsAssistant.h ui/BtrfsAssistant.cpp
    ui/Cli.h ui/Cli.cpp
    ui/DiffViewer.ui ui/DiffViewer.h ui/DiffViewer.cpp
    ui/FileBrowser.ui ui/FileBrowser.h ui/FileBrowser.cpp
)

file(GLOB_RECURSE UI_FILES "*.ui")
source_group("Ui Files" FILES ${UI_FILES})
