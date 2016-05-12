// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s

class Base_A {
  public:
   virtual int foo() {
    return 0;
   }
};

class Base_B {
  public:
   virtual int bar() {
    return 0;
   }
};

class Base_A_child : public Base_A {
  public:
   virtual int baz() {
    return 0;
   }
};

class Interface_A {
  public:
   virtual int foo() = 0;
};

class Interface_B {
  public:
   virtual int bar() = 0;
};

class Interface_C {
  public:
   virtual int blat() = 0;
};

class Interface_A_with_member {
  public:
   virtual int foo() = 0;
   int val = 0;
};

class Interface_with_A_Parent : public Base_A {
 public:
  virtual int baz() = 0;
};

// Inherits from multiple concrete classes.
class Bad_Child1 : public Base_A, Base_B {};
// CHECK-PLUGIN: {{.*}}error: [system-c++] Inheriting multiple classes which aren't pure virtual is disallowed
// CHECK-PLUGIN-NEXT: class Bad_Child1 : public Base_A, Base_B {};

class Bad_Child2 : public Base_A, Interface_A_with_member {
  virtual int foo() override { return 0; }
};
// CHECK-PLUGIN: {{.*}}error: [system-c++] Inheriting multiple classes which aren't pure virtual is disallowed
// CHECK-PLUGIN-NEXT: class Bad_Child2 : public Base_A, Interface_A_with_member {

class Bad_Child3: public Interface_with_A_Parent, Base_B {
  virtual int baz() override { return 0; }
};
// CHECK-PLUGIN: {{.*}}error: [system-c++] Inheriting multiple classes which aren't pure virtual is disallowed
// CHECK-PLUGIN-NEXT: class Bad_Child3: public Interface_with_A_Parent, Base_B {

// Easy cases of single inheritance
class Simple_Child1 : public Base_A {};
class Simple_Child2 : public Interface_A {
  virtual int foo() override { return 0; }
};

// Valid uses of multiple inheritance
class Good_Child1 : public Interface_A, Interface_B {
  virtual int foo() override { return 0; }
  virtual int bar() override { return 0; }
};
class Good_Child2 : public Base_A, Interface_B {
  virtual int bar() override { return 0; }
};
class Good_Child3 : public Base_A_child, Interface_C, Interface_B {
  virtual int bar() override { return 0; }
  virtual int blat() override { return 0; }
};


int main(void) {
  Bad_Child1 a;
  Bad_Child2 b;
  Bad_Child3 c;

  Simple_Child1 d;
  Simple_Child2 e;

  Good_Child1 f;
  Good_Child2 g;
  Good_Child3 h;
  return 0;
}
