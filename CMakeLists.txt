cmake_minimum_required(VERSION 3.10)

project(simpleupnp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

include_directories(src/server src/helpers)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/desc.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/content-directory-scpd.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/connection-manager-scpd.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/ms-media-registrar-scpd.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
find_package(tinyxml2 REQUIRED)

add_executable(
  simpleupnp
  src/main.cc
  src/server/SSDPServer.cc
  src/helpers/uuid_generator.cc
  src/server/HTTPServer.cc
  src/server/UPNPDevice.cc
  src/helpers/logger.cc
  src/helpers/ipv4_address.cc
  src/helpers/global.cc
  src/helpers/escape_xml.cc
  src/helpers/encode_file_path.cc
  src/server/ConnectionManager.cc
  src/server/ContentDirectory.cc)
target_link_libraries(simpleupnp tinyxml2)
