SUBDIRS = libkmerlin ui pixmaps
INCLUDES = -I$(srcdir)/ui \
        -I./ui \
        -I$(srcdir)/libkmerlin \
        -I./libkmerlin \
        -I$(srcdir)/../../library \
        -I$(srcdir)/../../library/ui \
        $(all_includes)
lib_LTLIBRARIES = libkopetemsn.la

libkopetemsn_la_SOURCES     = msnprotocol.cpp msnpreferences.cpp protocol.cpp msnaddcontactpage.cpp msncontact.cpp msnmessage.cpp msnuser.cpp
libkopetemsn_la_METASOURCES = AUTO
libkopetemsn_la_LDFLAGS     = 
libkopetemsn_la_LIBADD      = ../../library/libkopete.la \
        ./ui/libkopetemsnui.la \
        ./libkmerlin/libkmerlin.la

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete

