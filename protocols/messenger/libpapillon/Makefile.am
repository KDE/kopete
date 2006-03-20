SUBDIRS = tests
noinst_LTLIBRARIES = libpapillon.la

AM_CPPFLAGS = $(all_includes)

METASOURCES = AUTO

libpapillon_la_SOURCES = bytestream.cpp connector.cpp coreprotocol.cpp \
				safedelete.cpp stream.cpp task.cpp transfer.cpp papillonclientstream.cpp \
		qtconnector.cpp 	qtbytestream.cpp connection.cpp notificationconnection.cpp \
	logintask.cpp
libpapillon_la_LIBADD = $(LIB_QTCORE) $(LIB_QTNETWORK)
libpapillon_la_LDFLAGS = -no-undefined $(all_libraries)

noinst_HEADERS = papillon_macros.h transfer.h papillonclientstream.h \
			qtconnector.h qtbytestream.h connection.h notificationconnection.h logintask.h

