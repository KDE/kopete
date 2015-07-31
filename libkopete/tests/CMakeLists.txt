set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(kopete_test_mock_SRCS
mock/kopeteaccount_mock.cpp
mock/kopetecontact_mock.cpp
mock/kopetemessage_mock.cpp
mock/kopetemetacontact_mock.cpp
mock/kopeteprotocol_mock.cpp
)

#add_subdirectory( mock )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/mock/ ${KOPETE_INCLUDES} )

add_definitions( -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"" )

set( KOPETE_TEST_LIBRARIES Qt5::Test KF5::KIOCore kopete )

########### Automated tests ###############

# this test uses an ugly hack which does not work on win32 
if(NOT WIN32)
        set(kopetemessage_test_SRCS kopetemessage_test.cpp ${kopete_test_mock_SRCS})
        
                
        add_executable(kopetemessage_test ${kopetemessage_test_SRCS})
add_test(kopetemessage_test kopetemessage_test)
ecm_mark_as_test(kopetemessage_test)
        
        target_link_libraries(kopetemessage_test  ${KOPETE_TEST_LIBRARIES} )
        
endif(NOT WIN32)
####

set(kopetetask_test_SRCS kopetetask_test.cpp )

add_executable(kopetetask_test ${kopetetask_test_SRCS})
add_test(kopetetask_test kopetetask_test)
ecm_mark_as_test(kopetetask_test)

target_link_libraries(kopetetask_test  ${KOPETE_TEST_LIBRARIES} )

####

set(statusmessage_test_SRCS statusmessage_test.cpp )


add_executable(statusmessage_test  ${statusmessage_test_SRCS})
add_test(statusmessage_test statusmessage_test)
ecm_mark_as_test(statusmessage_test)

target_link_libraries(statusmessage_test  ${KOPETE_TEST_LIBRARIES} )

####

set(xmlcontactstorage_test_SRCS xmlcontactstorage_test.cpp)


add_executable(xmlcontactstorage_test  ${xmlcontactstorage_test_SRCS})
add_test(xmlcontactstorage_test xmlcontactstorage_test)
ecm_mark_as_test(xmlcontactstorage_test)

target_link_libraries(xmlcontactstorage_test ${KOPETE_TEST_LIBRARIES})

########### Tests Program ###############

set(avatarselector_test_SRCS avatarselectortest_program.cpp)

add_executable(avatarselectortest_program ${avatarselector_test_SRCS})
ecm_mark_as_test(avatarselectortest_program)

target_link_libraries(avatarselectortest_program ${KOPETE_TEST_LIBRARIES})

########### install files ###############





