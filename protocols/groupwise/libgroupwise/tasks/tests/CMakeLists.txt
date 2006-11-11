
include_directories( ${CMAKE_SOURCE_DIR}/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src  )


########### next target ###############

set(task_take_test_SRCS task_take_test.cpp )

kde4_automoc(${task_take_test_SRCS})

kde4_add_executable(task_take_test ${task_take_test_SRCS})

target_link_libraries(task_take_test  ${KDE4_KDECORE_LIBS} qt-mt gwtest )


########### install files ###############





