project(kopete)
KDE4_NO_ENABLE_FINAL(kopete)

FIND_PACKAGE(Blitz REQUIRED)

# Configure checks
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
check_type_size("long" SIZEOF_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
macro_bool_to_01(XMMS_FOUND HAVE_XMMS)
macro_optional_find_package(Xmms)
macro_optional_find_package(LibXml2)
macro_optional_find_package(LibXslt)
macro_bool_to_01(LIBXSLT_FOUND HAVE_XSLT)
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

