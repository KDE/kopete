add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_autoreplace_PART_SRCS autoreplaceplugin.cpp autoreplaceconfig.cpp )

kde4_add_plugin(kopete_autoreplace ${kopete_autoreplace_PART_SRCS})


target_link_libraries(kopete_autoreplace  ${KDE4_KDEUI_LIBS} kopete )

install(TARGETS kopete_autoreplace  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_autoreplace_PART_SRCS autoreplacepreferences.cpp autoreplaceconfig.cpp )

kde4_add_ui_files(kcm_kopete_autoreplace_PART_SRCS autoreplaceprefs.ui )


kde4_add_plugin(kcm_kopete_autoreplace ${kcm_kopete_autoreplace_PART_SRCS})


target_link_libraries(kcm_kopete_autoreplace  ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kopete )

install(TARGETS kcm_kopete_autoreplace  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_autoreplace.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_autoreplace_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




