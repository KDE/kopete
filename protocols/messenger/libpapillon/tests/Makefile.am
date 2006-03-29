 AM_CPPFLAGS = -DQT_NO_ASCII_CAST -DQT_NO_COMPAT $(all_includes) -I./..

METASOURCES = AUTO

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS)
LDADD = -lQtTest_debug ../libpapillon.la 

noinst_PROGRAMS = connector_test connection_test papillon_console

check_PROGRAMS = transfer_test coreprotocol_test
TESTS = transfer_test coreprotocol_test

# Automated tests
transfer_test_SOURCES = transfer_test.cpp
coreprotocol_test_SOURCES = coreprotocol_test.cpp

# Console test programs
connection_test_SOURCES = connection_test.cpp
connector_test_SOURCES = connector_test.cpp

# The Papillon GUI console.
papillon_console_SOURCES = papillon_console.cpp
papillon_console_LDADD = ${LIB_QTGUI} ../libpapillon.la 
