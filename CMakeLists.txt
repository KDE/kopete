project(kopete)
KDE4_NO_ENABLE_FINAL(kopete)

FIND_PACKAGE(Blitz REQUIRED)

# Configure checks
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
check_type_size("long" SIZEOF_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

macro_optional_find_package(Xmms)
macro_bool_to_01(XMMS_FOUND HAVE_XMMS)
macro_log_feature(XMMS_FOUND "XMMS" "X MultiMedia System development libraries" "http://www.xmms.org" FALSE "" "Used by the Kopete nowlistening plugin to support the XMMS player.")

if(XMMS_FOUND)
  # GLIB2_INCLUDE_DIR needed for Xmms
  macro_optional_find_package(GLIB2)
endif(XMMS_FOUND)

macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" "Required for the Kopete webpresence plugin.")

macro_optional_find_package(LibXslt)
macro_bool_to_01(LIBXSLT_FOUND HAVE_XSLT)
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XML files into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required for the Kopete webpresence plugin.")

macro_optional_find_package(LibOTR)
macro_bool_to_01(LIBOTR_FOUND HAVE_LIBOTR)
macro_log_feature(LIBOTR_FOUND "libotr" "A library to encrypt messages with Off-the-Record encryption" "http://www.cypherpunks.ca/otr" FALSE "3.1.0" "Required for the Kopete otr plugin.")

macro_optional_find_package(Libmsn)
macro_bool_to_01(LIBMSN_FOUND HAVE_LIBMSN)
macro_log_feature(LIBMSN_FOUND "libmsn" "A library providing support for the msn protocol" "http://sourceforge.net/projects/libmsn" FALSE "0.4.0" "Required for the Kopete wlm plugin")

macro_optional_find_package(LibMeanwhile)
macro_bool_to_01(LIBMEANWHILE_FOUND HAVE_LIBMEANWHILE)
macro_log_feature(LIBMEANWHILE_FOUND "libmeanwhile" "A library for protocol support for connection to Sametime servers" "http://libmeanwhile.sf.net" FALSE "" "Required for the Kopete Meanwhile protocol")

check_include_files(valgrind/valgrind.h HAVE_VALGRIND_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
macro_bool_to_01(X11_Xss_FOUND HAVE_XSCREENSAVER)

# Generate config-kopete.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kopete.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kopete.h )

# For config-kopete.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(KOPETE_INCLUDES 
	${KDE4_INCLUDES}
	${CMAKE_SOURCE_DIR}/kopete/libkopete/
	${CMAKE_BINARY_DIR}/kopete/libkopete/
	${CMAKE_SOURCE_DIR}/kopete/libkopete/ui/
	${CMAKE_BINARY_DIR}/kopete/libkopete/ui/
	${CMAKE_SOURCE_DIR}/kopete/libkopete/private/
	${CMAKE_SOURCE_DIR}/kopete/libkopete/contactlist/
	${CMAKE_SOURCE_DIR}/kopete/libkopete/tasks/
)

add_subdirectory( libkopete )
add_subdirectory( kopete )
add_subdirectory( protocols )
add_subdirectory( plugins )
add_subdirectory( icons )
add_subdirectory( sounds )
add_subdirectory( styles )

