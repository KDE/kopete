METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/msn/ $(all_includes)

noinst_LTLIBRARIES =  libkopete_msnslp.la

libkopete_msnslp_la_SOURCES = \
	binarypacketformatter.cpp dialog.cpp \
	sessionclient.cpp \
	sessionnotifier.cpp switchboardbridge.cpp \
	tcptransportbridge.cpp transport.cpp \
	transportbridge.cpp message.cpp \
	packet.cpp packetqueue.cpp packetscheduler.cpp session.cpp \
	transaction.cpp slpmessage.cpp \
	slprequest.cpp slpresponse.cpp \
	msnobjectsession.cpp filetransfersession.cpp \
	msnobjectstore.cpp

noinst_HEADERS = \
	binarypacketformatter.h dialog.h \
	sessionclient.h \
	sessionnotifier.h switchboardbridge.h \
	tcptransportbridge.h transport.h \
	transportbridge.h message.h \
	packet.h packetqueue.h packetscheduler.h session.h \
	transaction.h slpmessage.h \
	slprequest.h slpresponse.h \
	msnobjectsession.h filetransfersession.h \
	msnobjectstore.h
