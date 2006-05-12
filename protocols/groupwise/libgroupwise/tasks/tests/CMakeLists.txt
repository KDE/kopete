
include_directories( ${CMAKE_SOURCE_DIR}/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src  )


########### next target ###############

set(task_take_test_SRCS task_take_test.cpp )

kde4_automoc(${task_take_test_SRCS})

kde4_add_executable(task_take_test ${task_take_test_SRCS})

target_link_libraries(task_take_test  ${KDE4_KDECORE_LIBS} qt-mt gwtest )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/protocols/groupwise/libgroupwise/qca/src -I$(srcdir)/../../libgroupwise/ -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src $(all_includes)
#METASOURCES = AUTO
#noinst_PROGRAMS = task_take_test
#
#task_take_test_LDADD = -lqt-mt ../../libgwtest.la
#
#task_take_test_SOURCES = task_take_test.cpp
