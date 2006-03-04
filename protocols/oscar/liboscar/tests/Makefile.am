INCLUDES = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/oscar/liboscar $(all_includes)
METASOURCES = AUTO
check_PROGRAMS = buffertest serverversionstest connectiontest rateinfotest
TESTS = buffertest serverversionstest connectiontest rateinfotest
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

LDADD = ../liboscar.la -lQtTest_debug

buffertest_SOURCES = buffertest.cpp

serverversionstest_SOURCES = serverversionstest.cpp oscartestbase.cpp
connectiontest_SOURCES = connectiontest.cpp oscartestbase.cpp
rateinfotest_SOURCES = rateinfotest.cpp oscartestbase.cpp



