
if (LIBV4L2_FOUND)
include_directories( 
${KOPETE_INCLUDES} 
${LIBV4L2_INCLUDE_DIR}
)
add_definitions(-DHAVE_LIBV4L2)
else (LIBV4L2_FOUND)
include_directories(
${KOPETE_INCLUDES}
)
endif (LIBV4L2_FOUND)

include(CheckIncludeFile)
check_include_file(linux/videodev.h HAVE_V4L)
if(HAVE_V4L)
add_definitions(-DHAVE_V4L)
endif(HAVE_V4L)

########### next target ###############

set(kopete_videodevice_LIB_SRCS 
   videodevice.cpp 
   videoinput.cpp 
   videodevicepool.cpp 
   bayer.cpp
   sonix_compress.cpp )

add_library(kopete_videodevice SHARED ${kopete_videodevice_LIB_SRCS})

if (LIBV4L2_FOUND)
target_link_libraries(kopete_videodevice Qt5::Gui KF5::KDELibs4Support  ${LIBV4L2_LIBRARY})
else (LIBV4L2_FOUND)
target_link_libraries(kopete_videodevice Qt5::Gui KF5::KDELibs4Support )
endif (LIBV4L2_FOUND)

set_target_properties(kopete_videodevice PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kopete_videodevice ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############


########### install files ###############





