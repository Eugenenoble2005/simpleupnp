project(simpleupnp)

include_directories(src/includes/ src/includes/helpers)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/desc.xml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(simpleupnp src/main.cc src/server/SSDPServer.cc src/helpers/uuid_generator.cc src/server/HTTPServer.cc)


