
add_definitions(-std=gnu99 -fno-strict-aliasing -DG_LOG_DOMAIN=\\"MediaStreamer\\")

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${LIBORTP_INCLUDE_DIR}
   ${GLIB2_INCLUDE_DIR}
)

if ( ILBC_FOUND )
        include_directories ( ${ILBC_INCLUDE_DIR} )
endif ( ILBC_FOUND )

if ( SPEEX_FOUND )
        include_directories ( ${SPEEX_INCLUDE_DIR} ${SPEEX_INCLUDE_DIR}/speex )
endif ( SPEEX_FOUND )

########### next target ###############

if ( BUILD_libjingle_mediastreamer )

SET(mediastreamer_STAT_SRCS
   msfilter.c
   mscodec.c
   mssoundread.c
   mssoundwrite.c
   msbuffer.c
   msqueue.c
   msfifo.c
   ms.c
   mssync.c
   msnosync.c
   msread.c
   mswrite.c
   mscopy.c
   msosswrite.c
   msossread.c
   msringplayer.c
   msrtprecv.c
   msrtpsend.c
   msAlawenc.c
   msAlawdec.c
   msMUlawenc.c
   msMUlawdec.c
   mstimer.c
   msqdispatcher.c
   msfdispatcher.c
   sndcard.c
   osscard.c
   hpuxsndcard.c
   alsacard.c
   jackcard.c
   audiostream.c
   msspeexenc.c
   msspeexdec.c
   msilbcdec.c
   msilbcenc.c
)

add_library(libjingle_mediastreamer STATIC ${mediastreamer_STAT_SRCS})

endif ( BUILD_libjingle_mediastreamer )


########### install files ###############

