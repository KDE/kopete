option(WITH_history "Enable Kopete history plugin" ON)
option(WITH_akonadihistory "Enable Kopete akonadi history plugin" ON)


if (WITH_akonadihistory)
  add_subdirectory( akonadi_history_plugin )
endif (WITH_akonadihistory)

if (WITH_history)
#  add_subdirectory( history )
endif (WITH_history)

