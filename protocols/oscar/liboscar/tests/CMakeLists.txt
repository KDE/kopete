
include_directories(
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/utils
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/tasks
   ${KOPETE_INCLUDES}
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )

set(LIBOSCAR_TEST_LIBRARIES KF5::KDELibs4Support oscar Qt5::Test)

########### next target ###############

set(buffertest_SRCS buffertest.cpp )

add_executable(buffertest  ${buffertest_SRCS})
add_test(buffertest buffertest)
ecm_mark_as_test(buffertest)

target_link_libraries(buffertest ${LIBOSCAR_TEST_LIBRARIES} )



########### next target ###############

set(serverversionstest_SRCS serverversionstest.cpp oscartestbase.cpp )


add_executable(serverversionstest  ${serverversionstest_SRCS})
add_test(serverversionstest serverversionstest)
ecm_mark_as_test(serverversionstest)

target_link_libraries(serverversionstest ${LIBOSCAR_TEST_LIBRARIES} )



########### next target ###############

set(connectiontest_SRCS connectiontest.cpp oscartestbase.cpp )


add_executable(connectiontest  ${connectiontest_SRCS})
add_test(connectiontest connectiontest)
ecm_mark_as_test(connectiontest)

target_link_libraries(connectiontest ${LIBOSCAR_TEST_LIBRARIES})



########### next target ###############

set(rateinfotest_SRCS rateinfotest.cpp oscartestbase.cpp )


add_executable(rateinfotest  ${rateinfotest_SRCS})
add_test(rateinfotest rateinfotest)
ecm_mark_as_test(rateinfotest)

target_link_libraries(rateinfotest ${LIBOSCAR_TEST_LIBRARIES})



########### next target ###############

set(filetransfertest_SRCS filetransfertest.cpp oscartestbase.cpp )


add_executable(filetransfertest  ${filetransfertest_SRCS})
add_test(filetransfertest filetransfertest)
ecm_mark_as_test(filetransfertest)

target_link_libraries(filetransfertest ${LIBOSCAR_TEST_LIBRARIES} )




########################################


