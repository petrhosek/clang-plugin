// RUN: %clang_cpp -fsyntax-only -std=c++11 %s
// RUN:   | FileCheck %s

int add_one(const int arg){
  return arg;
}

auto get_add_one() -> int(*)(const int) {
    return add_one;
}
// CHECK-PLUGIN: {{.*}}error: [system-c++] Trailing returns are disallowed
// CHECK-PLUGIN-NEXT: auto get_add_one() -> int(*)(const int) {

int main(void) {
  get_add_one()(5);

  return 0;
}

// CHECK-NOT: {{.*}}error{{.*}}
