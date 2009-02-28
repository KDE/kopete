
include_directories(
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/utils
    ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/tasks
   ${KOPETE_INCLUDES}
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )

set(LIBOSCAR_TEST_LIBRARIES ${KDE4_KDECORE_LIBS} oscar ${QT_QTTEST_LIBRARY})

########### next target ###############

set(buffertest_SRCS buffertest.cpp )

kde4_add_unit_test(buffertest  ${buffertest_SRCS})

target_link_libraries(buffertest ${LIBOSCAR_TEST_LIBRARIES} )



########### next target ###############

set(serverversionstest_SRCS serverversionstest.cpp oscartestbase.cpp )


kde4_add_unit_test(serverversionstest  ${serverversionstest_SRCS})

target_link_libraries(serverversionstest ${LIBOSCAR_TEST_LIBRARIES} )



########### next target ###############

set(connectiontest_SRCS connectiontest.cpp oscartestbase.cpp )


kde4_add_unit_test(connectiontest  ${connectiontest_SRCS})

target_link_libraries(connectiontest ${LIBOSCAR_TEST_LIBRARIES})



########### next target ###############

set(rateinfotest_SRCS rateinfotest.cpp oscartestbase.cpp )


kde4_add_unit_test(rateinfotest  ${rateinfotest_SRCS})

target_link_libraries(rateinfotest ${LIBOSCAR_TEST_LIBRARIES})



########### next target ###############

set(filetransfertest_SRCS filetransfertest.cpp oscartestbase.cpp )


kde4_add_unit_test(filetransfertest  ${filetransfertest_SRCS})

target_link_libraries(filetransfertest ${LIBOSCAR_TEST_LIBRARIES} )




########################################


