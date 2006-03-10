kde4_header()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libgadu-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libgadu-config.h )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#noinst_LTLIBRARIES = libgadu_copy.la
#INCLUDES = $(SSL_INCLUDES) 
#libgadu_copy_la_LDFLAGS = $(SSL_LDFLAGS)
#libgadu_copy_la_LIBADD =  $(LIBSSL) $(LIBPTHREAD) 
#libgadu_copy_la_SOURCES = common.c \
#			  dcc.c \
#			  events.c \
#			  http.c \
#			  libgadu.c \
#			  pubdir50.c \
#			  pubdir.c 
