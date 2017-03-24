include_directories( 
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kcm_kopete_pluginconfig_PART_SRCS kopetepluginconfig.cpp )

add_library(kcm_kopete_pluginconfig MODULE ${kcm_kopete_pluginconfig_PART_SRCS})


target_link_libraries(kcm_kopete_pluginconfig   kopete)

install(TARGETS kcm_kopete_pluginconfig DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_pluginconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
