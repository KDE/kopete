include_directories(
    ${KOPETE_INCLUDES}
)

link_directories(
    ${KOPETE_BINARY_DIR}/libkopete
)

set(
  kopete_libraries
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
