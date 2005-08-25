METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/../ \
	-I$(srcdir)/../../ \
	-I$(srcdir)/ui/ \
	-I$(srcdir)/../../liboscar \
	-I$(srcdir)/../../../../ \
	$(KOPETE_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkopeteicqui.la

libkopeteicqui_la_SOURCES = icqadd.ui icqeditaccountui.ui \
	icqeditaccountwidget.cpp icqgeneralinfo.ui icqotherinfowidget.ui icqworkinfowidget.ui icqinterestinfowidget.ui\
	icquserinfowidget.cpp icqauthreplyui.ui icqauthreplydialog.cpp icqaddcontactpage.cpp \
		icqsearchbase.ui icqsearchdialog.cpp icqsearchdialog.h

libkopeteicqui_la_LIBADD = $(top_builddir)/kopete/libkopete/libkopete.la

