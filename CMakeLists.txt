create_subdirectory_options(CLANG_PLUGIN TOOL)

set(CLANG_PLUGIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CLANG_PLUGIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${LLVM_BUILD_DIR}/share/llvm/cmake")

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
if (APPLE)
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else (UNIX)
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
endif ()

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
  MultipleInheritance.cpp
  TrailingReturn.cpp
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

if (CMAKE_CFG_INTDIR STREQUAL ".")
  set(LLVM_BUILD_MODE ".")
else ()
  set(LLVM_BUILD_MODE "%(build_mode)s")
endif ()

string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} CLANG_TOOLS_DIR ${LLVM_RUNTIME_OUTPUT_INTDIR}) 

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/test/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/test/lit.site.cfg
  )

option(CLANG_PLUGIN_TEST_USE_VG "Run Clang Plugin tests under Valgrind" OFF)
if(CLANG_PLUGIN_TEST_USE_VG)
  set(CLANG_PLUGIN_TEST_EXTRA_ARGS ${CLANG_PLUGIN_TEST_EXTRA_ARGS} "--vg")
endif ()

set(CLANG_PLUGIN_TEST_DEPS
  # Base line deps.
  FileCheck count not

  clang
	libclangPlugin
  )

add_custom_target(clang-plugin-test-depends DEPENDS ${CLANG_PLUGIN_TEST_DEPS})
set_target_properties(clang-plugin-test-depends PROPERTIES FOLDER "Clang Plugin tests")

add_lit_testsuite(check-clang-plugin "Running the Clang Plugin regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}/test/
  DEPENDS ${CLANG_PLUGIN_TEST_DEPS}
  ARGS ${CLANG_PLUGIN_TEST_EXTRA_ARGS}
  )
set_target_properties(check-clang-plugin PROPERTIES FOLDER "Clang Plugin tests")
