INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise \
	-I$(top_srcdir)/kopete/protocols/groupwise \
	$(all_includes)
METASOURCES = AUTO
noinst_PROGRAMS = clientstream_test field_test coreprotocol_test client_test
coreprotocol_test_LDFLAGS = -no-undefined $(all_libraries)
coreprotocol_test_SOURCES = coreprotocol_test.cpp
coreprotocol_test_LDADD = \
	../libgwtest.la -lqt-mt
field_test_LDFLAGS = -no-undefined $(all_libraries)
field_test_SOURCES = field_test.cpp
field_test_LDADD = \
	../libgwtest.la -lqt-mt

clientstream_test_SOURCES = clientstream_test.cpp
clientstream_test_LDADD = -lqt-mt \
	../../kopete_groupwise.la

client_test_SOURCES = client_test.cpp
client_test_LDADD = ../../../../protocols/groupwise/kopete_groupwise.la \
	../libgwtest.la -lqt-mt
