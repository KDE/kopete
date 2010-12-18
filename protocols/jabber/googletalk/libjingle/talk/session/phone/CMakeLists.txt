
if ( BUILD_libjingle_cricketsessionphone )

include_directories(
   ${MEDIASTREAMER_INCLUDE_DIR}
   ${LIBORTP_INCLUDE_DIR}
)

if ( MSILBC_FOUND )
	add_definitions ( -DMSILBC_LIBRAY="${MSILBC_LIBRARY}" )
endif ( MSILBC_FOUND )

add_definitions(-D_REENTRANT -DORTP_INET6)

endif ( BUILD_libjingle_cricketsessionphone )

########### next target ###############

if ( BUILD_libjingle_cricketsessionphone )

SET(cricketsessionphone_STAT_SRCS
   linphonemediaengine.cc
   audiomonitor.cc
   channelmanager.cc
   voicechannel.cc
   call.cc
   phonesessionclient.cc
)

add_library(cricketsessionphone STATIC ${cricketsessionphone_STAT_SRCS})

endif ( BUILD_libjingle_cricketsessionphone )


########### install files ###############

