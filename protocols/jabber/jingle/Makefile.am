SUBDIRS = libjingle
METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/../libiris/iris/include \
	-I$(srcdir)/../libiris/iris/xmpp-im \
	-I$(srcdir)/../libiris/iris/jabber \
	-I$(srcdir)/../libiris/qca/src \
	-I$(srcdir)/../libiris/cutestuff/util \
	-I$(srcdir)/libjingle \
	-I$(srcdir)/.. \
	$(all_includes)

noinst_LTLIBRARIES = libkopetejabberjingle.la 

# libkopetejabberjingle_la_SOURCES = jinglevoicecaller.cpp  \
# 	jinglewatchsessiontask.cpp jinglesession.cpp jinglevoicesession.cpp jinglesessionmanager.cpp \
# 	jinglevoicesessiondialogbase.ui jinglevoicesessiondialog.cpp

libkopetejabberjingle_la_SOURCES = jinglevoicecaller.cpp jinglevoicesessiondialogbase.ui jinglevoicesessiondialog.cpp

libkopetejabberjingle_la_LIBADD = libjingle/talk/session/phone/libcricketsessionphone.la \
	libjingle/talk/p2p/client/libcricketp2pclient.la \
	libjingle/talk/p2p/base/libcricketp2pbase.la \
	libjingle/talk/xmpp/libcricketxmpp.la \
	libjingle/talk/xmllite/libcricketxmllite.la \
	libjingle/talk/base/libcricketbase.la \
	libjingle/talk/third_party/mediastreamer/libmediastreamer.la  \
	$(EXPAT_LIBS) $(ORTP_LIBS) -lpthread $(ILBC_LIBS) $(SPEEX_LIBS) $(GLIB_LIBS) $(ALSA_LIBS)
