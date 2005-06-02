SUBDIRS = mock .
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/tests/mock $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/libkopete/tests\"
METASOURCES = AUTO

check_LTLIBRARIES = kunittest_kopetemessage_test.la kunittest_kopetepropertiestest.la kunittest_kopetecontactlist_test.la 
noinst_LTLIBRARIES = kunittest_kopeteemoticontest.la

check_PROGRAMS = kopetewallettest_program kopetepasswordtest_program

kunittest_kopetepropertiestest_la_SOURCES = kopetepropertiestest.cpp ../kopeteproperties.cpp
kunittest_kopetepropertiestest_la_LIBADD = -lkunittest ../libkopete.la
kunittest_kopetepropertiestest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_kopeteemoticontest_la_SOURCES = kopeteemoticontest.cpp
kunittest_kopeteemoticontest_la_LIBADD = -lkunittest ../libkopete.la
kunittest_kopeteemoticontest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_kopetemessage_test_la_SOURCES = kopetemessage_test.cpp
kunittest_kopetemessage_test_la_LIBADD = -lkunittest mock/libkopete_mock.la
kunittest_kopetemessage_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kopetewallettest_program_SOURCES = kopetewallettest_program.cpp
kopetewallettest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetewallettest_program_LDADD = ../libkopete.la

kopetepasswordtest_program_SOURCES = kopetepasswordtest_program.cpp
kopetepasswordtest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetepasswordtest_program_LDADD = ../libkopete.la

kunittest_kopetecontactlist_test_la_SOURCES = kopetecontactlist_test.cpp
kunittest_kopetecontactlist_test_la_LIBADD = -lkunittest mock/libkopete_mock.la
kunittest_kopetecontactlist_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

noinst_HEADERS = kopetepropertiestest.h kopeteemoticontest.h

check-local: 
	kunittestmodrunner
guicheck:
	kunittestmod $(PWD)

