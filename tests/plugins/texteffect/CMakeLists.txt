include_directories(
  ${KOPETE_INCLUDES}
  ${CMAKE_SOURCE_DIR}/plugins/texteffect
)

set(
  kopete_texteffect_test_PART_SRCS 
  ${CMAKE_SOURCE_DIR}/plugins/texteffect/texteffectplugin.cpp 
  ${CMAKE_SOURCE_DIR}/plugins/texteffect/texteffectconfig.cpp 
  kopetetexteffectplugintest.cpp
)

set( KOPETE_TEST_LIBRARIES
  kopete
  Qt5::Test
  KF5::KIOCore
)

ecm_add_test(${kopete_texteffect_test_PART_SRCS} TEST_NAME texteffecttest LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
