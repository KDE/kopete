kde4_header()

add_subdirectory( liboscar ) 
add_subdirectory( aim ) 
add_subdirectory( icq ) 
add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/liboscar/ 
)


########### next target ###############

set(kopete_oscar_LIB_SRCS 
   oscaraccount.cpp 
   oscarcontact.cpp 
   oscarmyselfcontact.cpp 
   oscarencodingselectiondialog.cpp 
   oscarlistnonservercontacts.cpp 
   oscarvisibilitydialog.cpp )

kde4_add_ui3_files(kopete_oscar_LIB_SRCS oscarencodingselectionbase.ui oscarlistcontactsbase.ui oscarvisibilitybase.ui )

kde4_automoc(${kopete_oscar_LIB_SRCS})

kde4_add_library(kopete_oscar SHARED ${kopete_oscar_LIB_SRCS})

target_link_libraries(kopete_oscar ${KDE4_KIO_LIBS} kopete oscar )

set_target_properties(kopete_oscar PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kopete_oscar )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = liboscar . aim icq icons
#METASOURCES = AUTO
#AM_CPPFLAGS = -I./ui -I$(srcdir)/ui \
#	-I./liboscar -I$(srcdir)/liboscar \
#	 $(KOPETE_INCLUDES) $(all_includes)
#
#lib_LTLIBRARIES = libkopete_oscar.la
#
#libkopete_oscar_la_SOURCES = oscaraccount.cpp oscarcontact.cpp oscarmyselfcontact.cpp \
#	oscarencodingselectionbase.ui oscarencodingselectiondialog.cpp \
#	oscarlistcontactsbase.ui oscarlistnonservercontacts.cpp \
#	oscarvisibilitybase.ui oscarvisibilitydialog.cpp
#libkopete_oscar_la_LDFLAGS = -no-undefined -version-info 2:0:0 $(all_libraries)
#libkopete_oscar_la_LIBADD  = $(LIB_KIO) $(top_builddir)/kopete/libkopete/libkopete.la ./liboscar/liboscar.la
