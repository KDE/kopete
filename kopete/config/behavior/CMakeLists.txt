
include_directories( ${KOPETE_INCLUDES} )


########### next target ###############

set(kcm_kopete_behaviorconfig_PART_SRCS
   behaviorconfig.cpp 
   behaviorconfig_chat.cpp 
   behaviorconfig_events.cpp 
   behaviorconfig_general.cpp 
   behaviorconfig_away.cpp 
   )

kde4_add_ui_files(kcm_kopete_behaviorconfig_PART_SRCS 
   behaviorconfig_chat.ui 
   behaviorconfig_events.ui 
   behaviorconfig_general.ui 
   behaviorconfig_away.ui
   )


kde4_add_plugin(kcm_kopete_behaviorconfig ${kcm_kopete_behaviorconfig_PART_SRCS})


target_link_libraries(kcm_kopete_behaviorconfig  ${KDE4_KUTILS_LIBS} kopete)

install(TARGETS kcm_kopete_behaviorconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_behaviorconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


