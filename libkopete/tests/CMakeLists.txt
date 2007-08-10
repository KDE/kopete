
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

set( KOPETE_TEST_LIBRARIES ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kopete )

########### Automated tests ###############

set(kopeteemoticontest_SRCS kopeteemoticontest.cpp )

kde4_automoc(${kopeteemoticontest_SRCS})

kde4_add_unit_test(kopeteemoticontest ${kopeteemoticontest_SRCS})

target_link_libraries(kopeteemoticontest  ${KOPETE_TEST_LIBRARIES} )

####
# this test uses an ugly hack which does not work on win32 
if(NOT WIN32)
        set(kopetemessage_test_SRCS kopetemessage_test.cpp ${kopete_test_mock_SRCS})
        
        kde4_automoc(${kopetemessage_test_SRCS})
        
        kde4_add_unit_test(kopetemessage_test ${kopetemessage_test_SRCS})
        
        target_link_libraries(kopetemessage_test  ${KOPETE_TEST_LIBRARIES} )
        
endif(NOT WIN32)
####

set(kopetetask_test_SRCS kopetetask_test.cpp )

kde4_automoc(${kopetetask_test_SRCS})

kde4_add_unit_test(kopetetask_test ${kopetetask_test_SRCS})

target_link_libraries(kopetetask_test  ${KOPETE_TEST_LIBRARIES} )

####

set(statusmessage_test_SRCS statusmessage_test.cpp )

kde4_automoc(${statusmessage_test_SRCS})

kde4_add_unit_test(statusmessage_test  ${statusmessage_test_SRCS})

target_link_libraries(statusmessage_test  ${KOPETE_TEST_LIBRARIES} )

####

set(xmlcontactstorage_test_SRCS xmlcontactstorage_test.cpp)

kde4_automoc(${xmlcontactstorage_test_SRCS})

kde4_add_unit_test(xmlcontactstorage_test  ${xmlcontactstorage_test_SRCS})

target_link_libraries(xmlcontactstorage_test ${KOPETE_TEST_LIBRARIES})

########### Tests Program ###############

set(avatarselector_test_SRCS avatarselectortest_program.cpp)

kde4_add_test_executable(avatarselectortest_program ${avatarselector_test_SRCS})

target_link_libraries(avatarselectortest_program ${KOPETE_TEST_LIBRARIES})

########### install files ###############





