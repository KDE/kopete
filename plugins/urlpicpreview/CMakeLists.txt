
include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS KCMUtils)

set(kopete_urlpicpreview_PART_SRCS
#   linkpreview.cpp
   urlpicpreviewplugin.cpp )

kconfig_add_kcfg_files(kopete_urlpicpreview_PART_SRCS urlpicpreviewconfig.kcfgc )

add_library(kopete_urlpicpreview MODULE ${kopete_urlpicpreview_PART_SRCS})


target_link_libraries(kopete_urlpicpreview
    KF5::KDELibs4Support
    KF5::KIOCore
    KF5::KCMUtils
    kopete
)

install(TARGETS kopete_urlpicpreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_urlpicpreview_PART_SRCS
   urlpicpreviewpreferences.cpp )

ki18n_wrap_ui(kcm_kopete_urlpicpreview_PART_SRCS urlpicpreviewprefsbase.ui )

kconfig_add_kcfg_files(kcm_kopete_urlpicpreview_PART_SRCS urlpicpreviewconfig.kcfgc )

add_library(kcm_kopete_urlpicpreview MODULE ${kcm_kopete_urlpicpreview_PART_SRCS})


target_link_libraries(kcm_kopete_urlpicpreview
    KF5::KIOCore
    kopete
)

install(TARGETS kcm_kopete_urlpicpreview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES urlpicpreview.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_urlpicpreview.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_urlpicpreview_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
ecm_install_icons( ${DATA_INSTALL_DIR}/kopete/icons  )

