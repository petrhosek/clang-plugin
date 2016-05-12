// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s


int foo(int value=5) {
// CHECK-PLUGIN: {{.*}}error: [system-c++] Declaring functions which use default arguments is disallowed
// CHECK-PLUGIN-NEXT: int foo(int value=5) {
  return value;
}

int bar(int value) {
  return value;
}

int main(void) {
  foo();
// CHECK-PLUGIN: error: [system-c++] Calling functions which use default arguments is disallowed
// CHECK-PLUGIN-NEXT:   foo();
// CHECK-PLUGIN-NEXT: note: [system-c++] The default parameter was declared here:
// CHECK-PLUGIN-NEXT: int foo(int value=5) {
  foo(0);
  bar(0);
}

// CHECK-NOT: {{.*}}error{{.*}}
