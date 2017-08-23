include_directories(
    ${KOPETE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libkopete/ui
)

link_directories(
    ${KOPETE_BINARY_DIR}/kopete
)

set(
  KOPETE_TEST_LIBRARIES
  KF5::Contacts
  KF5::TextEditor
  KF5::NotifyConfig
  KF5::Crash
  KF5::DBusAddons
  ${QIMAGEBLITZ_LIBRARIES}
  kopete
  kopeteaddaccountwizard
  kopetestatusmenu
  kopetecontactlist
  kopeteidentity
  Qt5::Test
)

set(
  LIBKOPETE_ADDRESSBOOKLINK_GUI_TEST_SRCS 
  kopeteaddressbooklinkwidgettest.cpp
)

ki18n_wrap_ui(LIBKOPETE_ADDRESSBOOKLINK_GUI_TEST_SRCS ${CMAKE_SOURCE_DIR}/libkopete/ui/addressbooklinkwidget_base.ui)

ecm_add_test(${LIBKOPETE_ADDRESSBOOKLINK_GUI_TEST_SRCS} TEST_NAME libkopete_addressbooklink_gui_test LINK_LIBRARIES ${KOPETE_TEST_LIBRARIES})
