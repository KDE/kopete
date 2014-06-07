project(kopete)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KdepimLibs REQUIRED)
find_package(QImageBlitz REQUIRED)

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${KOPETE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${KDEWIN32_INCLUDES})
endif(WIN32)

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if(WIN32)
    set(DISABLE_VIDEOSUPPORT ON)
else (WIN32)
    option(DISABLE_VIDEOSUPPORT "Build Kopete without video-support" OFF)
endif(WIN32)

if(DISABLE_VIDEOSUPPORT)
    add_definitions(-DVIDEOSUPPORT_DISABLED)
endif(DISABLE_VIDEOSUPPORT)

kde4_no_enable_final(kopete)

### Alphabetical list of optional packages ###

macro_optional_find_package(Alsa)
macro_log_feature(ALSA_FOUND "Alsa" "Alsa is Advanced Linux Sound Architecture" "http://www.alsa-project.org/" FALSE "" "Required for the Jabber protocol with libjingle support")

macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND Boost "C++ Boost libraries" "http://www.boost.org/" FALSE "" "Required for the WLM protocol")

macro_optional_find_package(Expat)
macro_log_feature(EXPAT_FOUND "Expat" "Expat is a stream oriented XML parser" "http://expat.sourceforge.net/" FALSE "" "Required for the Jabber protocol with libjingle support")

macro_optional_find_package(GIF)
macro_log_feature(GIF_FOUND "GIF" "Package of portable tools and library routines for working with GIF images" "http://giflib.sourceforge.net/" FALSE "" "Required for the WLM protocol with handwriting messages support")

macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "GLIB2" "GLib library" "http://www.gtk.org/" FALSE "" "Required for the Nowlistening plugin with XMMS player support")

macro_optional_find_package(IDN)
macro_log_feature(IDN_FOUND "IDN" "GNU Libidn Internationalized Domain Name (IDN) implementation" "http://www.gnu.org/software/libidn/" FALSE "" "Required for the Jabber protocol")

macro_optional_find_package(Jasper)
macro_log_feature(JASPER_FOUND "Jasper" "JasPer library implements the codec specified in the JPEG-2000 Part-1 standard" "http://www.ece.uvic.ca/~mdadams/jasper/" FALSE "" "Required for the Yahoo protocol")

macro_optional_find_package(JsonCpp)
macro_log_feature(JSONCPP_FOUND "JsonCpp" "JsonCpp is an implementation of a JSON reader and writer in C++" "http://jsoncpp.sourceforge.net/" FALSE "" "Required for the Jabber protocol with libjingle support")

macro_optional_find_package(Kleopatra)
macro_log_feature(KLEOPATRA_FOUND "Kleopatra" "The Kleopatra libraries (from kdepim)" "http://kde.org/" FALSE "" "Required for the Cryptography plugin")

macro_optional_find_package(Libgadu)
macro_log_feature(LIBGADU_FOUND "Libgadu" "A library providing support for Gadu-Gadu protocol" "http://toxygen.net/libgadu/" FALSE "1.8.0" "Required for the Gadu-Gadu protocol")

macro_optional_find_package(LibMeanwhile)
macro_log_feature(LIBMEANWHILE_FOUND "LibMeanwhile" "A library for protocol support for connection to Sametime servers" "http://meanwhile.sourceforge.net/" FALSE "" "Required for the Meanwhile protocol")

macro_optional_find_package(Libmsn)
macro_log_feature(LIBMSN_FOUND "Libmsn" "A library providing support for the msn protocol" "http://libmsn.sourceforge.net/" FALSE "0.4.0" "Required for the WLM protocol")

macro_optional_find_package(LiboRTP)
macro_log_feature(LIBORTP_FOUND "LiboRTP" "oRTP provides an API to send rtp packets" "http://www.linphone.org/index.php/eng/code_review/ortp/" FALSE "0.13" "Required for the Jabber protocol with libjingle support and for the WLM protocol with voice clips support")

macro_optional_find_package(LibOTR)
macro_log_feature(LIBOTR_FOUND "LibOTR" "A library to encrypt messages with Off-the-Record encryption" "http://www.cypherpunks.ca/otr/" FALSE "4.0.0" "Required for the OTR plugin")

macro_optional_find_package(LibV4L2)
macro_log_feature(LIBV4L2_FOUND "LibV4L2" "Collection of video4linux support libraries" "http://hansdegoede.livejournal.com/ http://linuxtv.org/hg/~hgoede/libv4l/" FALSE "" "Required for better webcam support on Linux")

macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXml2" "Libraries used to develop XML applications" "http://xmlsoft.org/" FALSE "" "Required for the Webpresence plugin")

macro_optional_find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXslt" "A library to transform XML files into other XML files" "http://xmlsoft.org/XSLT/" FALSE "" "Required for the Webpresence plugin")

macro_optional_find_package(Mediastreamer)
macro_log_feature(MEDIASTREAMER_FOUND "Mediastreamer" "A streaming engine specialized for voice/video telephony applications" "http://www.linphone.org/index.php/eng/code_review/mediastreamer2/" FALSE "2.3.0" "Required for the Jabber protocol with libjingle support and for the WLM protocol with voice clips support")

macro_optional_find_package(OpenSSL)
macro_log_feature(OPENSSL_FOUND "OpenSSL" "OpenSSL implementation of SSL" "https://www.openssl.org/" FALSE "" "Required for the Jabber protocol with libjingle support")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca/" FALSE "2.0.0" "Required for the GroupWise and Jabber protocols")

macro_optional_find_package(QGpgme)
macro_log_feature(QGPGME_FOUND "QGpgme" "QGpgME library (from kdepimlibs)" "http://www.kde.org/" FALSE "" "Required for the Cryptography plugin")

macro_optional_find_package(Sqlite)
macro_log_feature(SQLITE_FOUND "Sqlite" "SQLite is a Binary-Database" "http://www.sqlite.org/" FALSE "" "Required for the Statistic plugin")

#macro_optional_find_package(Speex)
#macro_log_feature(SPEEX_FOUND "Speex" "Speex is an encoder/decoder for the speex audio format" "http://www.speex.org/" FALSE "" "Required for the Jabber protocol with Jingle support")

macro_optional_find_package(SRTP)
macro_log_feature(SRTP_FOUND "SRTP" "SRTP provides an implementation of the Secure Real-time Transport Protocol (SRTP)" "http://srtp.sourceforge.net/srtp.html" FALSE "" "Required for the Jabber protocol with libjingle support")

macro_optional_find_package(Xmms)
macro_log_feature(XMMS_FOUND "Xmms" "X MultiMedia System development libraries" "http://www.xmms.org/" FALSE "" "Required for the Nowlistening plugin with XMMS player support")

macro_optional_find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "ZLIB" "Zlib is a library implementing the deflate compression method" "http://zlib.net/" FALSE "" "Required for the GroupWise and Jabber protocols")

if(Q_WS_X11)
    macro_log_feature(X11_Xss_FOUND "libXss" "libXss provides access to screen server X11 extension" "" FALSE "" "Required for idle timer on X11")
endif(Q_WS_X11)

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

add_subdirectory(libkopete)
add_subdirectory(kopete)
add_subdirectory(protocols)
add_subdirectory(plugins)
add_subdirectory(icons)
add_subdirectory(sounds)
add_subdirectory(styles)
add_subdirectory(doc)

macro_display_feature_log()
