METASOURCES = AUTO
AM_CPPFLAGS = -I$(top_srcdir)/kopete/libkopete \
    -I$(top_srcdir)/kopete/libkopete/ui \
    -I$(top_builddir)/kopete/libkopete/ui \
    -I$(top_srcdir)/kopete/protocols/sms \
    -I../../../libkopete/ui \
    -I$(top_builddir)/kopete/protocols/sms/services \
    -I.. \
    $(all_includes)

noinst_LTLIBRARIES = libkopetesmsservices.la

libkopetesmsservices_la_SOURCES = smssend.cpp smssendprefs.ui smssendprovider.cpp \
	smsclient.cpp smsclientprefs.ui smssendarg.ui

