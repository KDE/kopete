
########### next target ###############

if ( BUILD_libjingle_cricketexampleslogin )

SET(cricketexampleslogin_STAT_SRCS
   xmppsocket.cc
   xmppauth.cc
   xmppthread.cc
   xmpppump.cc
   jingleinfotask.cc
   presenceouttask.cc
   presencepushtask.cc
)

add_library(cricketexampleslogin STATIC ${cricketexampleslogin_STAT_SRCS})

endif ( BUILD_libjingle_cricketexampleslogin )


########### next target ###############

if ( BUILD_libjingle_login )

SET(login_SRCS
   login_main.cc
   xmppsocket.cc
   xmppthread.cc
   xmpppump.cc
   xmppauth.cc
)

add_executable(login ${login_SRCS})

set_target_properties(login PROPERTIES OUTPUT_NAME googletalk-login)

target_link_libraries(login cricketxmpp cricketxmllite cricketbase ${EXPAT_LIBRARY} pthread ${SSL_LIBRARY} ${CRYPTO_LIBRARY})

endif ( BUILD_libjingle_login )


########### install files ###############

if ( BUILD_libjingle_login )
install(TARGETS login DESTINATION bin)
endif ( BUILD_libjingle_login )

