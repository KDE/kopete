METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) -I$(top_srcdir)/kopete/kopete/addcontactwizard

noinst_LTLIBRARIES = libkopeteaddcontactwizard.la
noinst_HEADERS = addcontactwizard.h 

libkopeteaddcontactwizard_la_SOURCES = addcontactwizard_base.ui \
	addcontactwizard.cpp 
libkopeteaddcontactwizard_la_LDFLAGS = $(all_libraries)
libkopeteaddcontactwizard_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

# vim: set noet:

