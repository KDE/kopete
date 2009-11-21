
########### next target ###############

if ( BUILD_libjingle_cricketxmpp )

SET(cricketxmpp_STAT_SRCS
   constants.cc
   jid.cc
   saslmechanism.cc
   xmppclient.cc
   xmppengineimpl.cc
   xmppengineimpl_iq.cc
   xmpplogintask.cc
   xmppstanzaparser.cc
   xmpptask.cc
   ratelimitmanager.cc
)

add_library(cricketxmpp STATIC ${cricketxmpp_STAT_SRCS})

endif ( BUILD_libjingle_cricketxmpp )


########### install files ###############

