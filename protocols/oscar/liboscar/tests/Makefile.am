INCLUDES = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/oscar/liboscar $(all_includes)
METASOURCES = AUTO
check_PROGRAMS = buffertest serverversionstest connectiontest rateinfotest \
		 offlinemessagestest aimlogintasktest guidtest
TESTS = buffertest connectiontest guidtest

AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

LDADD = ../liboscar.la -lQtTest_debug

buffertest_SOURCES = buffertest.cpp
guidtest_SOURCES = guidtest.cpp

serverversionstest_SOURCES = serverversionstest.cpp oscartestbase.cpp
connectiontest_SOURCES = connectiontest.cpp oscartestbase.cpp
rateinfotest_SOURCES = rateinfotest.cpp oscartestbase.cpp
offlinemessagestest_SOURCES = offlinemessagestest.cpp oscartestbase.cpp
aimlogintasktest_SOURCES = aimlogintasktest.cpp oscartestbase.cpp

check-local: serverversionstest rateinfotest offlinemessagestest aimlogintasktest
	./serverversionstest $(srcdir)
	./rateinfotest $(srcdir)
	./offlinemessagestest $(srcdir)
	./aimlogintasktest $(srcdir)
	
