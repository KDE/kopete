#!/bin/sh
sed "s/PluginData_JabberProtocol_ProxyType=SOCKS4/PluginData_JabberProtocol_ProxyType=SOCKS/" | sed "s/PluginData_JabberProtocol_ProxyType=SOCKS5/PluginData_JabberProtocol_ProxyType=SOCKS/"
