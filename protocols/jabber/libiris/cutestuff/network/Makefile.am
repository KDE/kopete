METASOURCES = AUTO

KDE_CXXFLAGS = $(QCA_CFLAGS)

noinst_LTLIBRARIES = libcutestuff_network.la
INCLUDES = -I$(srcdir)/../util $(all_includes)

libcutestuff_network_la_LIDBADD = $(QCA_LIBS) -I$(srcdir)/../util/libcutestuff_util.la
libcutestuff_network_la_SOURCES = \
	bsocket.cpp \
	httpconnect.cpp \
	httppoll.cpp \
	ndns.cpp \
	servsock.cpp \
	socks.cpp \
	srvresolver.cpp

KDE_OPTIONS = nofinal

