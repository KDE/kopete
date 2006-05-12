
#add_subdirectory( plugins ) 
add_subdirectory( accounts ) 
add_subdirectory( behavior ) 
add_subdirectory( appearance ) 
add_subdirectory( identity ) 

# TODO Renable avdevice config
#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AVDEVICEDIR)")



########### install files ###############





#original Makefile.am contents follow:

#
#if !enable_av
#AVDEVICEDIR =
#else
#AVDEVICEDIR = avdevice
#endif
#
#
#SUBDIRS = plugins accounts behavior appearance identity $(AVDEVICEDIR)
#
#
