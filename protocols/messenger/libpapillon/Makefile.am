noinst_LTLIBRARIES = libpapillon.la

AM_CPPFLAGS = $(all_includes)

METASOURCES = AUTO

libpapillon_la_SOURCES = bytestream.cpp connector.cpp coreprotocol.cpp \
		safedelete.cpp stream.cpp task.cpp transfer.cpp notificationstream.cpp qtconnector.cpp \
	qtbytestream.cpp
libpapillon_la_LIBADD = $(LIB_QTCORE) $(LIB_QTNETWORK)
libpapillon_la_LDFLAGS = -no-undefined $(all_libraries)

noinst_HEADERS = papillon_macros.h transfer.h notificationstream.h \
	qtconnector.h qtbytestream.h

