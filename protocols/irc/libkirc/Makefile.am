noinst_LTLIBRARIES = libkirc.la

libkirc_la_SOURCES = dcchandler.cpp kirc.cpp kircmessage.cpp ksslsocket.cpp
libkirc_la_LDFLAGS = -no-undefined $(KDE_PLUGIN)
libkirc_la_LIBADD = $(all_libraries) $(KDE_RPATH) $(LIB_KIO)

AM_CPPFLAGS = $(all_includes)
METASOURCES = AUTO

#Generates some warnings!!!
KDE_ICON = AUTO

