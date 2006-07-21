
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_videodevice_LIB_SRCS 
   videodevice.cpp 
   videoinput.cpp 
   videodevicepool.cpp 
   videodevicemodelpool.cpp )

kde4_automoc(${kopete_videodevice_LIB_SRCS})

kde4_add_library(kopete_videodevice SHARED ${kopete_videodevice_LIB_SRCS})

target_link_libraries(kopete_videodevice  ${KDE4_KDECORE_LIBS} )

set_target_properties(kopete_videodevice PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopete_videodevice  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES =$(GLINC) $(all_includes)
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#        $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
#        -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libkopete_videodevice.la
#noinst_LTLIBRARIES = libkvideoio.la
#libkopete_videodevice_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
#noinst_HEADERS = videodevice.h videoinput.h kxv.h qvideo.h qvideostream.h \
#		videodevicepool.h videodevicemodelpool.h
#libkopete_videodevice_la_SOURCES = videodevice.cpp videoinput.cpp \
#		videodevicepool.cpp videodevicemodelpool.cpp
#libkvideoio_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0:0
#libkvideoio_la_SOURCES = kxv.cpp qvideo.cpp qvideostream.cpp
#libkvideoio_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(GLLIB)
