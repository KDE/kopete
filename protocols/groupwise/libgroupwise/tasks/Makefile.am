INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libgroupwise_tasks.la
libgroupwise_tasks_la_LDFLAGS = $(all_libraries)
libgroupwise_tasks_la_SOURCES = gwtasklogin.cpp requesttask.cpp eventtask.cpp \
	logintask.cpp
noinst_HEADERS = gwtasklogin.h requesttask.h eventtask.h logintask.h
SUBDIRS = tests
