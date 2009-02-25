
add_subdirectory( icons )

include_directories( 
	${KOPETE_INCLUDES}
	${TELEPATHY_QT4_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/ui
	${CMAKE_CURRENT_BINARY_dir}/ui
)

# UI classes
set(kopete_telepathy_ui_SRCS
#ui/telepathyeditaccountwidget.cpp
#ui/telepathyeditparameterwidget.cpp
#ui/telepathyaddcontactpage.cpp
)

# Generated UI files
kde4_add_ui_files(kopete_telepathy_ui_SRCS
#ui/telepathyeditaccountwidget.ui
#ui/telepathyaddcontactpage.ui
)

# Normal protocol classes
set(kopete_telepathy_PART_SRCS 
telepathyprotocol.cpp
telepathyaccount.cpp
telepathycontact.cpp
${kopete_telepathy_ui_SRCS}
)

kde4_add_plugin(kopete_telepathy ${kopete_telepathy_PART_SRCS})

target_link_libraries(kopete_telepathy
	${KDE4_KIO_LIBS}
	kopete
	${TELEPATHY_QT4_LIBRARIES}
)

install(TARGETS kopete_telepathy  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_telepathy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
