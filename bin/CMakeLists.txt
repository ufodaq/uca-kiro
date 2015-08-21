include_directories(${KIROCS_SOURCE_DIR})
link_directories(${KIROCS_BINARY_DIR})

add_executable(kiro-camera-server kiro-camera-server.c)
target_link_libraries(kiro-camera-server ${KIROCS_DEPS})

#add_executable(test-camera-server test-server.c)
#target_link_libraries(test-camera-server ${KIROCS_DEPS})

install(TARGETS kiro-camera-server RUNTIME DESTINATION ${KIROCS_BINDIR})
