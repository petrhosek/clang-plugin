// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s


class AdderClass {
public:
  AdderClass(int value) {
    val = value;
  }

  AdderClass operator+=(int rhs) {
// CHECK-PLUGIN: {{.*}}error: [system-c++] Operator overloading is disallowed 'AdderClass (int)'
// CHECK-PLUGIN-NEXT:   AdderClass operator+=(int rhs) {
    val += rhs;
    return val;
  }
private:
  int val;
};

int main(void) {
  AdderClass a(0);
  a += 1;
}

// CHECK-NOT: {{.*}}error{{.*}}
