
########### next target ###############

if ( BUILD_libjingle_relayserver )

SET(relayserver_SRCS
   relayserver.cc
   relayserver_main.cc
)

add_executable(relayserver ${relayserver_SRCS})

set_target_properties(relayserver PROPERTIES OUTPUT_NAME googletalk-relayserver)

target_link_libraries(relayserver cricketp2pbase cricketbase pthread)

endif ( BUILD_libjingle_relayserver )


########### next target ###############

if ( BUILD_libjingle_stunserver )

SET(stunserver_SRCS
   stunserver.cc
   stunserver_main.cc
)

add_executable(stunserver ${stunserver_SRCS})

set_target_properties(stunserver PROPERTIES OUTPUT_NAME googletalk-stunserver)

target_link_libraries(stunserver cricketp2pbase cricketbase pthread)

endif ( BUILD_libjingle_stunserver )


########### next target ###############

if ( BUILD_libjingle_stunserver_unittest )

SET(stunserver_unittest_SRCS
   stunserver_unittest.cc
   stunserver.cc
)

add_executable(stunserver_unittest ${stunserver_unittest_SRCS})

target_link_libraries(stunserver_unittest cricketp2pbase cricketbase pthread crickettest)

endif ( BUILD_libjingle_stunserver_unittest )


########### next target ###############

if ( BUILD_libjingle_session_unittest )

SET(session_unittest_SRCS
   session_unittest.cc
   stunserver.cc
   relayserver.cc
)

add_executable(session_unittest ${session_unittest_SRCS})

target_link_libraries(session_unittest cricketp2pbase cricketbase pthread crickettest cricketxmllite cricketxmpp ${EXPAT_LIBRARY})

endif ( BUILD_libjingle_session_unittest )


########### next target ###############

if ( BUILD_libjingle_port_unittest )

SET(port_unittest_SRCS
   port_unittest.cc
   stunserver.cc
   relayserver.cc
)

add_executable(port_unittest ${port_unittest_SRCS})

target_link_libraries(port_unittest cricketp2pbase cricketbase pthread crickettest)

endif ( BUILD_libjingle_port_unittest )


########### next target ###############

if ( BUILD_libjingle_cricketp2pbase )

SET(cricketp2pbase_STAT_SRCS
   stun.cc
   port.cc
   udpport.cc
   tcpport.cc
   stunport.cc
   relayport.cc
   stunrequest.cc
   sessionmanager.cc
   session.cc
   transport.cc
   transportchannel.cc
   transportchannelproxy.cc
   p2ptransport.cc
   p2ptransportchannel.cc
   rawtransport.cc
   rawtransportchannel.cc
   constants.cc
   pseudotcp.cc
)

add_library(cricketp2pbase STATIC ${cricketp2pbase_STAT_SRCS})

endif ( BUILD_libjingle_cricketp2pbase )


########### install files ###############

if ( BUILD_libjingle_relayserver )
install(TARGETS relayserver DESTINATION bin)
endif ( BUILD_libjingle_relayserver )

if ( BUILD_libjingle_stunserver )
install(TARGETS stunserver DESTINATION bin)
endif ( BUILD_libjingle_stunserver )

