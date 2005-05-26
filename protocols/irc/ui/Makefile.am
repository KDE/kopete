METASOURCES = AUTO

noinst_LTLIBRARIES = libkopeteircui.la
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/..\
	-I$(srcdir)/../libkirc \
	$(all_includes)

libkopeteircui_la_SOURCES = \
	ircadd.ui \
	irceditaccountwidget.cpp \
	irceditaccount.ui \
	channellist.cpp \
	channellistdialog.cpp \
	networkconfig.ui \
	kcodecaction.cpp

EXTRA_DIST = ircadd.ui ircprefs.ui
