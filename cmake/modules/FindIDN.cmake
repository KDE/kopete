# CMake module to search for the IDN library
#
#  IDN_FOUND - Test has found IDN dependencies
#  IDN_INCLUDE_DIR - Include needed for IDN
#  IDN_LIBRARIES - Libraries needed for IDN
#  IDN_DEFINITIONS - Compiler swithces required for using IDN

# Copyright (c) 2006, Will Stephenson <wstephenson@kde.org>
# Copyright (c) 2011, Raphael Kubo da Costa <kubito@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT WIN32)
    find_package(PkgConfig)
	pkg_check_modules(PC_IDN libidn)
	set(IDN_DEFINITIONS ${PC_IDN_CFLAGS_OTHER})
endif (NOT WIN32)

find_path(IDN_INCLUDE_DIR idna.h
    HINTS
    ${PC_IDN_INCLUDEDIR}
    ${PC_IDN_INCLUDE_DIRS}
)

find_library(IDN_LIBRARIES
    NAMES idn libidn idn-11 libidn-11
	HINTS ${PC_IDN_LIBDIR} ${PC_IDN_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IDN REQUIRED_VARS IDN_LIBRARIES IDN_INCLUDE_DIR)

mark_as_advanced(IDN_INCLUDE_DIR IDN_LIBRARIES)
