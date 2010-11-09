# Try to find the KCM Telepathy Accounts library
# KCMTELEPATHYACCOUNTS_FOUND - system has libkcmtelepathyaccounts
# KCMTELEPATHYACCOUNTS_INCLUDE_DIR - the kcmtelepathyaccounts include directory
# KCMTELEPATHYACCOUNTS_LIBRARIES - Link these to use libkcmtelepathyaccounts

# Copyright (c) 2008, Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET (KCMTELEPATHYACCOUNTS_FIND_REQUIRED ${KCMTelepathyAccounts_FIND_REQUIRED})
if (KCMTELEPATHYACCOUNTS_INCLUDE_DIR AND KCMTELEPATHYACCOUNTS_LIBRARIES)
  # Already in cache, be silent
  set(KCMTELEPATHYACCOUNTS_FIND_QUIETLY TRUE)
endif (KCMTELEPATHYACCOUNTS_INCLUDE_DIR AND KCMTELEPATHYACCOUNTS_LIBRARIES)

find_path(KCMTELEPATHYACCOUNTS_INCLUDE_DIR
  NAMES KCMTelepathyAccounts/AbstractAccountUi KCMTelepathyAccounts/abstract-account-parameters-widget.h)
find_library(KCMTELEPATHYACCOUNTS_LIBRARIES NAMES kcmtelepathyaccounts )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KCMTELEPATHYACCOUNTS DEFAULT_MSG
                                  KCMTELEPATHYACCOUNTS_LIBRARIES KCMTELEPATHYACCOUNTS_INCLUDE_DIR)


mark_as_advanced(KCMTELEPATHYACCOUNTS_INCLUDE_DIR KCMTELEPATHYACCOUNTS_LIBRARIES)
