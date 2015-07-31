set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


include_directories( ${KOPETE_SOURCE_DIR}/protocols/yahoo/libkyahoo )

link_directories( ${KOPETE_BINARY_DIR}/protocols/yahoo/libkyahoo )

########### next target ###############

set(clientstream_test_SRCS clientstream_test.cpp )

add_executable(clientstream_test  ${clientstream_test_SRCS})
ecm_mark_as_test(clientstream_test)

target_link_libraries(clientstream_test Qt5::Gui KF5::KDELibs4Support kyahoo )
