METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/kopete/libkopete \
	-I$(top_srcdir)/kopete/libkopete/ui \
	$(all_includes)

noinst_LTLIBRARIES = libkopeteaddcontactwizard.la
noinst_HEADERS = addcontactwizard.h protocolboxitem.h

libkopeteaddcontactwizard_la_SOURCES = addcontactwizard_base.ui \
	addcontactwizard.cpp protocolboxitem.cpp
libkopeteaddcontactwizard_la_LDFLAGS = $(all_libraries)
libkopeteaddcontactwizard_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

# vim: set noet:

