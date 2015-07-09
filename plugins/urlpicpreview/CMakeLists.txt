
include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_urlpicpreview_PART_SRCS
#   linkpreview.cpp
   urlpicpreviewplugin.cpp )

kde4_add_kcfg_files(kopete_urlpicpreview_PART_SRCS urlpicpreviewconfig.kcfgc )

kde4_add_plugin(kopete_urlpicpreview ${kopete_urlpicpreview_PART_SRCS})


target_link_libraries(kopete_urlpicpreview  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_urlpicpreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_urlpicpreview_PART_SRCS
   urlpicpreviewpreferences.cpp )

kde4_add_ui_files(kcm_kopete_urlpicpreview_PART_SRCS urlpicpreviewprefsbase.ui )

kde4_add_kcfg_files(kcm_kopete_urlpicpreview_PART_SRCS urlpicpreviewconfig.kcfgc )

kde4_add_plugin(kcm_kopete_urlpicpreview ${kcm_kopete_urlpicpreview_PART_SRCS})


target_link_libraries(kcm_kopete_urlpicpreview ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_urlpicpreview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES urlpicpreview.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_urlpicpreview.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_urlpicpreview_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
kde4_install_icons( ${DATA_INSTALL_DIR}/kopete/icons  )

