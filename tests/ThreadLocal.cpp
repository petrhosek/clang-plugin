// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s



int main(void) {
  thread_local int foo;
// CHECK-PLUGIN: error: [system-c++] Thread local storage is disallowed
// CHECK-PLUGIN-NEXT:  thread_local int foo;
  extern thread_local int bar;
// CHECK-PLUGIN: error: [system-c++] Thread local storage is disallowed
// CHECK-PLUGIN-NEXT:  extern thread_local int bar;
  int baz;
  return 0;
}

// CHECK-NOT: {{.*}}error{{.*}}
