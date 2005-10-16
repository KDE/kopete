
noinst_LTLIBRARIES = libskypedbus.la

AM_CPPFLAGS = $(all_includes) $(DBUS_INCS)
METASOURCES = AUTO

libskypedbus_la_SOURCES = skypeconnection.cpp
libskypedbus_la_LDFLAGS = -module -Wundef -ldbus-1 -ldbus-qt-1

noinst_HEADERS = skypeconnection.h
