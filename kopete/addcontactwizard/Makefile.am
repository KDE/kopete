METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkopeteaddcontactwizard.la
noinst_HEADERS = addcontactwizard.h 

libkopeteaddcontactwizard_la_SOURCES = addcontactwizard_base.ui addcontactwizard.cpp 
libkopeteaddcontactwizard_la_LDFLAGS = $(all_libraries) -no-undefined
libkopeteaddcontactwizard_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI) $(LIB_KABC)

# vim: set noet:
