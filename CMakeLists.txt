cmake_minimum_required(VERSION 2.8.9)
project(kopete)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(KF5_VERSION "5.11.0")
set(REQUIRED_QT_VERSION "5.6.0")

#ECM modules configurations
find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_MODULE_DIR} ${ECM_KDE_MODULE_DIR} ${KOPETE_SOURCE_DIR}/cmake/modules)

find_package(Phonon4Qt5 REQUIRED NO_MODULE)
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Xml Sql Test)
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS
	CoreAddons
	I18n
	DocTools
	KCMUtils
	KHtml
	NotifyConfig
	Parts
	Wallet
	KDELibs4Support
	#[[Phonon]]
)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Emoticons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddTests)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include_directories(${PHONON_INCLUDES})

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${KDEWIN32_INCLUDES})
endif(WIN32)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

if(WIN32)
    set(DISABLE_VIDEOSUPPORT ON)
else (WIN32)
    option(DISABLE_VIDEOSUPPORT "Build Kopete without video-support" OFF)
endif(WIN32)

if(DISABLE_VIDEOSUPPORT)
    add_definitions(-DVIDEOSUPPORT_DISABLED)
endif(DISABLE_VIDEOSUPPORT)


### Alphabetical list of optional packages ###

find_package(Alsa QUIET)
set_package_properties(Alsa PROPERTIES DESCRIPTION "Alsa is advanced linux sound Architecture" URL "http://www.alsa-project.org" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support")

find_package(Boost QUIET)
set_package_properties(Boost PROPERTIES DESCRIPTION "C++ Boost libraries" URL "http://www.boost.org/" TYPE OPTIONAL PURPOSE "Required for the WLM protocol")

find_package(Expat QUIET)
set_package_properties(Expat PROPERTIES DESCRIPTION "Expat is a stream oriented XML parser" URL "http://expat.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support")

find_package(GIF QUIET)
set_package_properties(GIF PROPERTIES DESCRIPTION "GIF - Package of portable tools and library routines for working with GIF images" URL "http://giflib.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the WLM protocol with handwriting messages support")

find_package(GLIB2 QUIET)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "GLib library" URL "http://www.gtk.org" TYPE OPTIONAL PURPOSE "Required for the Nowlistening plugin with XMMS player support")

find_package(IDN QUIET)
set_package_properties(IDN PROPERTIES DESCRIPTION "GNU Libidn International Domain Name(IDN) implementation" URL "http://www.gnu.org/software/libidn/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol")

find_package(Jasper QUIET)
set_package_properties(Jasper PROPERTIES DESCRIPTION "Jasper library implements the codec specified in the JPEG-200 Part-1 standard" URL "http://www.ece.uvic.ca/~mdadams/jasper/" TYPE OPTIONAL PURPOSE "Required for the Yahoo protocol")

find_package(JsonCpp QUIET)
set_package_properties(JsonCpp PROPERTIES DESCRIPTION "JsonCpp is an implementation of a JSON reader and writer in C++" URL "http://jsoncpp.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the jabber protocol")

find_package(Kleopatra QUIET)
set_package_properties(Kleopatra PROPERTIES DESCRIPTION "The Kleopatra libraries (from kdepim)" URL "http://kde.org/" TYPE OPTIONAL PURPOSE "Required for the Cryptography plugin")

find_package(Libgadu QUIET)
set_package_properties(Libgadu PROPERTIES DESCRIPTION "A library providing support for the Gadu-Gadu protocol" URL "http://www.toxygen.net/libgadu" TYPE OPTIONAL PURPOSE "Required for the Gadu-Gadu protocol")

find_package(LibMeanwhile QUIET)
set_package_properties(LibMeanwhile PROPERTIES DESCRIPTION "A library for protocol support for connection to sametime servers" URL "http://meanwhile.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the meanwhile protocol")

find_package(Libmsn QUIET)
set_package_properties(Libmsn PROPERTIES DESCRIPTION "A library providing support for the msn protocol" URL "http://libmsn.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the WLM protocol")

