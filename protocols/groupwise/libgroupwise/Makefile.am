INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
		   -I$(top_srcdir)/kopete/protocols/groupwise \
		   $(all_includes)
METASOURCES = coreprotocol.moc task.moc client.moc safedelete.moc gwclientstream.moc

AM_CFLAGS = -DUSE_TLSHANDLER

CLEANFILES = securestream.moc

SUBDIRS = qca tasks tests

securestream.moc: $(srcdir)/securestream.cpp $(srcdir)/securestream.h
	${MOC} $(srcdir)/securestream.h > $@
	${MOC} $(srcdir)/securestream.cpp >> $@

noinst_HEADERS = connector.h tlshandler.h qcatlshandler.h bytestream.h \
		gwclientstream.h securestream.h stream.h coreprotocol.h gwfield.h gwerror.h \
		usertransfer.h eventtransfer.h transfer.h request.h requestfactory.h client.h task.h \
		safedelete.h response.h rtf2html.h

noinst_LTLIBRARIES = libgroupwise.la libgwtest.la
libgroupwise_la_COMPILE_FIRST = securestream.moc 
libgroupwise_la_SOURCES = connector.cpp bytestream.cpp tlshandler.cpp \
			qcatlshandler.cpp gwclientstream.cpp securestream.cpp stream.cpp coreprotocol.cpp \
			gwfield.cpp transferbase.cpp usertransfer.cpp eventtransfer.cpp transfer.cpp \
			request.cpp requestfactory.cpp safedelete.cpp response.cpp client.cpp task.cpp \
	rtf.cc
libgroupwise_la_LDFLAGS = -no-undefined $(all_libraries)
libgroupwise_la_LIBADD = tasks/libgroupwise_tasks.la -lqt-mt qca/src/libqca.la \

tests_COMPILE_FIRST = libgroupwise.la libgwtest.la

libgwtest_la_SOURCES = coreprotocol.cpp eventtransfer.cpp \
			gwfield.cpp	request.cpp requestfactory.cpp transfer.cpp usertransfer.cpp \
			client.cpp task.cpp safedelete.cpp gwclientstream.cpp qcatlshandler.cpp \
			stream.cpp tlshandler.cpp response.cpp connector.cpp securestream.cpp \
			bytestream.cpp
libgwtest_la_LDFLAGS = $(all_libraries) -no-undefined
libgwtest_la_LIBADD = qca/src/libqca.la \
	$(top_builddir)/kopete/protocols/groupwise/libgroupwise/tasks/libgroupwise_tasks.la -lqt-mt
