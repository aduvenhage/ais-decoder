PROJECT(AisDecoderLib)

SET(INCL_SRC
    ais_decoder.h
    ais_utils.h
    ais_file.h
    ais_quick.h
    default_sentence_parser.h
    strutils.h
)

SET(LIB_SRC
    ${INCL_SRC}
    ais_decoder.cpp
    ais_utils.cpp
    ais_file.cpp
    ais_quick.cpp
    default_sentence_parser.cpp
)

set(targetname "ais_decoder")
ADD_LIBRARY(${targetname} STATIC ${LIB_SRC})
set_property(TARGET ${targetname} PROPERTY FOLDER libs)

install(TARGETS ${targetname} DESTINATION ${AIS_LIB_INSTALL_PATH})
install(FILES ${INCL_SRC} DESTINATION ${AIS_SRC_INSTALL_PATH}/${targetname})

