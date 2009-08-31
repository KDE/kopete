include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory (KopeteTelepathy)

project (kopete-protocol-telepathy)

add_subdirectory (icons)

remove_definitions (-DQT_NO_STL)

add_definitions (-DKDE_DEFAULT_DEBUG_AREA=14400)

include_directories (${KDE4_INCLUDES}
                     ${KOPETE_INCLUDES}
                     ${KCMTELEPATHYACCOUNTS_INCLUDE_DIR}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

set (kopete_protocol_telepathy_SRCS
     telepathyprotocol.cpp
     ui/telepathyaddcontactpage.cpp
     ui/telepathyeditaccountwidget.cpp
)

kde4_add_ui_files (kopete_protocol_telepathy_UI_SRCS
                   ui/telepathyaddcontactpage.ui
)

kde4_add_plugin (kopete_telepathy
                 ${kopete_protocol_telepathy_SRCS}
                 ${kopete_protocol_telepathy_UI_SRCS}
)

target_link_libraries (kopete_telepathy
                       kopetetelepathy
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDEUI_LIBRARY}
                       ${KOPETE_LIBRARIES}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KCMTELEPATHYACCOUNTS_LIBRARIES}
)

install (TARGETS kopete_telepathy
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install (FILES kopete_telepathy.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

