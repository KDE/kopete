AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
	$(KOPETE_INCLUDES) $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/protocols/msn/tests\" \
	-I./..

METASOURCES = AUTO

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
LDADD = -lQtTest_debug ../libkopete_msn_shared.la 

check_PROGRAMS = challengetest
TESTS = challengetest

challengetest_SOURCES = challengetest.cpp
