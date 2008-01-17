INCLUDES =$(GLINC) $(all_includes)
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
        $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
        -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libkopete_videodevice.la
noinst_LTLIBRARIES = libkvideoio.la
libkopete_videodevice_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = kxv.h qvideo.h qvideostream.h videocontrol.h videodevice.h \
	videodevicemodelpool.h videodevicepool.h videoinput.h \
	sonix_compress.h bayer.h
libkopete_videodevice_la_SOURCES = videocontrol.cpp videodevice.cpp \
	videodevicemodelpool.cpp videodevicepool.cpp videoinput.cpp \
	sonix_compress.cpp bayer.cpp
libkvideoio_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0:0
libkvideoio_la_SOURCES = kxv.cpp qvideo.cpp qvideostream.cpp
libkvideoio_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(GLLIB)
