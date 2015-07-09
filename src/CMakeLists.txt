include_directories(${KIROCS_INCLUDE_DIR} ${KIROCS_SOURCE_DIR})
link_directories(${KIROCS_BINARY_DIR})

add_library(ucakiro SHARED uca-kiro-camera.c)
target_link_libraries(ucakiro ${KIROCS_DEPS})

set_target_properties(ucakiro PROPERTIES
    VERSION "${LIBKIROCS_VERSION_MAJOR}.${LIBKIROCS_VERSION_MINOR}"
    SOVERSION ${LIBKIROCS_VERSION_PATCH}
)

install(TARGETS ucakiro LIBRARY DESTINATION ${LIBUCA_PLUGINDIR})
install(FILES uca-kiro-camera.h DESTINATION ${KIROCS_INCLUDEDIR})
