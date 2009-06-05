project(kopete)

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${KOPETE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

if(NOT INSIDE_KDENETWORK)

	message ("We aren't inside KDENetwork, Loading KDE Macros")
	find_package(KDE4 REQUIRED)

	include (KDE4Defaults)
	include (MacroLibrary)

	include(CheckIncludeFile)
	include(CheckIncludeFiles)
	include(CheckSymbolExists)
	include(CheckFunctionExists)
	include(CheckLibraryExists)
	include(CheckPrototypeExists)
	include(CheckTypeSize)
	include(MacroBoolTo01)
	include(MacroOptionalAddSubdirectory)

	set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
	if (WIN32)
	   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
	   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
	endif (WIN32)

	find_package(KdepimLibs REQUIRED)

	macro_optional_find_package(Sqlite)
	macro_log_feature(SQLITE_FOUND "SQLite" "SQLite is a Binary-Database" "" FALSE "" "Needed for the SQLite-Backend of the KGet-History and the Kopete-Statistic-Plugin")

endif(NOT INSIDE_KDENETWORK)

KDE4_NO_ENABLE_FINAL(kopete)

FIND_PACKAGE(QImageBlitz REQUIRED)

# Configure checks
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
check_type_size("long" SIZEOF_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

#macro_optional_find_package(LiboRTP)
#macro_log_feature(LIBORTP_FOUND "libortp" "oRTP provides an API to send rtp packets" "http://www.linphone.org/index.php/eng/code_review/ortp" FALSE "0.13" "Needed to build Jingle support in Kopete.")

#macro_optional_find_package(Speex)
#macro_log_feature(SPEEX_FOUND "speex" "speex is an encoder/decoder for the speex audio format." "http://www.speex.org/" FALSE "" "Needed to build Jingle support in Kopete.")

#macro_optional_find_package(QCA2)
#macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed to build Kopete's GroupWise and Jabber protocols.")

#macro_optional_find_package(IDN)
#macro_log_feature(IDN_FOUND "IDN" "GNU Libidn Internationalized Domain Name (IDN) implemenation" "http://www.gnu.org/software/libidn/" FALSE "" "Used by the Kopete Jabber plugin.")

#macro_optional_find_package(Alsa)
#macro_log_feature(ALSA_FOUND "Alsa" "Alsa is Advanced Linux Sound Architecture" "" FALSE "" "Needed for Jingle Support in Kopete Jabber Plugin")

#macro_optional_find_package(Xmms)
#macro_bool_to_01(XMMS_FOUND HAVE_XMMS)
#macro_log_feature(XMMS_FOUND "XMMS" "X MultiMedia System development libraries" "http://www.xmms.org" FALSE "" "Used by the Kopete nowlistening plugin to support the XMMS player.")

#if(XMMS_FOUND)
  # GLIB2_INCLUDE_DIR needed for Xmms
#  macro_optional_find_package(GLIB2)
#endif(XMMS_FOUND)

#macro_optional_find_package(LibXml2)
#macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" "Required for the Kopete webpresence plugin.")

#macro_optional_find_package(LibXslt)
#macro_bool_to_01(LIBXSLT_FOUND HAVE_XSLT)
#macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XML files into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required for the Kopete webpresence plugin.")

#macro_optional_find_package(LibOTR)
#macro_bool_to_01(LIBOTR_FOUND HAVE_LIBOTR)
#macro_log_feature(LIBOTR_FOUND "libotr" "A library to encrypt messages with Off-the-Record encryption" "http://www.cypherpunks.ca/otr" FALSE "3.2.0" "Required for the Kopete otr plugin.")

#macro_optional_find_package(Libmsn)
#macro_bool_to_01(LIBMSN_FOUND HAVE_LIBMSN)
#macro_log_feature(LIBMSN_FOUND "libmsn" "A library providing support for the msn protocol" "http://sourceforge.net/projects/libmsn" FALSE "0.4.0" "Required for the Kopete wlm plugin")

#macro_optional_find_package(LibMeanwhile)
#macro_bool_to_01(LIBMEANWHILE_FOUND HAVE_LIBMEANWHILE)
#macro_log_feature(LIBMEANWHILE_FOUND "libmeanwhile" "A library for protocol support for connection to Sametime servers" "http://meanwhile.sf.net" FALSE "" "Required for the Kopete Meanwhile protocol")

#macro_optional_find_package(Libgadu)
#macro_bool_to_01(LIBGADU_FOUND HAVE_LIBGADU)
#macro_log_feature(LIBGADU_FOUND "libgadu" "A library providing support for Gadu-Gadu protocol" "http://toxygen.net/libgadu/" FALSE "1.8.0" "Required for Kopete Gadu-Gadu protocol")

macro_optional_find_package(LibV4L2)
macro_bool_to_01(LIBV4L2_FOUND HAVE_LIBV4L2)
macro_log_feature(LIBV4L2_FOUND "libv4l2" "Collection of video4linux support libraries" "http://hansdegoede.livejournal.com/3636.html" FALSE "" "Required for better webcam support")

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
	${KOPETE_SOURCE_DIR}/libkopete/
	${KOPETE_BINARY_DIR}/libkopete/
	${KOPETE_SOURCE_DIR}/libkopete/ui/
	${KOPETE_BINARY_DIR}/libkopete/ui/
	${KOPETE_SOURCE_DIR}/libkopete/private/
	${KOPETE_SOURCE_DIR}/libkopete/contactlist/
	${KOPETE_SOURCE_DIR}/libkopete/tasks/
)

#add_subdirectory( libkopete )
#add_subdirectory( kopete )
#add_subdirectory( protocols )
add_subdirectory( plugins )
#add_subdirectory( icons )
#add_subdirectory( sounds )
#add_subdirectory( styles )

if(NOT INSIDE_KDENETWORK)
	macro_display_feature_log()
endif(NOT INSIDE_KDENETWORK)
