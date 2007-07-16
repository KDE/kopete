

include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/yahoo/libkyahoo  )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )

########### next target ###############

set(clientstream_test_SRCS clientstream_test.cpp )

kde4_add_test_executable(clientstream_test  ${clientstream_test_SRCS})

target_link_libraries(clientstream_test  ${KDE4_KDECORE_LIBS} kyahoo )





