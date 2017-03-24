
add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 
add_subdirectory( Variants )

########### install files ###############

install( FILES main.css Footer.html Header.html Status.html  DESTINATION ${KDE_INSTALL_DATADIR}/kopete/styles/Clear/Contents/Resources)

