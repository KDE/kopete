INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES = libgroupwise_tasks.la

libgroupwise_tasks_la_LDFLAGS = $(all_libraries)
libgroupwise_tasks_la_SOURCES = gwtasklogin.cpp requesttask.cpp eventtask.cpp \
		logintask.cpp setstatustask.cpp
noinst_HEADERS = gwtasklogin.h requesttask.h eventtask.h logintask.h \
	setstatustask.h
SUBDIRS = tests
