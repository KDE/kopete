METASOURCES = AUTO
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopete_mock.la

libkopete_mock_la_SOURCES = kopetemessage_mock.cpp kopeteaccount_mock.cpp kopetecontact_mock.cpp kopetemetacontact_mock.cpp kopeteprotocol_mock.cpp

libkopete_mock_la_LDFLAGS = $(all_libraries) -lkabc
libkopete_mock_la_LIBADD = ../../libkopete.la ../../private/libkopeteprivate.la $(LIB_KHTML)

noinst_HEADERS = kopetemessage_mock.h kopetecontact_mock.h kopetemetacontact_mock.h kopeteaccount_mock.h kopeteprotocol_mock.h


