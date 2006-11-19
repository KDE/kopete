



########### next target ###############

set(challengetest_SRCS challengetest.cpp )

kde4_automoc(${challengetest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(challengetest ${challengetest_SRCS})

target_link_libraries(challengetest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





