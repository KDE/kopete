## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = libkopeteoscarsocket.la

AM_CPPFLAGS = -I../ -I../ui -I../oscarsocket/ -Wall -DDATADIR=\"$(kde_datadir)/kopete\"

libkopeteoscarsocket_la_SOURCES = oscarsocket.cpp oncomingsocket.cpp buffer.cpp md5.c tbuddylist.cpp aim.cpp ssidata.cpp oscarconnection.cpp oscardirectconnection.cpp

EXTRA_DIST =
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

libkopeteoscarsocket_la_METASOURCES = AUTO


