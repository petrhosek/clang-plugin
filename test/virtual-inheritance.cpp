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
  A(int value) : val(value) {}

  int do_A() { return val; }
 private:
  int val;
};

class B : public virtual A {
// CHECK: [[@LINE-1]]:1: error: [system-c++] Virtual inheritance is disallowed
// CHECK-NEXT: class B : public virtual A {
 public:
  B() : A(0) {}
  int do_B() { return 1 + do_A(); }
};

class C : public virtual A {
// CHECK: [[@LINE-1]]:1: error: [system-c++] Virtual inheritance is disallowed
// CHECK-NEXT: class C : public virtual A {
 public:
  C() : A(0) {}
  int do_C() { return 2 + do_A(); }
};

class D : public B, public C {
// CHECK: [[@LINE-1]]:1: error: [system-c++] Virtual inheritance is disallowed
// CHECK-NEXT: class D : public B, public C {
 public:
  D(int value) : A(value), B(), C() {}
// CHECK: [[@LINE-1]]:28: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-NEXT:  D(int value) : A(value), B(), C() {}
// CHECK: [[@LINE-3]]:33: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-NEXT:  D(int value) : A(value), B(), C() {}

  int do_D() { return do_A() + do_B() + do_C(); }
};

int main(void) {
  A *a = new A(0);
  B *b = new B();
// CHECK: [[@LINE-1]]:14: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-NEXT:  B *b = new B();
  C *c = new C();
// CHECK: [[@LINE-1]]:14: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-NEXT:  C *c = new C();
  D *d = new D(0);
// CHECK: [[@LINE-1]]:14: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-NEXT:  D *d = new D(0);
  return 0;
}
