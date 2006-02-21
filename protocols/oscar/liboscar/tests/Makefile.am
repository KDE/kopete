INCLUDES = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/oscar/liboscar $(all_includes)
METASOURCES = AUTO
check_PROGRAMS = buffertest serverversionstest
TESTS = buffertest serverversionstest
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

LDADD = ../liboscar.la -lQtTest_debug

buffertest_SOURCES = buffertest.cpp
serverversionstest_SOURCES = serverversionstest.cpp oscartestbase.cpp


