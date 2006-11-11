
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/oscar/liboscar ${KOPETE_INCLUDES} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )

set(LIBOSCAR_TEST_LIBRARIES ${KDE4_KDECORE_LIBS} oscar ${QT_QTTEST_LIBRARY})

########### next target ###############

set(buffertest_SRCS buffertest.cpp )

kde4_automoc(${buffertest_SRCS})

kde4_add_executable(buffertest RUN_UNINSTALLED ${buffertest_SRCS})

target_link_libraries(buffertest ${LIBOSCAR_TEST_LIBRARIES} )

add_test(buffertest ${EXECUTABLE_OUTPUT_PATH}/buffertest)

########### next target ###############

set(serverversionstest_SRCS serverversionstest.cpp oscartestbase.cpp )

kde4_automoc(${serverversionstest_SRCS})

kde4_add_executable(serverversionstest RUN_UNINSTALLED ${serverversionstest_SRCS})

target_link_libraries(serverversionstest ${LIBOSCAR_TEST_LIBRARIES} )

add_test(serverversionstest ${EXECUTABLE_OUTPUT_PATH}/serverversionstest)

########### next target ###############

set(connectiontest_SRCS connectiontest.cpp oscartestbase.cpp )

kde4_automoc(${connectiontest_SRCS})

kde4_add_executable(connectiontest RUN_UNINSTALLED ${connectiontest_SRCS})

target_link_libraries(connectiontest ${LIBOSCAR_TEST_LIBRARIES})

add_test(connectiontest ${EXECUTABLE_OUTPUT_PATH}/connectiontest)

########### next target ###############

set(rateinfotest_SRCS rateinfotest.cpp oscartestbase.cpp )

kde4_automoc(${rateinfotest_SRCS})

kde4_add_executable(rateinfotest RUN_UNINSTALLED ${rateinfotest_SRCS})

target_link_libraries(rateinfotest ${LIBOSCAR_TEST_LIBRARIES})

add_test(rateinfotest ${EXECUTABLE_OUTPUT_PATH}/rateinfotest)

########### next target ###############

set(offlinemessagestest_SRCS offlinemessagestest.cpp oscartestbase.cpp )

kde4_automoc(${offlinemessagestest_SRCS})

kde4_add_executable(offlinemessagestest RUN_UNINSTALLED ${offlinemessagestest_SRCS})

target_link_libraries(offlinemessagestest ${LIBOSCAR_TEST_LIBRARIES})

add_test(offlinemessagestest ${EXECUTABLE_OUTPUT_PATH}/offlinemessagestest)

########### next target ###############

set(filetransfertest_SRCS filetransfertest.cpp oscartestbase.cpp )

kde4_automoc(${filetransfertest_SRCS})

kde4_add_executable(filetransfertest RUN_UNINSTALLED ${filetransfertest_SRCS})

target_link_libraries(filetransfertest ${LIBOSCAR_TEST_LIBRARIES} )

add_test(filetransfertest ${EXECUTABLE_OUTPUT_PATH}/filetransfertest)


########################################
endif(KDE4_BUILD_TESTS)


