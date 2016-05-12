// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s

class A {
 public:
  A(int value) : val(value) {}

  int do_A() { return val; }
 private:
  int val;
};

class B : public virtual A {
// CHECK-PLUGIN: error: [system-c++] Virtual inheritance is disallowed
// CHECK-PLUGIN-NEXT: class B : public virtual A {
 public:
  B() : A(0) {}
  int do_B() { return 1 + do_A(); }
};

class C : public virtual A {
// CHECK-PLUGIN: error: [system-c++] Virtual inheritance is disallowed
// CHECK-PLUGIN-NEXT: class C : public virtual A {
 public:
  C() : A(0) {}
  int do_C() { return 2 + do_A(); }
};

class D : public B, public C{
 public:
  D(int value) : A(value), B(), C() {}
// CHECK-PLUGIN: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-PLUGIN-NEXT:  D(int value) : A(value), B(), C() {}
// CHECK-PLUGIN: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-PLUGIN-NEXT:  D(int value) : A(value), B(), C() {}

  int do_D() { return do_A() + do_B() + do_C(); }
};

int main(void) {
  A *a = new A(0);
  B *b = new B();
// CHECK-PLUGIN: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-PLUGIN-NEXT:  B *b = new B();
  C *c = new C();
// CHECK-PLUGIN: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-PLUGIN-NEXT:  C *c = new C();
  D *d = new D(0);
// CHECK-PLUGIN: error: [system-c++] Constructing a class which inherits a virtual base class is disallowed
// CHECK-PLUGIN-NEXT:  D *d = new D(0);
  return 0;
}

// CHECK-NOT: {{.*}}error{{.*}}
