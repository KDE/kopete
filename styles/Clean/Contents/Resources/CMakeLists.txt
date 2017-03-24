
add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 

########### install files ###############

install( FILES main.css Footer.html Header.html Status.html  DESTINATION ${KDE_INSTALL_DATADIR}/kopete/styles/Clean/Contents/Resources)

