cmake_minimum_required(VERSION 3.16)

project(kopete)

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(KF5_VERSION "5.25.0")
set(REQUIRED_QT_VERSION "5.6.0")

#ECM modules configurations
find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_MODULE_DIR} ${ECM_KDE_MODULE_DIR} ${KOPETE_SOURCE_DIR}/cmake/modules)

find_package(Phonon4Qt5 REQUIRED NO_MODULE)
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Xml Sql Test)
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    Crash
    DBusAddons
    DocTools
    Emoticons
    I18n
    KCMUtils
    KHtml
    NotifyConfig
    Parts
    TextEditor
    Wallet
    KDELibs4Support
)

find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddTests)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include_directories(${PHONON_INCLUDES})

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${KDEWIN32_INCLUDES})
endif()

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

if(WIN32)
    set(DISABLE_VIDEOSUPPORT ON)
else()
    option(DISABLE_VIDEOSUPPORT "Build Kopete without video-support" OFF)
endif()

if(DISABLE_VIDEOSUPPORT)
    add_definitions(-DVIDEOSUPPORT_DISABLED)
endif()

set(LIBKLEO_LIB_VERSION_LIB "5.4.0")
find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)

### Alphabetical list of optional packages ###

find_package(GLIB2 QUIET)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "GLib library" URL "http://www.gtk.org" TYPE OPTIONAL PURPOSE "Required for the Nowlistening plugin with XMMS player support")

find_package(IDN QUIET)
set_package_properties(IDN PROPERTIES DESCRIPTION "GNU Libidn International Domain Name(IDN) implementation" URL "http://www.gnu.org/software/libidn/" TYPE OPTIONAL PURPOSE "Required for the Jabber protocol")

find_package(JsonCpp QUIET)
set_package_properties(JsonCpp PROPERTIES DESCRIPTION "JsonCpp is an implementation of a JSON reader and writer in C++" URL "http://jsoncpp.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the jabber protocol")

find_package(Kleopatra QUIET)
set_package_properties(Kleopatra PROPERTIES DESCRIPTION "The Kleopatra libraries (from kdepim)" URL "http://kde.org/" TYPE OPTIONAL PURPOSE "Required for the Cryptography plugin")

find_package(Libgadu QUIET)
set_package_properties(Libgadu PROPERTIES DESCRIPTION "A library providing support for the Gadu-Gadu protocol" URL "http://www.toxygen.net/libgadu" TYPE OPTIONAL PURPOSE "Required for the Gadu-Gadu protocol")

find_package(LibMeanwhile QUIET)
set_package_properties(LibMeanwhile PROPERTIES DESCRIPTION "A library for protocol support for connection to sametime servers" URL "http://meanwhile.sourceforge.net/" TYPE OPTIONAL PURPOSE "Required for the meanwhile protocol")

find_package(LibOTR QUIET)
set_package_properties(LibOTR PROPERTIES DESCRIPTION "Library to encrypt messages with off-the-record encryption" URL "http://www.cypherpunks.ca/otr/" TYPE OPTIONAL PURPOSE "Required for the OTR plugin")

find_package(LibV4L2 QUIET)
set_package_properties(LibV4L2 PROPERTIES DESCRIPTION "Collection of video4linux support libraries" URL "http://hansdegoede.livejournal.com http://linuxtv.org/hq/~hgoede/libv4l/" TYPE OPTIONAL PURPOSE "Required for better webcam support on Linux")

find_package(LibXml2 QUIET)
set_package_properties(LibXml2 PROPERTIES DESCRIPTION "Libraries used to develop XML applications" URL "http://www.xmlsoft.org/" TYPE OPTIONAL PURPOSE "Required for Webpresence plugin")

find_package(LibXslt QUIET)
set_package_properties(LibXslt PROPERTIES DESCRIPTION "A library to transform XML files into other XML files" URL "http://www.xmlsoft.org/XSLT/" TYPE OPTIONAL PURPOSE "Required for the Webpresence plugin")

find_package(Qca-qt5 2.1.0)
set_package_properties(Qca-qt5 PROPERTIES DESCRIPTION "Support for encryption"
                       URL "http://download.kde.org/stable/qca-qt5/"
                       TYPE RECOMMENDED)

set(GPGMEPP_LIB_VERSION "1.8.0")
find_package(Gpgmepp ${GPGMEPP_LIB_VERSION} CONFIG REQUIRED)

find_package(Sqlite QUIET)
set_package_properties(Sqlite PROPERTIES DESCRIPTION "SQLite is a Binary-Database" URL "http://www.sqlite.org" TYPE OPTIONAL PURPOSE "Required for the Statistic plugin")

find_package(Xmms QUIET)
set_package_properties(Xmms PROPERTIES DESCRIPTION "X Multimedia System development libraries" URL "http://www.xmms.org/" TYPE OPTIONAL PURPOSE "Required for the Nowlistening plugin with XMMS player support")

find_package(ZLIB QUIET)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Zlib is a library implementing the deflate compression method" URL "http://zlib.net/" TYPE OPTIONAL PURPOSE "Required for the GroupWise and Jabber protocols")

if(Q_WS_X11)
    macro_log_feature(X11_Xss_FOUND "libXss" "libXss provides access to screen server X11 extension" "" FALSE "" "Required for idle timer on X11")
endif()

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
add_subdirectory(tests)

if (${ECM_VERSION} VERSION_GREATER "5.58.0")
    install( FILES kopete.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR} )
else()
    install( FILES kopete.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
endif()

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
