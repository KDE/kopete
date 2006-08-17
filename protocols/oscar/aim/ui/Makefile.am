METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
    -I$(srcdir)/.. \
	-I$(srcdir)/../.. \
    -I$(srcdir)/../../liboscar \
	$(all_includes)

noinst_LTLIBRARIES = libkopeteaimui.la

libkopeteaimui_la_SOURCES = aimaddcontactui.ui aimeditaccountui.ui \
	aiminfobase.ui aimjoinchatbase.ui aimaddcontactpage.cpp aimeditaccountwidget.cpp

libkopeteaimui_la_LIBADD = $(top_builddir)/kopete/libkopete/libkopete.la


