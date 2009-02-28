
include_directories( ${KOPETE_SOURCE_DIR}/protocols/groupwise/libgroupwise/qca/src  )


########### next target ###############

set(task_take_test_SRCS task_take_test.cpp )

kde4_add_executable(task_take_test ${task_take_test_SRCS})

target_link_libraries(task_take_test  ${KDE4_KDECORE_LIBS} qt-mt gwtest )


########### install files ###############





