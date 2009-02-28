set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


include_directories( ${KOPETE_SOURCE_DIR}/protocols/yahoo/libkyahoo )

link_directories( ${KOPETE_BINARY_DIR}/protocols/yahoo/libkyahoo )

########### next target ###############

set(clientstream_test_SRCS clientstream_test.cpp )

kde4_add_executable(clientstream_test TEST  ${clientstream_test_SRCS})

target_link_libraries(clientstream_test ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kyahoo )
