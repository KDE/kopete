SUBDIRS = tests
METASOURCES = AUTO
noinst_LTLIBRARIES = libkyahoo.la

AM_CPPFLAGS = $(all_includes)


libkyahoo_la_SOURCES = client.cpp task.cpp connector.cpp inputprotocolbase.cpp \
	ymsgprotocol.cpp ymsgtransfer.cpp transfer.cpp rtf.cc yahoobytestream.cpp bytestream.cpp yahooclientstream.cpp yahooconnector.cpp\
	safedelete.cpp stream.cpp sha.c md5.c crypt.c challengeresponsev10.cpp coreprotocol.cpp 
libkyahoo_la_LDFLAGS = -no-undefined $(all_libraries)
libkyahoo_la_LIBADD = $(LIB_QT)

