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
find_package(LiboRTP)
find_package(Alsa)
find_package(Speex)
find_package(QCA2)
find_package(IW)
find_package(JPEG)
find_package(X11VidMode)
find_package(Decibel)
find_package(IDN)
find_package(Plasma)
find_package(LibVNCServer)

macro_optional_find_package(DNSSD)
macro_optional_find_package(SLP)
macro_optional_find_package(IDN)
macro_optional_find_package(LibVNCServer)

macro_log_feature(LIBVNCSERVER_FOUND   "libvncserver" "VNC Server lib, needed to build krfb"       "http://libvncserver.sourceforge.net/")
macro_log_feature(DNSSD_FOUND   "libdnssd" "Support for Zeroconf, needed to build dnssd"       "http://www.zeroconf.org")
macro_log_feature(QCA2_FOUND    "libqca2"  "Qca2 is needed to build some parts of kopete"              "Module into kdesupport" FALSE "2.0")
macro_log_feature(SLP_FOUND     "openslp"  "OpenSLP is needed to build krdc and krfb"         "http://www.openslp.org/")
macro_log_feature(IW_FOUND      "libiw"    "Wireless Tools are needed to build wifi project" "http://www.hpl.hp.com/personal/Jean_Tourrilhes/Linux/Tools.html")
macro_log_feature(DECIBEL_FOUND "Decibel"  "Decibel is the KDE framework for real-time communication" "HOWTO in kopete/protocols/telepathy")
macro_log_feature(IDN_FOUND   "libidn" "libidn provides International Domain Name Support for Kopete's Jabber plugin"       "http://www.gnu.org/software/libidn")
macro_log_feature(PLASMA_FOUND "Plasma" "Plasma is a really cool replacement for kicker, needed for KNewsTicker" "Part of kdebase")
macro_log_feature(LIBORTP_FOUND "libortp" "oRTP provides an API to send rtp packets" "http://www.linphone.org/index.php/eng/code_review/ortp")

macro_bool_to_01(SLP_FOUND HAVE_SLP) # for both krfb and krdc

#add_definitions ( -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

#macro_optional_add_subdirectory(doc)
#macro_optional_add_subdirectory(kfile-plugins)
#macro_optional_add_subdirectory(lanbrowsing)
#macro_optional_add_subdirectory(kget)
add_subdirectory(cmake)

if(Q_WS_X11)

  if(PLASMA_FOUND)
  #  macro_optional_add_subdirectory(knewsticker)
  endif(PLASMA_FOUND)

  #macro_optional_add_subdirectory(kppp)

  #add_subdirectory(kpf)
  if(JPEG_FOUND AND SLP_FOUND)

    if(X11VIDMODE_FOUND)
     #  macro_optional_add_subdirectory(krdc)
    endif(X11VIDMODE_FOUND)

  endif(JPEG_FOUND AND SLP_FOUND)

  if(LIBVNCSERVER_FOUND)
  #  macro_optional_add_subdirectory(krfb)
  endif(LIBVNCSERVER_FOUND)

  if(NOT LIBORTP_FOUND OR NOT SPEEX_FOUND OR NOT ALSA_FOUND)
    set(BUILD_JINGLE FALSE)
  else(NOT LIBORTP_FOUND OR NOT SPEEX_FOUND OR NOT ALSA_FOUND)
    set(BUILD_JINGLE TRUE)
  endif(NOT LIBORTP_FOUND OR NOT SPEEX_FOUND OR NOT ALSA_FOUND)

endif(Q_WS_X11)

if (DNSSD_FOUND)
 #  macro_optional_add_subdirectory(kdnssd)
endif (DNSSD_FOUND)

macro_optional_add_subdirectory(kopete)

if (IW_FOUND)
  # macro_optional_add_subdirectory(wifi)
endif (IW_FOUND)

message(STATUS "TODO: port kpf!")
if(NOT WIN32)
  #macro_optional_add_subdirectory(filesharing)
  #macro_optional_add_subdirectory(kdict)
endif(NOT WIN32)

macro_display_feature_log()
