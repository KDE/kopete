INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES =  coreprotocol.moc
bin_PROGRAMS = field_test coreprotocol_test client_test
coreprotocol_test_LDFLAGS = -no-undefined $(all_libraries)
coreprotocol_test_SOURCES = coreprotocol_test.cpp
coreprotocol_test_LDADD = \
	../libgwtest.la -lqt-mt
field_test_LDFLAGS = -no-undefined $(all_libraries)
field_test_SOURCES = field_test.cpp
field_test_LDADD = \
	../libgwtest.la -lqt-mt


client_test_SOURCES = client_test.cpp
client_test_LDADD = ../../../../protocols/groupwise/kopete_groupwise.la \
	../libgwtest.la -lqt-mt
