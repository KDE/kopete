include_directories(
  ${KOPETE_INCLUDES}
  ${CMAKE_SOURCE_DIR}/plugins/autoreplace
)

set(
  kopete_autoreplace_test_PART_SRCS 
  ${CMAKE_SOURCE_DIR}/plugins/autoreplace/autoreplaceplugin.cpp 
  ${CMAKE_SOURCE_DIR}/plugins/autoreplace/autoreplaceconfig.cpp 
  kopeteautoreplaceplugintest.cpp
)

set( KOPETE_TEST_LIBRARIES
  kopete
  Qt5::Test
  KF5::KIOCore
)

ecm_add_test(${kopete_autoreplace_test_PART_SRCS} TEST_NAME autoreplacetest LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})

