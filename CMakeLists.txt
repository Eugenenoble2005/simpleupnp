project(simpleupnp)

include_directories(src/includes)

add_executable(simpleupnp src/main.cc src/server/SSDPServer.cc)


