METASOURCES = AUTO

INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libwrappers.la

libwrappers_la_SOURCES = bindingobject.cpp kopetemessage_imp.cpp \
					kopeteonlinestatus_imp.cpp

libwrappers_la_LDFLAGS     = $(all_libraries)
libwrappers_la_LIBADD      =  -lkjsembed ../../../libkopete/libkopete.la


