include_directories( ${KOPETE_INCLUDES} )

########### next target ###############

set(kcm_kopete_statusconfig_PART_SRCS
    statusconfig.cpp
    statusconfig_manager.cpp
    statusconfig_general.cpp
    statusmodel.cpp
    )

kde4_add_ui_files(kcm_kopete_statusconfig_PART_SRCS
    statusconfig_manager.ui
    statusconfig_general.ui
)

kde4_add_plugin(kcm_kopete_statusconfig ${kcm_kopete_statusconfig_PART_SRCS})

target_link_libraries(kcm_kopete_statusconfig  ${KDE4_KUTILS_LIBS} ${QT_QTXML_LIBRARY} kopete)

install(TARGETS kcm_kopete_statusconfig  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_statusconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
