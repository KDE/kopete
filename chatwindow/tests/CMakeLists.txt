set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


set( CHATWINDOW_TEST_LIBRARIES ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kopetechatwindow  )

########### next target ###############
set(chatwindowstylerendering_test_SRCS chatwindowstylerendering_test.cpp )

kde4_add_executable(chatwindowstylerendering_test TEST ${chatwindowstylerendering_test_SRCS})

target_link_libraries(chatwindowstylerendering_test  ${CHATWINDOW_TEST_LIBRARIES})

