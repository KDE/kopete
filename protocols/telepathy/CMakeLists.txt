project (kopete-telepathy)

add_subdirectory (icons)

add_definitions (-DKDE_DEFAULT_DEBUG_AREA=14400)
remove_definitions (-DQT_NO_STL)

include_directories (${KDE4_INCLUDES}
                     ${KOPETE_INCLUDES}
                     ${KCMTELEPATHYACCOUNTS_INCLUDE_DIR}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

kde4_add_ui_files (kopete_telepathy_UI_SRCS
                   ui/telepathyaddcontactpage.ui
)

set (kopete_telepathy_SRCS
     telepathyprotocol.cpp
     telepathyaccount.cpp
     telepathyaddedinfoevent.cpp
     telepathychannelmanager.cpp
     telepathychatsession.cpp
     telepathyclienthandler.cpp
     telepathycontact.cpp
     telepathycontactmanager.cpp
     telepathyprotocolinternal.cpp
     telepathyfiletransfer.cpp
     ui/telepathyaddcontactpage.cpp
     ui/telepathyeditaccountwidget.cpp
)

kde4_add_plugin (kopete_telepathy
                 ${kopete_telepathy_SRCS}
                 ${kopete_telepathy_UI_SRCS}
)

target_link_libraries (kopete_telepathy
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDEUI_LIBRARY}
                       ${KDE4_KIO_LIBRARY}
                       ${KOPETE_LIBRARIES}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KCMTELEPATHYACCOUNTS_LIBRARIES}
)

install (TARGETS kopete_telepathy DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES kopete_telepathy.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES telepathychatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_telepathy)
