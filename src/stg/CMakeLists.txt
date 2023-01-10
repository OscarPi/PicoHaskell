add_library(stg INTERFACE)
target_include_directories(stg INTERFACE include)
target_link_libraries(stg INTERFACE parser types)
target_sources(stg INTERFACE stg.cpp)
