INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src $(all_includes)
METASOURCES = coreprotocol.moc task.moc client.moc safedelete.moc gwclientstream.moc
lib_LTLIBRARIES = libgroupwise.la libgwtest.la
libgroupwise_la_LDFLAGS = -no-undefined $(KDE_PLUGIN) $(all_libraries)
noinst_HEADERS = connector.h tlshandler.h qcatlshandler.h bytestream.h \
	gwclientstream.h securestream.h stream.h coreprotocol.h gwfield.h gwerror.h \
	usertransfer.h eventtransfer.h transfer.h request.h requestfactory.h client.h task.h \
	safedelete.h
libgroupwise_la_SOURCES = connector.cpp bytestream.cpp 					tlshandler.cpp \
		qcatlshandler.cpp gwclientstream.cpp securestream.cpp stream.cpp 			coreprotocol.cpp \
		gwfield.cpp transferbase.cpp usertransfer.cpp 	eventtransfer.cpp transfer.cpp \
		request.cpp requestfactory.cpp safedelete.cpp response.cpp

SUBDIRS = qca tests
AM_CFLAGS = -DUSE_TLSHANDLER

libgwtest_la_SOURCES = coreprotocol.cpp eventtransfer.cpp \
	gwfield.cpp	request.cpp requestfactory.cpp transfer.cpp usertransfer.cpp client.cpp task.cpp \
	safedelete.cpp gwclientstream.cpp qcatlshandler.cpp
libgwtest_la_LIBADD = -lqt-mt
libgwtest_la_LDFLAGS = $(all_libraries)
libgroupwise_la_LIBADD = -lqt-mt qca/src/libqca.la -lkdecore

libgroupwise_la_COMPILE_FIRST = securestream.moc

CLEANFILES = securestream.moc
securestream.moc: $(srcdir)/securestream.cpp $(srcdir)/securestream.h
	${MOC} $(srcdir)/securestream.h > $@
	${MOC} $(srcdir)/securestream.cpp >> $@

