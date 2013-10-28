project(kopete)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)
include(CheckTypeSize)

set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if(WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif(WIN32)
add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${KOPETE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

if (WIN32)
	set(DISABLE_VIDEOSUPPORT ON)
else (WIN32)
	option(DISABLE_VIDEOSUPPORT "Build Kopete without video-support" OFF)
endif (WIN32)
if (DISABLE_VIDEOSUPPORT)
	add_definitions(-DVIDEOSUPPORT_DISABLED)
endif (DISABLE_VIDEOSUPPORT)

find_package(KdepimLibs REQUIRED)

macro_optional_find_package(Sqlite)
macro_log_feature(SQLITE_FOUND "SQLite" "SQLite is a Binary-Database" "" FALSE "" "Needed for the SQLite-Backend of the Kopete-Statistic-Plugin")

KDE4_NO_ENABLE_FINAL(kopete)

FIND_PACKAGE(QImageBlitz REQUIRED)

# Configure checks
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
check_type_size("long" SIZEOF_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

macro_optional_find_package(LiboRTP)
macro_log_feature(LIBORTP_FOUND "libortp" "oRTP provides an API to send rtp packets" "http://www.linphone.org/index.php/eng/code_review/ortp" FALSE "0.13" "Needed to build Google Talk libjingle audio call support and voice clips in WLM.")

macro_optional_find_package(SRTP)
macro_log_feature(SRTP_FOUND "srtp" "SRTP provides an implementation of the Secure Real-time Transport Protocol (SRTP)" "http://srtp.sourceforge.net/srtp.html" FALSE "" "Needed to build Google Talk libjingle audio call support")

macro_optional_find_package(JsonCpp)
macro_log_feature(JSONCPP_FOUND "jsoncpp" "jsoncpp is an implementation of a JSON reader and writer in C++" "http://jsoncpp.sourceforge.net/" FALSE "" "Needed to build Google Talk libjingle audio call support")

#macro_optional_find_package(Speex)
#macro_log_feature(SPEEX_FOUND "speex" "speex is an encoder/decoder for the speex audio format." "http://www.speex.org/" FALSE "" "Needed to build Jingle support in Kopete.")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed to build Kopete's GroupWise and Jabber protocols.")

macro_optional_find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "zlib" "zlib is a library implementing the deflate compression method" "http://zlib.net/" FALSE "" "Needed to build Kopete's GroupWise and Jabber protocols.")

macro_optional_find_package(IDN)
macro_log_feature(IDN_FOUND "IDN" "GNU Libidn Internationalized Domain Name (IDN) implementation" "http://www.gnu.org/software/libidn/" FALSE "" "Used by the Kopete Jabber plugin.")

macro_optional_find_package(Alsa)
macro_log_feature(ALSA_FOUND "Alsa" "Alsa is Advanced Linux Sound Architecture" "" FALSE "" "Needed to build Google Talk libjingle audio call support")

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
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XML files into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required for the Kopete webpresence plugin.")

macro_optional_find_package(LibOTR)
macro_bool_to_01(LIBOTR_FOUND HAVE_LIBOTR)
macro_log_feature(LIBOTR_FOUND "libotr" "A library to encrypt messages with Off-the-Record encryption (version 4.0.0) or greater" "http://www.cypherpunks.ca/otr" FALSE "4.0.0" "Required for the Kopete otr plugin.")

macro_optional_find_package(Libmsn)
macro_bool_to_01(LIBMSN_FOUND HAVE_LIBMSN)
macro_log_feature(LIBMSN_FOUND "libmsn" "A library providing support for the msn protocol" "http://sourceforge.net/projects/libmsn" FALSE "0.4.0" "Required for the Kopete wlm plugin")

include(CheckStructMember)
macro_optional_find_package(GIF)
macro_bool_to_01(GIF_FOUND HAVE_GIFLIB)
macro_log_feature( GIF_FOUND "giflib" "Handwriting messages support for WLM" "http://sourceforge.net/projects/giflib/" FALSE "" "Required for the Kopete WLM protocol")

macro_optional_find_package(Mediastreamer)
macro_bool_to_01(MEDIASTREAMER_FOUND HAVE_MEDIASTREAMER)
macro_log_feature( MEDIASTREAMER_FOUND "mediastreamer" "A streaming engine specialized for voice/video telephony applications" "http://www.linphone.org/index.php/eng/code_review/mediastreamer2" FALSE "2.3.0" "Required to enable voice clips support in WLM plugin and needed to build Google Talk libjingle audio call support in Kopete.")

macro_optional_find_package(LibMeanwhile)
macro_bool_to_01(LIBMEANWHILE_FOUND HAVE_LIBMEANWHILE)
macro_log_feature(LIBMEANWHILE_FOUND "libmeanwhile" "A library for protocol support for connection to Sametime servers" "http://meanwhile.sf.net" FALSE "" "Required for the Kopete Meanwhile protocol")

macro_optional_find_package(Libgadu)
macro_bool_to_01(LIBGADU_FOUND HAVE_LIBGADU)
macro_log_feature(LIBGADU_FOUND "libgadu" "A library providing support for Gadu-Gadu protocol" "http://toxygen.net/libgadu/" FALSE "1.8.0" "Required for Kopete Gadu-Gadu protocol")

macro_optional_find_package(Kleopatra)
macro_log_feature(KLEOPATRA_FOUND "Kleopatra" "The Kleopatra libraries (from kdepim)" "" FALSE "" "Kleopatra is required to build the cryptography plugin")

macro_optional_find_package(QGpgme)
macro_log_feature(QGPGME_FOUND "QGpgME" "QGpgME library (from kdepimlibs)" "" FALSE "" "QGpgME library is required to build the cryptography plugin")

macro_optional_find_package(Jasper)
macro_log_feature(JASPER_FOUND Jasper "JasPer library implements the codec specified in the JPEG-2000 Part-1 standard" "http://www.ece.uvic.ca/~mdadams/jasper/" FALSE "" "Required in order to be able to receive and send webcam images from/to Yahoo in Kopete")

macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND Boost "C++ Boost libraries" "http://www.boost.org/" FALSE "" "Required for the Kopete WLM protocol")

if (NOT DISABLE_VIDEOSUPPORT)
  set (LIBV4L2_REQUIRED FALSE)
  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set (LIBV4L2_REQUIRED TRUE)
  endif (CMAKE_SYSTEM_NAME MATCHES Linux)
  find_package(LibV4L2)
  macro_log_feature(LIBV4L2_FOUND "libv4l" "Collection of video4linux support libraries" "http://hansdegoede.livejournal.com, http://linuxtv.org/hg/~hgoede/libv4l/" LIBV4L2_REQUIRED "" "Required for better webcam support")
endif (NOT DISABLE_VIDEOSUPPORT)
macro_bool_to_01(LIBV4L2_FOUND HAVE_LIBV4L2)

check_include_files(valgrind/valgrind.h HAVE_VALGRIND_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(linux/videodev.h HAVE_V4L)

macro_optional_find_package(Expat)
macro_log_feature(EXPAT_FOUND "Expat" "Expat is a stream oriented XML parser" "http://expat.sourceforge.net/" FALSE "" "Needed to build Google Talk libjingle support")

if ( NOT WIN32 )
        macro_optional_find_package(OpenSSL)
        macro_log_feature(OPENSSL_FOUND "OpenSSL" "OpenSSL implementation of SSL" "https://www.openssl.org/" FALSE "" "Needed to build Google Talk libjingle support")
endif ( NOT WIN32 )

# Generate config-kopete.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kopete.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kopete.h )

# For config-kopete.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(KOPETE_INCLUDES 
	${KDE4_INCLUDES}
        ${KDEPIMLIBS_INCLUDE_DIRS}
	${KOPETE_SOURCE_DIR}/libkopete/
	${KOPETE_BINARY_DIR}/libkopete/
	${KOPETE_SOURCE_DIR}/libkopete/ui/
	${KOPETE_SOURCE_DIR}/libkopete/private/
	${KOPETE_SOURCE_DIR}/libkopete/contactlist/
	${KOPETE_SOURCE_DIR}/libkopete/tasks/
)

add_subdirectory( libkopete )
add_subdirectory( kopete )
add_subdirectory( protocols )
add_subdirectory( plugins )
add_subdirectory( icons )
add_subdirectory( sounds )
add_subdirectory( styles )
add_subdirectory( doc )

macro_display_feature_log()
