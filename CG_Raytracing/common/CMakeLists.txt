add_library(common STATIC "./include/Utility.hpp" "./include/config.hpp" "./src/Utility.cpp" "./include/stb_image.h")
target_include_directories(common PUBLIC "./include")
