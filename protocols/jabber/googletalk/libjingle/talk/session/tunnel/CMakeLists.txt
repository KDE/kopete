
########### next target ###############

if ( BUILD_libjingle_cricketsessiontunnel )

SET(cricketsessiontunnel_STAT_SRCS
   tunnelsessionclient.cc
   pseudotcpchannel.cc
)

add_library(cricketsessiontunnel STATIC ${cricketsessiontunnel_STAT_SRCS})

endif ( BUILD_libjingle_cricketsessiontunnel )


########### install files ###############

