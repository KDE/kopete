INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES = libgroupwise_tasks.la

libgroupwise_tasks_la_LDFLAGS = $(all_libraries)
libgroupwise_tasks_la_SOURCES = requesttask.cpp eventtask.cpp logintask.cpp \
		setstatustask.cpp statustask.cpp conferencetask.cpp createconferencetask.cpp \
		sendmessagetask.cpp getdetailstask.cpp getstatustask.cpp typingtask.cpp 	connectiontask.cpp \
	sendinvitetask.cpp joinconferencetask.cpp leaveconferencetask.cpp rejectinvitetask.cpp \
	keepalivetask.cpp createcontacttask.cpp modifycontactlisttask.cpp createfoldertask.cpp \
	deletecontacttask.cpp movecontacttask.cpp
noinst_HEADERS = requesttask.h eventtask.h logintask.h setstatustask.h \
		statustask.h conferencetask.h createconferencetask.h sendmessagetask.h \
		getdetailstask.h getstatustask.h typingtask.h connectiontask.h sendinvitetask.h \
	joinconferencetask.h leaveconferencetask.h rejectinvitetask.h createcontacttask.h \
	modifycontactlisttask.h createfoldertask.h deletecontacttask.h movecontacttask.h
SUBDIRS = tests
