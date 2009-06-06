
include_directories(
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kopete_addbookmarks_PART_SRCS addbookmarksplugin.cpp addbookmarksprefssettings.cpp )


kde4_add_plugin(kopete_addbookmarks ${kopete_addbookmarks_PART_SRCS})


target_link_libraries(kopete_addbookmarks  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_addbookmarks  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_addbookmarks_PART_SRCS addbookmarkspreferences.cpp addbookmarksprefssettings.cpp )

kde4_add_ui_files(kcm_kopete_addbookmarks_PART_SRCS addbookmarksprefsui.ui )


kde4_add_plugin(kcm_kopete_addbookmarks ${kcm_kopete_addbookmarks_PART_SRCS})


target_link_libraries(kcm_kopete_addbookmarks kopete ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_addbookmarks  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_addbookmarks.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_addbookmarks_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




