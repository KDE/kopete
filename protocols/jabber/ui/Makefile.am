METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/../libxmpp/xmpp/src \
	-I$(srcdir)/.. \
	$(all_includes)

noinst_LTLIBRARIES = libkopetejabberui.la

libkopetejabberui_la_SOURCES = \
	dlgjabbersendraw.cpp dlgsendraw.ui \
	jabberaddcontactpage.cpp dlgaddcontact.ui \
	dlgjabbervcard.cpp dlgvcard.ui \
	dlgjabberservices.cpp dlgservices.ui \
	dlgjabberregister.cpp dlgregister.ui \
	dlgjabberbrowse.cpp dlgbrowse.ui \
	dlgjabberchatjoin.cpp dlgchatjoin.ui \
	jabbereditaccountwidget.cpp dlgjabbereditaccountwidget.ui \
	empty.cpp

EXTRA_DIST = dlgjabbereditaccountwidget.ui \
	dlgsendraw.ui \
	dlgaddcontact.ui \
	dlgrename.ui \
	dlgvcard.ui \
	dlgservices.ui \
	dlgregister.ui \
	dlgbrowse.ui
