
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_connectionstatus_PART_SRCS connectionstatusplugin.cpp )

kde4_automoc(${kopete_connectionstatus_PART_SRCS})

kde4_add_plugin(kopete_connectionstatus ${kopete_connectionstatus_PART_SRCS})


target_link_libraries(kopete_connectionstatus  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_connectionstatus  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_connectionstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




