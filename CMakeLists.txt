project(simpleupnp)

include_directories(src/server src/helpers)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/desc.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/content-directory-scpd.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/connection-manager-scpd.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(simpleupnp src/main.cc src/server/SSDPServer.cc src/helpers/uuid_generator.cc src/server/HTTPServer.cc src/server/UPNPDevice.cc src/helpers/logger.cc src/helpers/ipv4_address.cc)


