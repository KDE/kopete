
if(KDE4_BUILD_TESTS)

set(kopete_test_mock_SRCS
mock/kopeteaccount_mock.cpp
mock/kopetecontact_mock.cpp
mock/kopetemessage_mock.cpp
mock/kopetemetacontact_mock.cpp
mock/kopeteprotocol_mock.cpp
)

#add_subdirectory( mock )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/mock/ ${KOPETE_INCLUDES} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions( -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"" )

set( KOPETE_TEST_LIBRARIES ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kopete )

########### Automated tests ###############

set(kopeteemoticontest_SRCS kopeteemoticontest.cpp )

kde4_automoc(${kopeteemoticontest_SRCS})

kde4_add_executable(kopeteemoticontest NOGUI RUN_UNINSTALLED ${kopeteemoticontest_SRCS})

target_link_libraries(kopeteemoticontest  ${KOPETE_TEST_LIBRARIES} )

add_test(kopeteemoticontest ${EXECUTABLE_OUTPUT_PATH}/kopeteemoticontest)

####

set(kopetemessage_test_SRCS kopetemessage_test.cpp ${kopete_test_mock_SRCS})

kde4_automoc(${kopetemessage_test_SRCS})

kde4_add_executable(kopetemessage_test NOGUI RUN_UNINSTALLED ${kopetemessage_test_SRCS})

target_link_libraries(kopetemessage_test  ${KOPETE_TEST_LIBRARIES} )

add_test(kopetemessage_test ${EXECUTABLE_OUTPUT_PATH}/kopetemessage_test)

####

set(kopetetask_test_SRCS kopetetask_test.cpp )

kde4_automoc(${kopetetask_test_SRCS})

kde4_add_executable(kopetetask_test NOGUI RUN_UNINSTALLED ${kopetetask_test_SRCS})

target_link_libraries(kopetetask_test  ${KOPETE_TEST_LIBRARIES} )

add_test(kopetetask_test ${EXECUTABLE_OUTPUT_PATH}/kopetetask_test)

####

set(statusmessage_test_SRCS statusmessage_test.cpp )

kde4_automoc(${statusmessage_test_SRCS})

kde4_add_executable(statusmessage_test NOGUI RUN_UNINSTALLED ${statusmessage_test_SRCS})

target_link_libraries(statusmessage_test  ${KOPETE_TEST_LIBRARIES} )

add_test(statusmessage_test ${EXECUTABLE_OUTPUT_PATH}/statusmessage_test)

####

set(xmlcontactstorage_test_SRCS xmlcontactstorage_test.cpp)

kde4_automoc(${xmlcontactstorage_test_SRCS})

kde4_add_executable(xmlcontactstorage_test NOGUI RUN_UNINSTALLED ${xmlcontactstorage_test_SRCS})

target_link_libraries(xmlcontactstorage_test ${KOPETE_TEST_LIBRARIES})

add_test(xmlcontactstorage_test ${EXECUTABLE_OUTPUT_PATH}/xmlcontactstorage_test)

endif(KDE4_BUILD_TESTS)

########### install files ###############





