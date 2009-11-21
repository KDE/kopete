
########### next target ###############

if ( BUILD_libjingle_cricketsessionfileshare )

SET(cricketsessionfileshare_STAT_SRCS
   fileshare.cc
)

add_library(cricketsessionfileshare STATIC ${cricketsessionfileshare_STAT_SRCS})

endif ( BUILD_libjingle_cricketsessionfileshare )


########### install files ###############

