
add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 

########### install files ###############

install( FILES main.css Footer.html Header.html Status.html  DESTINATION ${DATA_INSTALL_DIR}/kopete/styles/Clean/Contents/Resources)

