noinst_LTLIBRARIES = libkirc.la

libkirc_la_SOURCES = \
	kirc.cpp \
	kirc_commands.cpp \
	kirc_ctcp.cpp \
	kirc_numericreplies.cpp \
	kircentity.cpp \
	kircmessage.cpp \
	kirctransfer.cpp \
	kirctransferhandler.cpp \
	kirctransferserver.cpp \
	ksslsocket.cpp
libkirc_la_LDFLAGS = -no-undefined $(KDE_PLUGIN)
libkirc_la_LIBADD = $(all_libraries) $(KDE_RPATH) $(LIB_KIO)

AM_CPPFLAGS = -I$(top_srcdir)/kopete/protocols/irc $(KOPETE_INCLUDES) $(all_includes)
METASOURCES = AUTO

#Generates some warnings!!!
KDE_ICON = AUTO

