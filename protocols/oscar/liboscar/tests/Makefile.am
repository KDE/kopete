INCLUDES = -I$(top_srcdir)/kopete/protocols/oscar/liboscar -I$(top_builddir)/kopete/protocols/oscar/liboscar \
 -I$(top_srcdir)/kopete/libkopete $(all_includes)
METASOURCES = AUTO
check_PROGRAMS = logintest userinfotest ssigrouptest redirecttest ipaddrtest

clientstream_test_SOURCES = clientstream_test.cpp
clientstream_test_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

logintest_SOURCES = logintest.cpp
logintest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

userinfotest_SOURCES = userinfotest.cpp
userinfotest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

ssigrouptest_SOURCES = ssigrouptest.cpp
ssigrouptest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

#ssitest_SOURCES = ssitest.cpp
#ssitest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

redirecttest_SOURCES = redirecttest.cpp
redirecttest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

ipaddrtest_SOURCES = ipaddrtest.cpp
ipaddrtest_LDADD = $(LIB_QT) $(LIB_KDECORE) ../liboscar.la

