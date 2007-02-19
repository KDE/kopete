
if(KDE4_BUILD_TESTS)

include_directories( ${PAPILLON_INCLUDES} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( PAPILLON_TEST_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTTEST_LIBRARY} papillon_kopete )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../ )

add_definitions(-DPAPILLON_TESTS_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")

########### Tests programs ###############

set(connection_test_SRCS connection_test.cpp )

kde4_automoc(${connection_test_SRCS})

kde4_add_executable(connection_test NOGUI RUN_UNINSTALLED ${connection_test_SRCS})

target_link_libraries(connection_test ${PAPILLON_TEST_LIBRARIES} )

####
set(connector_test_SRCS connector_test.cpp)

kde4_automoc(${connector_test_SRCS})

kde4_add_executable(connector_test NOGUI RUN_UNINSTALLED ${connector_test_SRCS})

target_link_libraries(connector_test ${PAPILLON_TEST_LIBRARIES} )

####
# this test uses an ugly hack which does not work on win32 
if(NOT WIN32)
        set(papillon_console_SRCS papillon_console.cpp)
        
        kde4_automoc(${papillon_console_SRCS})
        
        kde4_add_executable(papillon_console NOGUI RUN_UNINSTALLED ${papillon_console_SRCS})
        
        target_link_libraries(papillon_console ${QT_QTGUI_LIBRARY} ${PAPILLON_TEST_LIBRARIES})
endif(NOT WIN32)

########### Automated tests ###############

set(transfer_test_SRCS transfer_test.cpp )

kde4_automoc(${transfer_test_SRCS})

kde4_add_executable(transfer_test NOGUI RUN_UNINSTALLED ${transfer_test_SRCS})

target_link_libraries(transfer_test ${PAPILLON_TEST_LIBRARIES} )

add_test(transfer_test ${EXECUTABLE_OUTPUT_PATH}/transfer_test)

#####

set( coreprotocol_test_SRCS coreprotocol_test.cpp )

kde4_automoc(${coreprotocol_test_SRCS} )

kde4_add_executable(coreprotocol_test NOGUI RUN_UNINSTALLED ${coreprotocol_test_SRCS})

target_link_libraries(coreprotocol_test ${PAPILLON_TEST_LIBRARIES})

add_test(coreprotocol_test ${EXECUTABLE_OUTPUT_PATH}/coreprotocol_test)

#####

set( mimeheader_test_SRCS mimeheader_test.cpp )

kde4_automoc(${mimeheader_test_SRCS} )

kde4_add_executable(mimeheader_test NOGUI RUN_UNINSTALLED ${mimeheader_test_SRCS})

target_link_libraries(mimeheader_test ${PAPILLON_TEST_LIBRARIES})

add_test(mimeheader_test ${EXECUTABLE_OUTPUT_PATH}/mimeheader_test)


#####

set( httptransfer_test_SRCS httptransfer_test.cpp )

kde4_automoc(${httptransfer_test_SRCS})

kde4_add_executable(httptransfer_test NOGUI RUN_UNINSTALLED ${httptransfer_test_SRCS})

target_link_libraries(httptransfer_test ${PAPILLON_TEST_LIBRARIES})

add_test(httptransfer_test ${EXECUTABLE_OUTPUT_PATH}/httptransfer_test)

######

set( httpcoreprotocol_test_SRCS httpcoreprotocol_test.cpp )

kde4_automoc(${httpcoreprotocol_test_SRCS})

kde4_add_executable(httpcoreprotocol_test NOGUI RUN_UNINSTALLED ${httpcoreprotocol_test_SRCS})

target_link_libraries(httpcoreprotocol_test ${PAPILLON_TEST_LIBRARIES})

add_test(httpcoreprotocol_test ${EXECUTABLE_OUTPUT_PATH}/httpcoreprotocol_test)

######

set( statusmessage_test_SRCS statusmessage_test.cpp )

kde4_automoc(${statusmessage_test_SRCS})

kde4_add_executable(statusmessage_test NOGUI RUN_UNINSTALLED ${statusmessage_test_SRCS})

target_link_libraries(statusmessage_test ${PAPILLON_TEST_LIBRARIES})

add_test(statusmessage_test ${EXECUTABLE_OUTPUT_PATH}/statusmessage_test)

######
# this test uses an ugly hack which does not work on win32 
if(NOT WIN32)
        set( challenge_test_SRCS challengetest.cpp )
        
        kde4_automoc(${challenge_test_SRCS})
        
        kde4_add_executable(challenge_test NOGUI RUN_UNINSTALLED ${challenge_test_SRCS})
        
        target_link_libraries(challenge_test ${PAPILLON_TEST_LIBRARIES} ${QCA2_LIBRARIES})
        
        add_test(challenge_test ${EXECUTABLE_OUTPUT_PATH}/challenge_test)
endif(NOT WIN32)
### ENDIF ###
endif(KDE4_BUILD_TESTS)

#############

