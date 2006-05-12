
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libgadu-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libgadu-config.h )



set(libgadu_SRCS common.c dcc.c events.c http.c libgadu.c pubdir50.c pubdir.c)

kde4_add_library(gadu_kopete SHARED ${libgadu_SRCS})

target_link_libraries(gadu_kopete pthread)

install_targets( ${LIB_INSTALL_DIR} gadu_kopete )

########### next target ###############


########### install files ###############





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
