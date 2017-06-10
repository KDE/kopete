include_directories(
  ${KOPETE_INCLUDES}
)

set( KOPETE_TEST_LIBRARIES
  Qt5::Test
  KF5::KIOCore
  kopete
)
ecm_add_test(kopetetasktest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetestatusmessagetest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
