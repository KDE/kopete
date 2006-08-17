METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/../libiris/iris/include \
	-I$(srcdir)/../libiris/iris/xmpp-im \
	-I$(srcdir)/../libiris/iris/jabber \
	-I$(srcdir)/../libiris/qca/src \
	-I$(srcdir)/../libiris/cutestuff/util \
	-I$(srcdir)/.. \
	$(all_includes)

noinst_LTLIBRARIES = libkopetejabberui.la

libkopetejabberui_la_SOURCES = dlgsendraw.ui dlgjabbersendraw.cpp \
	dlgaddcontact.ui jabberaddcontactpage.cpp dlgvcard.ui dlgjabbervcard.cpp \
	dlgjabberservices.cpp dlgregister.ui dlgjabberregister.cpp dlgbrowse.ui dlgjabberbrowse.cpp \
	dlgjabbereditaccountwidget.ui jabbereditaccountwidget.cpp dlgjabberregisteraccount.ui \
	jabberregisteraccount.cpp dlgjabberchooseserver.ui jabberchooseserver.cpp dlgchangepassword.ui \
	dlgjabberchangepassword.cpp empty.cpp dlgchatroomslist.ui dlgjabberchatroomslist.cpp dlgchatjoin.ui \
	dlgjabberchatjoin.cpp dlgservices.ui

EXTRA_DIST = dlgjabbereditaccountwidget.ui \
	dlgsendraw.ui \
	dlgaddcontact.ui \
	dlgrename.ui \
	dlgvcard.ui \
	dlgservices.ui \
	dlgregister.ui \
	dlgbrowse.ui


noinst_HEADERS = dlgjabberchatroomslist.h dlgjabberchatjoin.h
