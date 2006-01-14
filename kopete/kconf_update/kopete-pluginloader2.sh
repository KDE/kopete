#!/bin/sh

IFS=:
SUFF=kconf_update_bin/kopete-pluginloader2-kconf_update
for path in `kde-config --path lib`; do
  if test -x "$path/$SUFF"; then
     exec "$path/$SUFF"
  fi
done

