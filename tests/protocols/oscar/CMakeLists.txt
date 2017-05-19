add_definitions("-DKDESRCDIR=\"${CMAKE_CURRENT_BINARY_DIR}\"")
find_package(Qt5 REQUIRED COMPONENTS Test)
include_directories(
  ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar
  ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/utils
  ${KOPETE_SOURCE_DIR}/protocols/oscar/liboscar/tasks
  ${KOPETE_INCLUDES}
)

set(
  LIBOSCAR_TEST_LIBRARIES
  KF5::KDELibs4Support
  oscar
  Qt5::Test
)
# Test for buffer
ecm_add_test(buffertest.cpp LINK_LIBRARIES ${LIBOSCAR_TEST_LIBRARIES})

# Test for server version tasks
set(serverversionstest_SRCS serverversionstest.cpp oscartestbase.cpp )
ecm_add_test(
  ${serverversionstest_SRCS}
  TEST_NAME serverversionstest
  LINK_LIBRARIES ${LIBOSCAR_TEST_LIBRARIES}
)

# Test for connection
set(connectiontest_SRCS connectiontest.cpp oscartestbase.cpp )
ecm_add_test(
  ${connectiontest_SRCS}
  TEST_NAME connectiontest
  LINK_LIBRARIES ${LIBOSCAR_TEST_LIBRARIES}
)

# Tests for File Transfer
set(filetransfertest_SRCS filetransfertest.cpp oscartestbase.cpp )
ecm_add_test(
  ${filetransfertest_SRCS}
  TEST_NAME filetransfertest
  LINK_LIBRARIES ${LIBOSCAR_TEST_LIBRARIES}
)

#test for oscar guid
ecm_add_test(guidtest.cpp LINK_LIBRARIES ${LIBOSCAR_TEST_LIBRARIES})
