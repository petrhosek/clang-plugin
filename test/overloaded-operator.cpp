// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 | FileCheck %s

class A {
public:
// CHECK: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  int operator+(int);
};

class B {
public:
// CHECK-NOT: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  B& operator=(B other);
// CHECK-NOT: [[@LINE+1]]:3: error: [system-c++] Operator overloading is disallowed 'int (int)'
  B& operator=(B&& other);
};
