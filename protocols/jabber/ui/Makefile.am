METASOURCES = AUTO
INCLUDES = -I$(srcdir)/../../../libkopete \
	-I$(srcdir)/../../../libkopete/ui \
	-I../../../libkopete/ui \
	-I$(srcdir)/.. \
	$(all_includes)

noinst_LTLIBRARIES = libkopetejabberui.la

libkopetejabberui_la_SOURCES = dlgjabberprefs.cpp dlgpreferences.ui \
	dlgjabberstatus.cpp dlgstatus.ui \
	dlgjabbersendraw.cpp dlgsendraw.ui \
	jabberaddcontactpage.cpp dlgaddcontact.ui \
	dlgjabberrename.cpp dlgrename.ui \
	dlgjabbervcard.cpp dlgvcard.ui \
	dlgjabberservices.cpp dlgservices.ui \
	empty.cpp

EXTRA_DIST = dlgpreferences.ui \
	dlgstatus.ui \
	dlgsendraw.ui \
	dlgaddcontact.ui \
	dlgrename.ui \
	dlgvcard.ui \
	dlgservices.ui
