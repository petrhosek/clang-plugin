// Check that no errors occur when compiling without the plugin.
// RUN: %clangxx -std=c++11 -fsyntax-only  %s 2>&1 | FileCheck %s -allow-empty \
// RUN:   --check-prefix=CHECK-NO-PLUGIN
// CHECK-NO-PLUGIN-NOT: {{.*}}error{{.*}}

// "not" is used here because compiling with our plugin SHOULD result in an
// error (and "not" of an error implies success).
// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang \
// RUN:   %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 \
// RUN:   | FileCheck %s

// CHECK: [[@LINE+1]]:9: error: [system-c++] Declaring functions which use default arguments is disallowed
int foo(int value = 5) { return value; }

int bar(int value) { return value; }

int main(void) {
  // CHECK: [[@LINE+1]]:3: error: [system-c++] Calling functions which use  default arguments is disallowed
  foo();
  // CHECK: note: [system-c++] The default parameter was declared here:
  // CHECK-NEXT: int foo(int value = 5) { return value; }
  foo(0);
  bar(0);
}
