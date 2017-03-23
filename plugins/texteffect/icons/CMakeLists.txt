
########### install files ###############

file(GLOB icons_png "*.png")
file(GLOB icons_svg "*.svg")
set(all_icons ${icons_png} ${icons_svg})
ecm_install_icons(ICONS ${all_icons}  DESTINATION ${KDE_INSTALL_DATADIR}/kopete/icons THEME oxygen)



