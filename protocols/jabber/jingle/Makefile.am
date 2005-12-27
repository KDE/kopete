METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/../libiris/iris/include \
	-I$(srcdir)/../libiris/iris/xmpp-im \
	-I$(srcdir)/../libiris/iris/jabber \
	-I$(srcdir)/../libiris/qca/src \
	-I$(srcdir)/../libiris/cutestuff/util \
	-I$(srcdir)/../libiris/libjingle \
	-I$(srcdir)/.. \
	$(all_includes)

noinst_LTLIBRARIES = libkopetejabberjingle.la 

libkopetejabberjingle_la_SOURCES = jinglevoicecaller.cpp  ui_voicecall.ui  voicecalldlg.cpp


EXTRA_DIST = dlgjabbereditaccountwidget.ui \
	dlgsendraw.ui \
	dlgaddcontact.ui \
	dlgrename.ui \
	dlgvcard.ui \
	dlgservices.ui \
	dlgregister.ui \
	dlgbrowse.ui
