// Check that no errors occur when compiling without the plugin.
// RUN: %clangxx -std=c++11 -fsyntax-only  %s 2>&1 | FileCheck %s -allow-empty \
// RUN:   --check-prefix=CHECK-NO-PLUGIN
// CHECK-NO-PLUGIN-NOT: {{.*}}error{{.*}}

// "not" is used here because compiling with our plugin SHOULD result in an
// error (and "not" of an error implies success).
// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang \
// RUN:   %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 \
// RUN:   | FileCheck %s

int main(void) {
  thread_local int foo;
// CHECK: [[@LINE-1]]:3: error: [system-c++] Thread local storage is disallowed
// CHECK-NEXT:  thread_local int foo;
  extern thread_local int bar;
// CHECK: [[@LINE-1]]:3: error: [system-c++] Thread local storage is disallowed
// CHECK-NEXT:  extern thread_local int bar;
  int baz;
  return 0;
}
