SUBDIRS = tests
noinst_LTLIBRARIES = libpapillon.la

AM_CPPFLAGS = $(all_includes) $(QCA_CFLAGS)

METASOURCES = AUTO

libpapillon_la_SOURCES = bytestream.cpp connector.cpp coreprotocol.cpp \
		safedelete.cpp stream.cpp task.cpp transfer.cpp papillonclientstream.cpp \
		qtconnector.cpp qtbytestream.cpp connection.cpp logintask.cpp securestream.cpp \
		tweenerhandler.cpp client.cpp notifymessagetask.cpp mimeheader.cpp contactlistmanager.cpp \
		contactinfo.cpp
libpapillon_la_LIBADD = $(LIB_QTCORE) $(LIB_QTNETWORK) $(QCA_LIBS)
libpapillon_la_LDFLAGS = -no-undefined $(all_libraries)

noinst_HEADERS = papillon_macros.h transfer.h papillonclientstream.h \
							qtconnector.h qtbytestream.h connection.h logintask.h 	securestream.h \
			tweenerhandler.h client.h notifymessagetask.h mimeheader.h

