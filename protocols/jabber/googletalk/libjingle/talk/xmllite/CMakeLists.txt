
########### next target ###############

if ( BUILD_libjingle_cricketxmllite )

SET(cricketxmllite_STAT_SRCS
   qname.cc
   xmlbuilder.cc
   xmlconstants.cc
   xmlelement.cc
   xmlnsstack.cc
   xmlparser.cc
   xmlprinter.cc

)

add_library(cricketxmllite STATIC ${cricketxmllite_STAT_SRCS})

endif ( BUILD_libjingle_cricketxmllite )


########### install files ###############

