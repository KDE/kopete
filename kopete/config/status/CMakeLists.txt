include_directories( ${KOPETE_INCLUDES} )

########### next target ###############

set(kcm_kopete_statusconfig_PART_SRCS
    statusconfig.cpp
    statusconfig_manager.cpp
    statusconfig_general.cpp
    statusmodel.cpp
    )

ki18n_wrap_ui(kcm_kopete_statusconfig_PART_SRCS
    statusconfig_manager.ui
    statusconfig_general.ui
)

add_library(kcm_kopete_statusconfig MODULE ${kcm_kopete_statusconfig_PART_SRCS})

target_link_libraries(kcm_kopete_statusconfig   Qt5::Xml kopete)

install(TARGETS kcm_kopete_statusconfig  DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install( FILES kopete_statusconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
