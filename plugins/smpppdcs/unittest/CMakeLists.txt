
include_directories( ${CMAKE_SOURCE_DIR}/src  )


########### next target ###############

set(smpppdcstests_SRCS main.cpp clienttest.cpp )

kde4_add_test_executable(smpppdcstests ${smpppdcstests_SRCS})

target_link_libraries(smpppdcstests  ${KDE4_KDECORE_LIBS} smpppdclient kunittestgui )

########### install files ###############





