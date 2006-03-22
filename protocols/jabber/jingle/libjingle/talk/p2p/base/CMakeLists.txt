kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(relayserver_SRCS relayserver.cc relayserver_main.cc )

kde4_automoc(${relayserver_SRCS})

kde4_add_executable(relayserver ${relayserver_SRCS})

target_link_libraries(relayserver  ${KDE4_KDECORE_LIBS} cricketbase cricketp2pbase pthread )

install_targets(/bin relayserver )


########### next target ###############

set(stunserver_SRCS stunserver.cc stunserver_main.cc )

kde4_automoc(${stunserver_SRCS})

kde4_add_executable(stunserver ${stunserver_SRCS})

target_link_libraries(stunserver  ${KDE4_KDECORE_LIBS} cricketbase cricketp2pbase pthread )

install_targets(/bin stunserver )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#libcricketp2pbase_la_SOURCES = stun.cc \
#			       port.cc \
#			       udpport.cc \
#			       tcpport.cc \
#			       helpers.cc \
#			       sessionmanager.cc \
#			       session.cc \
#			       p2psocket.cc \
#			       relayport.cc \
#			       stunrequest.cc \
#			       stunport.cc \
#			       socketmanager.cc 
#
#noinst_HEADERS =               candidate.h \
#                               portallocator.h \
#			       relayport.h \
#		     	       session.h \
#		       	       sessionmessage.h \
#			       stunport.h \
#			       tcpport.h \
#                               helpers.h \
#			       port.h \
#		       	       sessionid.h \
#			       socketmanager.h \
#			       stunrequest.h \
#			       udpport.h \
#                               p2psocket.h \
#			       sessiondescription.h \
#			       sessionmanager.h \
#			       stun.h  \
#			       relayserver.h \
#			       stunserver.h
#
#AM_CPPFLAGS = -DPOSIX  $(all_includes) -I$(srcdir)/../../..
#
#bin_PROGRAMS = relayserver stunserver
#relayserver_SOURCES = relayserver.cc relayserver_main.cc
#relayserver_LDADD = ../../base/libcricketbase.la libcricketp2pbase.la -lpthread
#stunserver_SOURCES = stunserver.cc stunserver_main.cc
#stunserver_LDADD = ../../base/libcricketbase.la libcricketp2pbase.la -lpthread
#
#noinst_LTLIBRARIES = libcricketp2pbase.la
#
#DEFAULT_INCLUDES = -I$(srcdir)/../../..
