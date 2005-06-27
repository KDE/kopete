INCLUDES = $(all_includes)

check_PROGRAMS = kunittest

METASOURCES = AUTO

kunittest_SOURCES = main.cpp kunittest.cpp chatnavtests.cpp
kunittest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kunittest_LDADD = $(LIB_KDECORE) ../liboscar.la

check: kunittest
	@./kunittest 2>&1 | grep "tests:"
