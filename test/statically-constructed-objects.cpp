// Check that no errors occur when compiling without the plugin.
// RUN: %clangxx -std=c++11 -fsyntax-only  %s 2>&1 | FileCheck %s -allow-empty \
// RUN:   --check-prefix=CHECK-NO-PLUGIN
// CHECK-NO-PLUGIN-NOT: {{.*}}error{{.*}}

// "not" is used here because compiling with our plugin SHOULD result in an
// error (and "not" of an error implies success).
// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang \
// RUN:   %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 \
// RUN:   | FileCheck %s

class AdderClass {
public:
  AdderClass(int value1, int value2) : val(value1 + value2) {}
  constexpr AdderClass(int value) : val(value) {}

private:
  int val;
};

struct mystruct {
  int a;
};

struct mystruct_with_method {
  int a;
  void b() { a++; }
};

struct static_mystruct_container {
  int a;
  static int b;
  static struct mystruct c;
};

int main(void) {
  static AdderClass a(1, 2);
  // CHECK: [[@LINE-1]]:3: error: [system-c++] Statically constructed objects are disallowed
  // CHECK-NEXT:  static AdderClass a(1, 2);

  // Allowed (constexpr ctor)
  static AdderClass b(0);

  static struct mystruct c;
  // CHECK: [[@LINE-1]]:3: error: [system-c++] Statically constructed objects are disallowed
  // CHECK-NEXT:  static struct mystruct c;
  static struct mystruct_with_method d;
  // CHECK: [[@LINE-1]]:3: error: [system-c++] Statically constructed objects are disallowed
  // CHECK-NEXT:  static struct mystruct_with_method d;

  static int e;
  struct static_mystruct_container f;
}
