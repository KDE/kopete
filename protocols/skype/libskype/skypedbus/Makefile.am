
noinst_LTLIBRARIES = libskypedbus.la

AM_CPPFLAGS = $(all_includes) $(DBUS_INCS)
METASOURCES = AUTO

libskypedbus_la_SOURCES = skypeconnection.cpp connection.cpp message.cpp integrator.cpp
libskypedbus_la_LDFLAGS = -module -ldbus-1

noinst_HEADERS = skypeconnection.h
