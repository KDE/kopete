bin_PROGRAMS = call 
call_CXXFLAGS = $(AM_CXXFLAGS)
call_SOURCES = call_main.cc callclient.cc console.cc presencepushtask.cc presenceouttask.cc
noinst_HEADERS = callclient.h  console.h  presenceouttask.h  presencepushtask.h  status.h
call_LDADD =  \
	       $(srcdir)/../../../talk/examples/login/libcricketexampleslogin.la \
	       $(srcdir)/../../../talk/session/phone/libcricketsessionphone.la \
	       $(srcdir)/../../../talk/p2p/client/libcricketp2pclient.la \
	       $(srcdir)/../../../talk/p2p/base/libcricketp2pbase.la \
	       $(srcdir)/../../../talk/xmpp/libcricketxmpp.la \
	       $(srcdir)/../../../talk/xmllite/libcricketxmllite.la \
	       $(srcdir)/../../../talk/base/libcricketbase.la \
	       $(srcdir)/../../../talk/third_party/mediastreamer/libmediastreamer.la \
	       $(EXPAT_LIBS) $(ORTP_LIBS) -lpthread $(ILBC_LIBS) $(SPEEX_LIBS) $(GLIB_LIBS) -lasound 
AM_CPPFLAGS  = -DPOSIX
DEFAULT_INCLUDES = -I$(srcdir)/../../..