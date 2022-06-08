include(ui/CMakeLists.txt)
include(model/CMakeLists.txt)
include(util/CMakeLists.txt)
include(widgets/CMakeLists.txt)

set_property(SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS "VERSION=\"1.5\"")

set(btrfsassistant_SRCS
        main.cpp
        ${UI_SRC}
	${MODEL_SRC}
        ${UTIL_SRC}
        ${WIDGETS_SRC}
        ${PROJECT_SOURCE_DIR}/icons/icons.qrc
)

file(GLOB btrfsassistant_UIFILES ui/*.ui)
file(GLOB TS_FILES ${PROJECT_SOURCE_DIR}/translations/*.ts)

qt5_create_translation(FILES_TS ${btrfsassistant_UIFILES} ${TS_FILES})

add_executable(btrfs-assistant-bin
    ${btrfsassistant_SRCS} ${FILES_TS}
)

install(FILES ${FILES_TS} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/btrfs-assistant/translations)
install(FILES btrfs-assistant.conf DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})
install(FILES btrfs-assistant.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES btrfs-assistant.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
install(FILES org.btrfs-assistant.pkexec.policy DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions/)
install(PROGRAMS btrfs-assistant DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS btrfs-assistant-launcher DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS btrfs-assistant-bin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

find_library(BTRFSUTIL_LIB btrfsutil)
target_link_libraries(btrfs-assistant-bin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${BTRFSUTIL_LIB})
target_compile_options(btrfs-assistant-bin PRIVATE -Werror -Wall -Wextra -Wconversion)
