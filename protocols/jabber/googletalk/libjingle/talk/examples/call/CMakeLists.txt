
########### next target ###############

if ( BUILD_libjingle_call )

SET(call_SRCS
   call_main.cc
   callclient.cc
   console.cc
)

add_executable(call ${call_SRCS})

set_target_properties(call PROPERTIES OUTPUT_NAME googletalk-call)

if ( ILBC_FOUND )
	set ( call_link_libraries ${call_link_libraries} ${ILBC_LIBRARY} )
endif ( ILBC_FOUND )

if ( SPEEX_FOUND )
	set ( call_link_libraries ${call_link_libraries} ${SPEEX_LIBRARY} )
endif ( SPEEX_FOUND )

if ( ALSA_FOUND )
	set ( call_link_libraries ${call_link_libraries} ${ASOUND_LIBRARY} )
endif ( ALSA_FOUND )

target_link_libraries(call cricketexampleslogin cricketsessionphone cricketp2pclient cricketp2pbase cricketxmpp cricketxmllite cricketbase ${EXPAT_LIBRARY} ${LIBORTP_LIBRARY} pthread ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${GLIB2_LIBRARIES} libjingle_mediastreamer ${call_link_libraries})

endif ( BUILD_libjingle_call )


########### install files ###############

if ( BUILD_libjingle_call )
install(TARGETS call DESTINATION bin)
endif ( BUILD_libjingle_call )

