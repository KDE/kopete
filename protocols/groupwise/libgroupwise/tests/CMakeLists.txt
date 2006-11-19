
include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise  )


########### next target ###############

set(clientstream_test_SRCS clientstream_test.cpp )

kde4_automoc(${clientstream_test_SRCS})

kde4_add_executable(clientstream_test ${clientstream_test_SRCS})

target_link_libraries(clientstream_test  ${KDE4_KDECORE_LIBS} qt-mt )


########### next target ###############

set(field_test_SRCS field_test.cpp )

kde4_automoc(${field_test_SRCS})

kde4_add_executable(field_test ${field_test_SRCS})

target_link_libraries(field_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### next target ###############

set(coreprotocol_test_SRCS coreprotocol_test.cpp )

kde4_automoc(${coreprotocol_test_SRCS})

kde4_add_executable(coreprotocol_test ${coreprotocol_test_SRCS})

target_link_libraries(coreprotocol_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### next target ###############

set(client_test_SRCS client_test.cpp )

kde4_automoc(${client_test_SRCS})

kde4_add_executable(client_test ${client_test_SRCS})

target_link_libraries(client_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### install files ###############





