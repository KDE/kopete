
########### next target ###############

if ( BUILD_libjingle_cricketp2pclient )

SET(cricketp2pclient_STAT_SRCS
   basicportallocator.cc
   httpportallocator.cc
   socketmonitor.cc
)

add_library(cricketp2pclient STATIC ${cricketp2pclient_STAT_SRCS})

endif ( BUILD_libjingle_cricketp2pclient )


########### install files ###############

