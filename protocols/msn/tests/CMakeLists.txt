



########### next target ###############

set(challengetest_SRCS challengetest.cpp )

kde4_add_test_executable(challengetest ${challengetest_SRCS})

target_link_libraries(challengetest  ${KDE4_KDECORE_LIBS} )

########### install files ###############





