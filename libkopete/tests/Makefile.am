
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)

LDADD = $(LIB_KIO)

METASOURCES = AUTO

check_PROGRAMS = kopeteemoticontest kopetewallettest kopetepasswordtest kopetelinktest

kopeteemoticontest_SOURCES = kopeteemoticontest.cpp
kopeteemoticontest_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopeteemoticontest_LDADD = ../libkopete.la

kopetewallettest_SOURCES = kopetewallettest.cpp
kopetewallettest_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetewallettest_LDADD = ../libkopete.la

kopetepasswordtest_SOURCES = kopetepasswordtest.cpp
kopetepasswordtest_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetepasswordtest_LDADD = ../libkopete.la

kopetelinktest_SOURCES = kopetelinktest.cpp
kopetelinktest_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopetelinktest_LDADD = ../libkopete.la
