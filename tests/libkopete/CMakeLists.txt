include_directories(
  ${KOPETE_INCLUDES}
  ${CMAKE_SOURCE_DIR}/libkopete
)

set(BLACKLIST_LIB 
  kopeteblacklistertest.cpp
  ${CMAKE_SOURCE_DIR}/libkopete/kopeteblacklister.cpp
)

set( KOPETE_TEST_LIBRARIES
  kopete
  Qt5::Test
  KF5::KIOCore
)
ecm_add_test(kopetetasktest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetestatusmessagetest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(${BLACKLIST_LIB} TEST_NAME blacklisttest LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetecontactlistelementtest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopeteglobaltest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopeteidentitytest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopeteonlinestatusmanagertest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopeteinfoeventtest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetecontacttest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetemessagetest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopeteaccounttest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
ecm_add_test(kopetemetacontacttest.cpp LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})

