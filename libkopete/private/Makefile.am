METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkopeteprivate.la

libkopeteprivate_la_SOURCES = kopeteemoticons.cpp \
	kopetecommand.cpp kopeteviewmanager.cpp kopeteutils_private.cpp
libkopeteprivate_la_LDFLAGS = $(all_libraries)
libkopeteprivate_la_LIBADD = $(LIB_KDEUI)
# vim: set noet:

