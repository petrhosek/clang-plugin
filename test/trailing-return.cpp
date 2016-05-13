// Check that no errors occur when compiling without the plugin.
// RUN: %clangxx -std=c++11 -fsyntax-only  %s 2>&1 | FileCheck %s -allow-empty \
// RUN:   --check-prefix=CHECK-NO-PLUGIN
// CHECK-NO-PLUGIN-NOT: {{.*}}error{{.*}}

// "not" is used here because compiling with our plugin SHOULD result in an
// error (and "not" of an error implies success).
// RUN: not %clangxx -std=c++11 -fsyntax-only -Xclang -load -Xclang \
// RUN:   %clang_plugin -Xclang -add-plugin -Xclang system-c++ %s 2>&1 \
// RUN:   | FileCheck %s

int add_one(const int arg){
  return arg;
}

auto get_add_one() -> int(*)(const int) {
// CHECK: [[@LINE-1]]:1: error: [system-c++] Trailing returns are disallowed
// CHECK-NEXT: auto get_add_one() -> int(*)(const int) {
    return add_one;
}

int main(void) {
  get_add_one()(5);

  return 0;
}
