INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src -I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src $(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = task_take_test

task_take_test_LDADD = -I$(srcdir)/../../libgroupwise/tasks -lqt-mt \
	../../libgwtest.la

task_take_test_SOURCES = task_take_test.cpp
