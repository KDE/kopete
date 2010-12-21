
########### next target ###############

if ( BUILD_libjingle_call )

if ( NOT DEFINED SOUND_INSTALL_DIR )
	set ( SOUND_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/sounds )
endif ( NOT DEFINED SOUND_INSTALL_DIR )

add_definitions ( -DRINGWAV="\\"${SOUND_INSTALL_DIR}/KDE-Im-Phone-Ring.wav\\"" -DCALLWAV="\\"${SOUND_INSTALL_DIR}/KDE-Im-Phone-Ring.wav\\"" )

SET(call_SRCS
   call_main.cc
   callclient.cc
   console.cc
)

add_executable(call ${call_SRCS})

set_target_properties(call PROPERTIES OUTPUT_NAME googletalk-call)

target_link_libraries(call cricketexampleslogin cricketsessionphone cricketp2pclient cricketp2pbase cricketxmpp cricketxmllite cricketbase ${EXPAT_LIBRARY} ${LIBORTP_LIBRARY} pthread ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${MEDIASTREAMER_LIBRARIES} ${call_link_libraries})

endif ( BUILD_libjingle_call )


########### install files ###############

if ( BUILD_libjingle_call )
install(TARGETS call DESTINATION bin)
endif ( BUILD_libjingle_call )

