## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = libkopeteoscarsocket.la

AM_CPPFLAGS = -Wall -DDATADIR=\"$(kde_datadir)/kopete\"

libkopeteoscarsocket_la_SOURCES = oscarsocket.cpp oncomingsocket.cpp buffer.cpp md5.c tbuddylist.cpp aim.cpp ssidata.cpp oscarconnection.cpp oscardirectconnection.cpp

EXTRA_DIST =
# set the include path for X, qt and KDE
INCLUDES= -I$(top_srcdir)/kopete/protocols/oscar -I$(top_srcdir)/kopete/protocols/oscar/ui -I$(top_builddir)/kopete/protocols/oscar/ui -I$(top_srcdir)/kopete/protocols/oscar/oscarsocket/ $(all_includes)

libkopeteoscarsocket_la_METASOURCES = AUTO


