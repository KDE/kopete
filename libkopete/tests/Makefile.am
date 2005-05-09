AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)
METASOURCES = AUTO

check_LTLIBRARIES = kunittest_kopetelinktest.la kunittest_kopetepropertiestest.la kunittest_kopeteemoticontest.la

check_PROGRAMS = kopetewallettest_program kopetepasswordtest_program

kunittest_kopetelinktest_la_SOURCES = kopetelinktest.cpp
kunittest_kopetelinktest_la_LIBADD = -lkunittest ../libkopete.la
kunittest_kopetelinktest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_kopetepropertiestest_la_SOURCES = kopetepropertiestest.cpp ../kopeteproperties.cpp
kunittest_kopetepropertiestest_la_LIBADD = -lkunittest ../libkopete.la
kunittest_kopetepropertiestest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_kopeteemoticontest_la_SOURCES = kopeteemoticontest.cpp
kunittest_kopeteemoticontest_la_LIBADD = -lkunittest ../libkopete.la
kunittest_kopeteemoticontest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kopetewallettest_program_SOURCES = kopetewallettest_program.cpp
kopetewallettest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetewallettest_program_LDADD = ../libkopete.la

kopetepasswordtest_program_SOURCES = kopetepasswordtest_program.cpp
kopetepasswordtest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetepasswordtest_program_LDADD = ../libkopete.la

noinst_HEADERS = kopetelinktest.h kopetepropertiestest.h kopeteemoticontest.h

check-local:
	kunittestmodrunner
guicheck:
	kunittestmod $(PWD)

