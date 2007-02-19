
add_subdirectory( liboscar )
add_subdirectory( aim )
add_subdirectory( icq )
add_subdirectory( icons )

include_directories(
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/liboscar/
${CMAKE_CURRENT_SOURCE_DIR}/liboscar/utils
${CMAKE_CURRENT_SOURCE_DIR}/liboscar/tasks
)


########### next target ###############

set(kopete_oscar_LIB_SRCS
   oscarprotocol.cpp
   oscaraccount.cpp
   oscarcontact.cpp
   oscarmyselfcontact.cpp
   oscarencodingselectiondialog.cpp
   oscarlistnonservercontacts.cpp
   oscarversionupdater.cpp
   oscarprivacyengine.cpp
   aimcontactbase.cpp
   icqcontactbase.cpp )

kde4_add_ui_files(kopete_oscar_LIB_SRCS
   oscarencodingselectionbase.ui
   oscarlistcontactsbase.ui
)

kde4_automoc(${kopete_oscar_LIB_SRCS})

kde4_add_library(kopete_oscar SHARED ${kopete_oscar_LIB_SRCS})

target_link_libraries(kopete_oscar ${KDE4_KIO_LIBS} kopete oscar )

set_target_properties(kopete_oscar PROPERTIES VERSION ${GENERIC_LIB_SOVERSION} SOVERSION ${GENERIC_LIB_VERSION} )
install(TARGETS kopete_oscar  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############





