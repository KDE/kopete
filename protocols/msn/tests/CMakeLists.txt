set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )




########### next target ###############

set(challengetest_SRCS challengetest.cpp )

kde4_add_executable(challengetest TEST ${challengetest_SRCS})

target_link_libraries(challengetest  ${KDE4_KDECORE_LIBS} )

########### install files ###############





