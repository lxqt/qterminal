# List of files to be renamed
set(FILES_TO_RENAME
    Qterminal.desktop
    Qterminal_drop.desktop
)

set(DNS "org.lxqt_project.")

foreach(FILE ${FILES_TO_RENAME})
    set(NEW_FILE_NAME "${DNS}${FILE}")
    file(INSTALL ${FILE} RENAME ${NEW_FILE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
endforeach()
