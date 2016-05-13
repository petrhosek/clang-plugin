// Check that no errors occur when compiling without the plugin.
// RUN: %clangxx -std=c++11 -fsyntax-only  %s 2>&1 | FileCheck %s -allow-empty \
// RUN:   --check-prefix=CHECK-NO-PLUGIN
// CHECK-NO-PLUGIN-NOT: {{.*}}error{{.*}}

// "not" is used here because compiling with our plugin SHOULD result in an
// error (and "not" of an error implies success).
// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang \
// RUN:   %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 \
// RUN:   | FileCheck %s

class A {
public:
  // CHECK: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  int operator+(int);
};

class B {
public:
  // CHECK-NOT: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  B &operator=(B other);
  // CHECK-NOT: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  B &operator=(B &&other);
};
