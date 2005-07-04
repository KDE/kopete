INCLUDES = $(all_includes) $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/oscar/liboscar
METASOURCES = AUTO
check_PROGRAMS = kunittest clientstream_test logintest userinfotest ssigrouptest redirecttest ipaddrtest

kunittest_SOURCES = main.cpp kunittest.cpp chatnavtests.cpp
kunittest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kunittest_LDADD = $(LIB_KDECORE) ../liboscar.la

clientstream_test_SOURCES = clientstream_test.cpp
clientstream_test_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

logintest_SOURCES = logintest.cpp
logintest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

userinfotest_SOURCES = userinfotest.cpp
userinfotest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

ssigrouptest_SOURCES = ssigrouptest.cpp
ssigrouptest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

redirecttest_SOURCES = redirecttest.cpp
redirecttest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

ipaddrtest_SOURCES = ipaddrtest.cpp
ipaddrtest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

check: kunittest
	@./kunittest 2>&1 | grep "tests:"
