METASOURCES = AUTO
SUBDIRS = ui icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-I./ui \
	$(all_includes) \
	$(MEANWHILE_INCLUDES)

noinst_HEADERS = \
	meanwhileprotocol.h \
	meanwhileaddcontactpage.h \
	meanwhileeditaccountwidget.h \
	meanwhileaccount.h \
	meanwhilecontact.h \
	meanwhilelibrary.h \
	meanwhileplugin.h \
	meanwhileserver.h

kde_module_LTLIBRARIES =  kopete_meanwhile.la

kopete_meanwhile_la_SOURCES = \
	meanwhileprotocol.cpp \
	meanwhileaddcontactpage.cpp \
	meanwhileeditaccountwidget.cpp \
	meanwhileaccount.cpp \
	meanwhilecontact.cpp \
	meanwhilelibrary.cpp \
	meanwhileplugin.cpp \
	meanwhileserver.cpp

kopete_meanwhile_la_LDFLAGS = -no-undefined -module \
	$(KDE_PLUGIN) $(all_libraries) 

kopete_meanwhile_la_LIBADD = $(top_builddir)/kopete/libkopete/libkopete.la \
	ui/libkopetemeanwhileui.la $(MEANWHILE_LIBS)

service_DATA = kopete_meanwhile.desktop
servicedir= $(kde_servicesdir)
