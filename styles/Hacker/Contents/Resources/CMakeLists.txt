
add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 
add_subdirectory( Variants ) 

########### install files ###############

install( FILES main.css Footer.html Header.html Status.html  DESTINATION ${DATA_INSTALL_DIR}/kopete/styles/Hacker/Contents/Resources)

