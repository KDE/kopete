noinst_LTLIBRARIES = libkopeteircui.la
AM_CPPFLAGS = -I$(top_srcdir)/kopete/libkopete\
	-I$(top_srcdir)/kopete/libkopete/ui\
	-I$(srcdir)/..\
	-I$(top_srcdir)/kopete/protocols/irc/libkirc \
	-I$(top_srcdir)/kopete/protocols/irc \
	$(all_includes)

libkopeteircui_la_METASOURCES = AUTO
libkopeteircui_la_SOURCES = ircadd.ui ircprefs.ui ircmessagebase.ui ircchatwindow.cpp  dccconfirmbase.ui ircsendbase.ui ircqueryview.cpp dccconfirm.cpp ircreceivebase.ui empty.cpp
EXTRA_DIST = ircadd.ui ircprefs.ui empty.cpp
