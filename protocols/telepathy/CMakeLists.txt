project(kopete-protocol-telepathy)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Kopete REQUIRED)
find_package(TelepathyQt4 REQUIRED)
find_package(KCMTelepathyAccounts REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)
include(MacroOptionalAddSubdirectory)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${KOPETE_INCLUDE_DIR}/kopete )

# needed for the share-my-desktop plugin to link against the KopeteTelepathy library.
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/protocols/telepathy)

add_subdirectory(src)
