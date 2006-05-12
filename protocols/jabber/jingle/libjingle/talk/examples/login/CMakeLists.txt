



########### next target ###############


########### next target ###############

set(login_SRCS 
   login_main.cc 
   xmppsocket.cc 
   xmppthread.cc 
   xmpppump.cc 
   xmppauth.cc )

kde4_automoc(${login_SRCS})

kde4_add_executable(login ${login_SRCS})

target_link_libraries(login  ${KDE4_KDECORE_LIBS} cricketxmpp cricketxmllite cricketbase pthread )

install_targets(/bin login )


########### install files ###############





#original Makefile.am contents follow:

#noinst_LTLIBRARIES= libcricketexampleslogin.la
#libcricketexampleslogin_la_SOURCES = xmppsocket.cc \
#				     xmppauth.cc \
#				     xmppthread.cc \
#				     xmpppump.cc 
#noinst_HEADERS = xmppauth.h  xmpppump.h  xmppsocket.h  xmppthread.h
#bin_PROGRAMS = login
#login_CXXFLAGS = $(AM_CXXFLAGS)
#login_SOURCES = login_main.cc xmppsocket.cc xmppthread.cc xmpppump.cc xmppauth.cc
#login_LDADD =  $(srcdir)/../../../talk/xmpp/libcricketxmpp.la \
#	       $(srcdir)/../../../talk/xmllite/libcricketxmllite.la \
#	       $(srcdir)/../../../talk/base/libcricketbase.la \
#	       $(EXPAT_LIBS) -lpthread 
#AM_CPPFLAGS  = -DPOSIX
#DEFAULT_INCLUDES = -I$(srcdir)/../../..
