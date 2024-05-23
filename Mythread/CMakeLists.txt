set(CMAKE_CXX_STANDARD 17)

add_library(Mythread worker.cpp payload.cpp)

target_link_libraries(Mythread PRIVATE rocksdb)