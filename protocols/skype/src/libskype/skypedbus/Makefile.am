
noinst_LTLIBRARIES = libskypedbus.la

AM_CPPFLAGS = $(all_includes) $(DBUS_INCS) -DOLD_DBUS
METASOURCES = AUTO

libskypedbus_la_SOURCES = skypeconnection.cpp message.cpp integrator.cpp connection.cpp
libskypedbus_la_LDFLAGS = -module -Wundef $(DBUS_LIBS)

noinst_HEADERS = skypeconnection.h
