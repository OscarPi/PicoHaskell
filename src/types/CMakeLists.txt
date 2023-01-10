add_library(types INTERFACE)
target_include_directories(types INTERFACE include)
target_link_libraries(types INTERFACE parser)
target_sources(types INTERFACE types.cpp)
