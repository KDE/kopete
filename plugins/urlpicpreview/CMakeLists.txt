
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

install(TARGETS kopete_urlpicpreview DESTINATION ${KDE_INSTALL_PLUGINDIR})


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

install(TARGETS kcm_kopete_urlpicpreview  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES urlpicpreview.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES kopete_urlpicpreview.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopete_urlpicpreview_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)
file(GLOB icons_png "*.png")
file(GLOB icons_svg "*.svg")
set(all_icons ${icons_png} ${icons_svg})
ecm_install_icons(ICONS ${all_icons}  DESTINATION ${KDE_INSTALL_DATADIR}/kopete/icons THEME oxygen)

