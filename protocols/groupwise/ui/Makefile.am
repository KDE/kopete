INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise \
	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src
METASOURCES = AUTO
noinst_LTLIBRARIES = libkopetegroupwiseui.la
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
			  -I$(srcdir)/..\
			  -I$(srcdir)/../libgroupwise \
			  $(all_includes)

libkopetegroupwiseui_la_LDFLAGS = $(all_libraries)
libkopetegroupwiseui_la_SOURCES = gwaccountpreferences.ui gwaddcontactpage.cpp \
	gwaddui.ui gweditaccountwidget.cpp gwreceiveinvitationdialog.cpp \
	gwshowinvitation.ui

noinst_HEADERS = gwreceiveinvitationdialog.h
