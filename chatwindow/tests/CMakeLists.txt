ENABLE_TESTING()

ADD_DEFINITIONS(-DSRCDIR ${CMAKE_CURRENT_SOURCE_DIR})

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( CHATWINDOW_TEST_LIBRARIES ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kopete_chatwindow  )


include_directories( 
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/../ 
${CMAKE_CURRENT_SOURCE_DIR}/../../ 
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )
########### next target ###############
set(chatwindowstylerendering_test_SRCS chatwindowstylerendering_test.cpp )

kde4_add_unit_test(chatwindowstylerendering_test TESTNAME kopete-chatwindowstylerenderin_test ${chatwindowstylerendering_test_SRCS})

target_link_libraries(chatwindowstylerendering_test  ${CHATWINDOW_TEST_LIBRARIES})

########### next target ###############
