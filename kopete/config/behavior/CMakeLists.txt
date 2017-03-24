
include_directories( ${KOPETE_INCLUDES} )


########### next target ###############

set(kcm_kopete_behaviorconfig_PART_SRCS
   behaviorconfig.cpp 
   behaviorconfig_chat.cpp 
   behaviorconfig_events.cpp 
   behaviorconfig_general.cpp 
   behaviorconfig_away.cpp 
   )

ki18n_wrap_ui(kcm_kopete_behaviorconfig_PART_SRCS 
   behaviorconfig_chat.ui 
   behaviorconfig_events.ui 
   behaviorconfig_general.ui 
   behaviorconfig_away.ui
   )


add_library(kcm_kopete_behaviorconfig MODULE ${kcm_kopete_behaviorconfig_PART_SRCS})


target_link_libraries(kcm_kopete_behaviorconfig   kopete)

install(TARGETS kcm_kopete_behaviorconfig  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_behaviorconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})


