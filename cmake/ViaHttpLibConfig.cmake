include(CMakeFindDependencyMacro)
find_dependency(Boost 1.51)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/ViaHttpLibTargets.cmake")
