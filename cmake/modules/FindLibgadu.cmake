# - Try to find libgadu (Gadu Gadu protocol support library)
# Once done this will define
#
#  LIBGADU_FOUND - system has LIBGADU
#  LIBGADU_INCLUDE_DIR - the LIBGADU include directory
#  LIBGADU_LIBRARIES - the libraries needed to use LIBGADU
#  LIBGADU_DEFINITIONS - Compiler switches required for using LIBGADU
#
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

# Copyright (c) 2008, Maciej Mrozowski, <reavertm@poczta.fm>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include (FindLibraryWithDebug)

if (NOT LIBGADU_INCLUDE_DIR OR NOT LIBGADU_LIBRARIES)

    if (NOT WIN32)

        find_package (PkgConfig)
        pkg_check_modules (PC_LIBGADU libgadu)

        # If pkgconfig found libgadu, get the full path to the library using find_library()
        # but only in the path reported by pkgconfig.
        # Otherwise do a normal search.
        if (PC_LIBGADU_FOUND)

            set (LIBGADU_DEFINITIONS ${PC_LIBGADU_CFLAGS})

            if (PC_LIBGADU_INCLUDE_DIRS)
                set (LIBGADU_INCLUDE_DIR ${PC_LIBGADU_INCLUDE_DIRS})
            else (PC_LIBGADU_INCLUDE_DIRS)
                find_path (LIBGADU_INCLUDE_DIR libgadu.h PATH_SUFFIXES libgadu)
            endif (PC_LIBGADU_INCLUDE_DIRS)

            find_library (LIBGADU_LIBRARIES NAMES gadu
                PATHS
                ${PC_LIBGADU_LIBDIR}
                NO_DEFAULT_PATH
            )

        else (PC_LIBGADU_FOUND)

            find_library (LIBGADU_LIBRARIES NAMES gadu)
            find_path (LIBGADU_INCLUDE_DIR libgadu.h PATH_SUFFIXES libgadu)

        endif (PC_LIBGADU_FOUND)

    else (NOT WIN32)

        find_library_with_debug (LIBGADU_LIBRARIES
            WIN32_DEBUG_POSTFIX d
            NAMES gadu
        )
        find_path (LIBGADU_INCLUDE_DIR libgadu.h PATH_SUFFIXES libgadu)

    endif (NOT WIN32)

    include (FindPackageHandleStandardArgs)
    find_package_handle_standard_args (LIBGADU DEFAULT_MSG LIBGADU_LIBRARIES LIBGADU_INCLUDE_DIR)

    mark_as_advanced (LIBGADU_INCLUDE_DIR LIBGADU_LIBRARIES)

endif (NOT LIBGADU_INCLUDE_DIR OR NOT LIBGADU_LIBRARIES)

if (LIBGADU_INCLUDE_DIR AND LIBGADU_LIBRARIES)

    if (NOT WIN32)

        try_run (run_result compile_result ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/gadu_pthread_check.cpp
            CMAKE_FLAGS -DINCLUDE_DIRECTORIES:STRING=${LIBGADU_INCLUDE_DIR}
            COMPILE_DEFINITIONS "${LIBGADU_DEFINITIONS}"
        )

        if (compile_result)

            if (run_result EQUAL 1)

                set (LIBGADU_FOUND TRUE)

            else (run_result EQUAL 1)

                MESSAGE(STATUS "libgadu must be compiled with pthreads support")
                set (LIBGADU_FOUND FALSE)

            endif (run_result EQUAL 1)

        else (compile_result)

            MESSAGE(STATUS "unable to compile libgadu pthread check")
            set (LIBGADU_FOUND FALSE)

        endif (compile_result)

    else (NOT WIN32)

        set (LIBGADU_FOUND TRUE)

    endif (NOT WIN32)

endif (LIBGADU_INCLUDE_DIR AND LIBGADU_LIBRARIES)
