
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/kopete_otr/icons )

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

#icondir = $(kde_datadir)/kopete_otr/icons
#icon_ICON = AUTO
#KDE_ICON = kopete_otr
#