find_package(LiboRTP QUIET)
set_package_properties(LiboRTP PROPERTIES DESCRIPTION "oRTP provides an API to send rtp packets" URL "http://www.linphone.org/index.php/eng/code_review/ortp/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support and for the WLM protocol with voice clips support")

find_package(LibOTR QUIET)
set_package_properties(LibOTR PROPERTIES DESCRIPTION "Library to encrypt messages with off-the-record encryption" URL "http://www.cypherpunks.ca/otr/" TYPE OPTIONAL PURPOSE "Required for the OTR plugin")

find_package(LibV4L2 QUIET)
set_package_properties(LibV4L2 PROPERTIES DESCRIPTION "Collection of video4linux support libraries" URL "http://hansdegoede.livejournal.com http://linuxtv.org/hq/~hgoede/libv4l/" TYPE OPTIONAL PURPOSE "Required for better webcam support on Linux")

find_package(LibXml2 QUIET)
set_package_properties(LibXml2 PROPERTIES DESCRIPTION "Libraries used to develop XML applications" URL "http://www.xmlsoft.org/" TYPE OPTIONAL PURPOSE "Required for Webpresence plugin")

find_package(LibXslt QUIET)
set_package_properties(LibXslt PROPERTIES DESCRIPTION "A library to transform XML files into other XML files" URL "http://www.xmlsoft.org/XSLT/" TYPE OPTIONAL PURPOSE "Required for the Webpresence plugin")

find_package(Mediastreamer QUIET)
set_package_properties(Mediastreamer PROPERTIES DESCRIPTION "A streaming enginer specialized for voice/video telephony applications" URL "http://www.linphone.org/index.php/eng/code_review/mediastreamer2/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support and for the WLM protocol with voice clips support")

find_package(OpenSSL QUIET)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "OpenSSL implementation of SSL" URL "https://www.openssl.org/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support")

find_package(Qca-qt5 QUIET CONFIG)
set_package_properties(Qca-qt5 PROPERTIES DESCRIPTION "Qt Cryptographic Architecture" URL "http://delta.affinix.com/qca/" TYPE OPTIONAL PURPOSE "Required for the GroupWise and Jabber protocols")

find_package(QGpgme QUIET CONFIG)
set_package_properties(QGpgme PROPERTIES DESCRIPTION "QGpgME library (from kdepimlibs)" URL "http://www.kde.org/" TYPE OPTIONAL PURPOSE "Required for the Cryptography plugin")

find_package(Sqlite QUIET)
set_package_properties(Sqlite PROPERTIES DESCRIPTION "SQLite is a Binary-Database" URL "http://www.sqlite.org" TYPE OPTIONAL PURPOSE "Required for the Statistic plugin")

find_package(Speex QUIET)
set_package_properties(Speex PROPERTIES DESCRIPTION "Speex is an encoder/decoder for the speex audio format" URL "http://www.speex.org/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with Jingle support")

find_package(SRTP QUIET)
set_package_properties(SRTP PROPERTIES DESCRIPTION "SRTP provides an implementation of the Secure Real-time Transport protocol(SRTP)" URL "http://srtp.sourceforge.net/srtp.html" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol with libjingle support")

find_package(Xmms QUIET)
set_package_properties(Xmms PROPERTIES DESCRIPTION "X Multimedia System development libraries" URL "http://www.xmms.org/" TYPE OPTIONAL PURPOSE "Required for the Nowlistening plugin with XMMS player support")

find_package(ZLIB QUIET)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Zlib is a library implementing the deflate compression method" URL "http://zlib.net/" TYPE OPTIONAL PURPOSE "Required for the GroupWise and Jabber protocols")

if(Q_WS_X11)
    macro_log_feature(X11_Xss_FOUND "libXss" "libXss provides access to screen server X11 extension" "" FALSE "" "Required for idle timer on X11")
endif(Q_WS_X11)

set(KOPETE_INCLUDES

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


#macro_display_feature_log()
