INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES = coreprotocol.moc
bin_PROGRAMS = coreprotocol_test
coreprotocol_test_LDFLAGS = -no-undefined $(all_libraries)
coreprotocol_test_SOURCES = coreprotocol_test.cpp
coreprotocol_test_LDADD = \
	../../../../protocols/groupwise/libgroupwise/libgwtest.la -lqt-mt

