
########### next target ###############

if ( BUILD_libjingle_pcp )

SET(pcp_SRCS
   pcp_main.cc
)

add_executable(pcp ${pcp_SRCS})

set_target_properties(pcp PROPERTIES OUTPUT_NAME googletalk-pcp)

target_link_libraries(pcp cricketexampleslogin cricketsessionfileshare cricketsessiontunnel cricketp2pclient cricketp2pbase cricketxmpp cricketxmllite cricketbase ${EXPAT_LIBRARY} pthread ${SSL_LIBRARY} ${CRYPTO_LIBRARY})

endif ( BUILD_libjingle_pcp )


########### install files ###############

if ( BUILD_libjingle_pcp )
install(TARGETS pcp DESTINATION bin)
endif ( BUILD_libjingle_pcp )

