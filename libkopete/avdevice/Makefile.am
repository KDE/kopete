INCLUDES = 
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
        $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
        -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libkopete_videodevice.la
libkopete_videodevice_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = videodevice.h videoinput.h videodevicelistitem.h
libkopete_videodevice_la_SOURCES = videodevice.cpp videoinput.cpp \
	videodevicelistitem.cpp
