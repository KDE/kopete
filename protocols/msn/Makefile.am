SUBDIRS = libkmerlin ui pixmaps
INCLUDES = -I$(top_srcdir)/kopete/protocols/msn/ui \
        -I$(top_builddir)/kopete/protocols/msn/ui \
        -I$(top_builddir)/kopete/protocols/msn/libkmerlin \
        -I$(top_srcdir)/kopete/library \
        -I$(top_srcdir)/kopete/library/ui \
        -I$(top_srcdir)/kopete/protocols/msn/libkmerlin \
        $(all_includes)
lib_LTLIBRARIES = libkopetemsn.la

libkopetemsn_la_SOURCES     = msnprotocol.cpp msnpreferences.cpp protocol.cpp msnaddcontactpage.cpp msncontact.cpp msnmessage.cpp msnuser.cpp
libkopetemsn_la_METASOURCES = AUTO
libkopetemsn_la_LDFLAGS     = 
libkopetemsn_la_LIBADD      = $(top_builddir)/kopete/library/libkopete.la \
        $(top_builddir)/kopete/protocols/msn/ui/libkopetemsnui.la \
        $(top_builddir)/kopete/protocols/msn/libkmerlin/libkmerlin.la

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete

