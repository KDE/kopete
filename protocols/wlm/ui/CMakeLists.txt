INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/$(srcdir)/..
	)

SET(kopetewlmui_STAT_SRCS
   wlmwebcamdialog.cpp
)

ADD_LIBRARY(kopetewlmui STATIC ${kopetewlmui_STAT_SRCS})




#original Makefile.am contents follow:

#INCLUDES = 
#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/.. $(all_includes)
#libkopetewlmui_la_LDFLAGS = $(all_libraries)
#noinst_LTLIBRARIES = libkopetewlmui.la
#noinst_HEADERS = wlmwebcamdialog.h
#libkopetewlmui_la_SOURCES = wlmwebcamdialog.cpp
