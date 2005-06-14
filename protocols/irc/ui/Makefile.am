METASOURCES = AUTO

noinst_LTLIBRARIES = libkopeteircui.la
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/..\
	-I$(srcdir)/../libkirc \
	$(all_includes)

libkopeteircui_la_SOURCES = \
	ircadd.ui \
	ircaddcontactpage.cpp \
	irceditaccountwidget.cpp \
	irceditaccount.ui \
	channellist.cpp \
	channellistdialog.cpp \
	networkconfig.ui \
	networkconfigwidget.cpp \
	kcodecaction.cpp

EXTRA_DIST = ircadd.ui ircprefs.ui
