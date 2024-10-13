project(simpleupnp)

include_directories(src/includes/ src/includes/helpers)

add_executable(simpleupnp src/main.cc src/server/SSDPServer.cc src/helpers/uuid_generator.cc)


