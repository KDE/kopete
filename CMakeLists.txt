project(kdenetwork)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
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

# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if (WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
endif (WIN32)

find_package(KdepimLibs REQUIRED)
# find_package(X11VidMode) not used at this time
find_package(LiboRTP)
find_package(Alsa)
find_package(Speex)
find_package(QCA2)
find_package(JPEG)
find_package(X11VidMode)
find_package(Decibel)
find_package(IDN)
find_package(LibVNCServer)

macro_optional_find_package(LibVNCServer)
macro_log_feature(LIBVNCSERVER_FOUND "libvncserver" "VNC Server library" "http://libvncserver.sourceforge.net/" FALSE "0.9" "Needed to build Krfb and VNC support in Krdc")

# NX support is not ready for KDE 4.2; disabled (uwolfer)
# macro_optional_find_package(LibNXCL)
# macro_log_feature(LIBNXCL_FOUND "libnxcl" "NX X compression client library" "http://svn.berlios.de/svnroot/repos/freenx/trunk/freenx-client/nxcl/" FALSE "1.0" "Needed to build Krdc with NX support")

macro_log_feature(LIBORTP_FOUND "libortp" "oRTP provides an API to send rtp packets" "http://www.linphone.org/index.php/eng/code_review/ortp" FALSE "0.13" "Needed to build Jingle support in Kopete.")

macro_log_feature(SPEEX_FOUND "speex" "speex is an encoder/decoder for the speex audio format." "http://www.speex.org/" FALSE "" "Needed to build Jingle support in Kopete.")

macro_optional_find_package(JPEG)
macro_log_feature(JPEG_FOUND "libjpeg" "Development library for JPEG formatted image files" "http://www.ijg.org/" FALSE "" "Provides JPEG images in Krdc.")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed to build Kopete's GroupWise and Jabber protocols.")

macro_optional_find_package(Decibel)
macro_log_feature(DECIBEL_FOUND "Decibel"  "KDE framework for real-time communication" "http://decibel.kde.org/" FALSE "" "Used by the Kopete telepathy protocol.")

macro_optional_find_package(IDN)
macro_log_feature(IDN_FOUND "IDN" "GNU Libidn Internationalized Domain Name (IDN) implemenation" "http://www.gnu.org/software/libidn/" FALSE "" "Used by the Kopete Jabber plugin.")

macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "" FALSE "" "Soprano is needed for Nepomuk")

macro_optional_find_package(Nepomuk)
macro_log_feature(Nepomuk_FOUND "Nepomuk" "Semantic Desktop" "http://nepomuk.kde.org" FALSE "" "Nepomuk is needed for the integration into KGet")

macro_optional_find_package(Sqlite)
macro_log_feature(SQLITE_FOUND "SQLite" "SQLite is a Binary-Database" "" FALSE "" "Needed for the SQLite-Backend of the KGet-History and the Kopete-Statistic-Plugin")


add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR})

#macro_optional_add_subdirectory(doc)
#macro_optional_add_subdirectory(kfile-plugins)
#macro_optional_add_subdirectory(kget)

if(LIBORTP_FOUND AND SPEEX_FOUND AND ALSA_FOUND AND NOT NO_JINGLE)
  set(BUILD_JINGLE TRUE)
else(LIBORTP_FOUND AND SPEEX_FOUND AND ALSA_FOUND AND NOT NO_JINGLE)
  set(BUILD_JINGLE FALSE)
endif(LIBORTP_FOUND AND SPEEX_FOUND AND ALSA_FOUND AND NOT NO_JINGLE)
macro_optional_add_subdirectory(kopete)

#if(Q_WS_X11)
#  macro_optional_add_subdirectory(krdc)
#  macro_optional_add_subdirectory(kppp)
#
#  if(LIBVNCSERVER_FOUND)
#    macro_optional_add_subdirectory(krfb)
#  endif(LIBVNCSERVER_FOUND)
#
#endif(Q_WS_X11)

#macro_optional_add_subdirectory(kdnssd)

#if(NOT WIN32)
#  macro_optional_add_subdirectory(filesharing)
#endif(NOT WIN32)

macro_display_feature_log()
