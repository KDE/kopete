METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/msn/ $(all_includes)

noinst_LTLIBRARIES =  libkopete_msnslp.la

libkopete_msnslp_la_SOURCES = application.cpp \
	binarypacketformatter.cpp dialog.cpp \
	filetransfersession.cpp sessionclient.cpp \
	sessionnotifier.cpp switchboardbridge.cpp \
	tcptransportbridge.cpp transport.cpp \
	transportbridge.cpp mediasession.cpp message.cpp \
	packet.cpp packetqueue.cpp packetscheduler.cpp session.cpp \
	transaction.cpp slpmessage.cpp \
	slprequest.cpp slpresponse.cpp \
	udisession.cpp

noinst_HEADERS = application.h \
	binarypacketformatter.h dialog.h \
	filetransfersession.h sessionclient.h \
	sessionnotifier.h switchboardbridge.h \
	tcptransportbridge.h transport.h \
	transportbridge.h mediasession.h message.h \
	packet.h packetqueue.h packetscheduler.h session.h \
	transaction.h slpmessage.h \
	slprequest.h slpresponse.h \
	udisession.h
