include_directories(${KIROCS_SOURCE_DIR})
link_directories(${KIROCS_BINARY_DIR})

add_library(libucakiro SHARED uca-kiro-camera.c)
target_link_libraries(libucakiro ${KIROCS_DEPS})

set_target_properties(libucakiro PROPERTIES
    VERSION "${LIBKIROCS_VERSION_MAJOR}.${LIBKIROCS_VERSION_MINOR}"
    SOVERSION ${LIBKIROCS_VERSION_PATCH}
)

install(FILES uca-kiro-camera.h DESTINATION ${KIROCS_BINDIR})
install(TARGETS uca-kiro-camera LIBRARY DESTINATION ${LIBUCA_LIBDIR}/uca)
