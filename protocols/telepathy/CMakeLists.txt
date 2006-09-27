
add_subdirectory( icons )

include_directories( 
${KOPETE_INCLUDES} ${DECIBEL_INCLUDES}
)

add_definitions( ${DECIBEL_DEFINITIONS} )

########### next target ###############

set(kopete_telepathy_PART_SRCS telepathyprotocol.cpp telepathyaccount.cpp telepathycontact.cpp )

kde4_automoc(${kopete_telepathy_PART_SRCS})

kde4_add_plugin(kopete_telepathy ${kopete_telepathy_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_telepathy )

target_link_libraries(kopete_telepathy  ${KDE4_KIO_LIBS} kopete ${DECIBEl_LIBRARIES} )

install(TARGETS kopete_telepathy  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_telepathy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
