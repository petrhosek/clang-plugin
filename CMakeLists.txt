list(APPEND CMAKE_MODULE_PATH "${LLVM_BUILD_DIR}/share/llvm/cmake")

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")

include_directories("${CMAKE_SOURCE_DIR}/include"
                    "${CMAKE_SOURCE_DIR}/tools/clang/include"
                    "${CMAKE_BINARY_DIR}/include"
                    "${CMAKE_BINARY_DIR}/tools/clang/include")

link_directories("${CMAKE_BINARY_DIR}/lib"
                 "${CMAKE_BINARY_DIR}/tools/clang/lib")

set(plugin_sources
  OverloadedOperator.cpp
  DefaultArguments.cpp
  VirtualInheritance.cpp
  StaticallyConstructedObjects.cpp
  ThreadLocal.cpp
  ClangPlugin.cpp)

add_llvm_loadable_module(
  libclangPlugin ${plugin_sources}
  LINK_LIBS
  -lclangASTMatchers
)

add_dependencies(libclangPlugin clang)

install(TARGETS libclangPlugin
        LIBRARY DESTINATION lib
        COMPONENT clangPlugin OPTIONAL)
