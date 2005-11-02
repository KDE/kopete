INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src \
	-I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
	$(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES = libgroupwise_tasks.la

libgroupwise_tasks_la_SOURCES = requesttask.cpp eventtask.cpp logintask.cpp \
		setstatustask.cpp statustask.cpp conferencetask.cpp createconferencetask.cpp \
		sendmessagetask.cpp getdetailstask.cpp getstatustask.cpp typingtask.cpp connectiontask.cpp \
		sendinvitetask.cpp joinconferencetask.cpp leaveconferencetask.cpp rejectinvitetask.cpp \
		keepalivetask.cpp createcontacttask.cpp modifycontactlisttask.cpp createfoldertask.cpp \
		movecontacttask.cpp updateitemtask.cpp createcontactinstancetask.cpp deleteitemtask.cpp \
		updatefoldertask.cpp updatecontacttask.cpp pollsearchresultstask.cpp privacyitemtask.cpp \
		needfoldertask.cpp searchchattask.cpp searchusertask.cpp searchusertask.h \
		getchatsearchresultstask.cpp chatcountstask.cpp chatpropertiestask.cpp joinchattask.cpp
noinst_HEADERS = requesttask.h eventtask.h logintask.h setstatustask.h \
		statustask.h conferencetask.h createconferencetask.h sendmessagetask.h \
		getdetailstask.h getstatustask.h typingtask.h connectiontask.h sendinvitetask.h \
		joinconferencetask.h leaveconferencetask.h rejectinvitetask.h createcontacttask.h \
		modifycontactlisttask.h createfoldertask.h movecontacttask.h updateitemtask.h deleteitemtask.h \
		updatefoldertask.h updatecontacttask.h pollsearchresultstask.h privacyitemtask.h \
		needfoldertask.h searchchattask.h getchatsearchresultstask.h searchusertask.h \
		chatcountstask.h joinchattask.h


libgroupwise_tasks_la_LDFLAGS = -no-undefined $(all_libraries)
libgroupwise_tasks_la_LIBADD = $(LIB_QT)



