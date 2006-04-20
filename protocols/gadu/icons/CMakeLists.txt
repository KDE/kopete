kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

kde4_install_icons( ${DATA_INSTALL_DIR}/kopete/icons crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kopeteicondir = $(kde_datadir)/kopete/icons
#kopeteicon_ICON = AUTO
