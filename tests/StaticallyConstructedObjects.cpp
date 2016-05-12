// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s


class AdderClass {
public:
  AdderClass(int value1, int value2) {
    val = value1 + value2;
  }
  constexpr AdderClass(int value) : val(value) {}
private:
  int val;
};

struct mystruct {
  int a;
};

struct mystruct_with_method {
  int a;
  void b() {
    a++;
  }
};

struct static_mystruct_container {
  int a;
  static int b;
  static struct mystruct c;
};

int main(void) {
  static AdderClass a(1, 2);
// CHECK-PLUGIN: error: [system-c++] Statically constructed objects are disallowed
// CHECK-PLUGIN-NEXT:  static AdderClass a(1, 2);

  // Allowed (constexpr ctor)
  static AdderClass b(0);

  static struct mystruct c;
// CHECK-PLUGIN: error: [system-c++] Statically constructed objects are disallowed
// CHECK-PLUGIN-NEXT:  static struct mystruct c;
  static struct mystruct_with_method d;
// CHECK-PLUGIN: error: [system-c++] Statically constructed objects are disallowed
// CHECK-PLUGIN-NEXT:  static struct mystruct_with_method d;

  static int e;
  struct static_mystruct_container f;
}

// CHECK-NOT: {{.*}}error{{.*}}
