INCLUDES = -I$(top_srcdir)/kopete/protocols/yahoo/libkyahoo $(all_includes)
METASOURCES = AUTO
noinst_PROGRAMS = clientstream_test login_test

clientstream_test_SOURCES = clientstream_test.cpp
clientstream_test_LDADD = $(LIB_QT) $(LIB_KDECORE) ../libkyahoo.la

login_test_SOURCES = logintest.cpp
login_test_LDADD = $(LIB_QT) $(LIB_KDECORE) ../libkyahoo.la
