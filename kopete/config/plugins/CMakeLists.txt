include_directories( 
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kcm_kopete_pluginconfig_PART_SRCS kopetepluginconfig.cpp )

kde4_add_plugin(kcm_kopete_pluginconfig ${kcm_kopete_pluginconfig_PART_SRCS})


target_link_libraries(kcm_kopete_pluginconfig  ${KDE4_KUTILS_LIBS} kopete)

install(TARGETS kcm_kopete_pluginconfig DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_pluginconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
