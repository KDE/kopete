INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
		   -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise \
		   -I$(top_srcdir)/kopete/protocols/groupwise \
		   $(all_includes) 
METASOURCES = AUTO
noinst_PROGRAMS = clientstream_test
clientstream_test_SOURCES = clientstream_test.cpp
clientstream_test_LDADD = -lqt-mt \
	../../../protocols/groupwise/libgroupwise/libgwtest.la \
	../../../protocols/groupwise/kopete_groupwise.la
