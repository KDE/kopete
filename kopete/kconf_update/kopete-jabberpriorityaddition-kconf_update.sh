#!/bin/sh
sed -e 's/^\(PluginData_JabberProtocol_Resource=.*\)$/\1\nPluginData_JabberProtocol_Priority=5/'
