PROJECT(quick)

# linker flags
IF(WIN32)
    LINK_DIRECTORIES(${PATH_LIB_OUTPUT} ${WIN32_3RDPARTY_LIBS})
ELSE(WIN32)
	LINK_DIRECTORIES(${PATH_LIB_OUTPUT} /usr/local/lib /opt/local/lib)
ENDIF(WIN32)

# source files
SET(APP_SRC
	quick.cpp
)

# linker settings
set(targetname "quick")
ADD_EXECUTABLE(${targetname} ${APP_SRC})
set_property(TARGET ${targetname} PROPERTY FOLDER examples)

IF(WIN32)
    TARGET_LINK_LIBRARIES(${targetname} optimized ais_decoder)
    TARGET_LINK_LIBRARIES(${targetname} debug ais_decoder)

ELSE(WIN32)
    IF(MACX)
        TARGET_LINK_LIBRARIES(${targetname} "-framework CoreFoundation -framework Foundation")
        TARGET_LINK_LIBRARIES(${targetname} "-Wl,-export_dynamic,-force_flat_namespace,-F/Library/Frameworks")
		TARGET_LINK_LIBRARIES(${targetname} ais_decoder)
        TARGET_LINK_LIBRARIES(${targetname} "-stdlib=libc++")

    ELSE(MACX)
        TARGET_LINK_LIBRARIES(${targetname} "-Wl,--no-as-needed,-ldl,-lpthread")
        TARGET_LINK_LIBRARIES(${targetname} ais_decoder)

    ENDIF(MACX)
ENDIF(WIN32)


