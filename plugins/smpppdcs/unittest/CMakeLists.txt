
include_directories( ${CMAKE_SOURCE_DIR}/src  )


########### next target ###############

set(smpppdcstests_SRCS main.cpp clienttest.cpp )

kde4_automoc(smpppdcstests ${smpppdcstests_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(smpppdcstests ${smpppdcstests_SRCS})

target_link_libraries(smpppdcstests  ${KDE4_KDECORE_LIBS} smpppdclient kunittestgui )

endif(KDE4_BUILD_TESTS)

########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src $(all_includes) -I../libsmpppdclient
#METASOURCES = AUTO
#
#
#check_PROGRAMS = smpppdcstests
#
#smpppdcstests_SOURCES = main.cpp clienttest.cpp
#smpppdcstests_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#smpppdcstests_LDADD = ../libsmpppdclient/libsmpppdclient.la -lkunittestgui
#
#noinst_HEADERS = clienttest.h
#
#check:
#	kunittest ./smpppdcstests ClientTest
#
