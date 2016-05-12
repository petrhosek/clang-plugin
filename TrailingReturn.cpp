// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ClangPluginCheck.h"
#include "ClangPluginRegistry.h"

#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <string>

using namespace clang;

namespace clang {
namespace ast_matchers {

AST_MATCHER(FunctionDecl, hasTrailingReturn) {
  const Type *T = Node.getType().getTypePtr();
  const FunctionProtoType *F = cast<FunctionProtoType>(T);
  return F->hasTrailingReturn();
}

}
}

namespace {

using namespace clang::ast_matchers;

class NoTrailingReturnCallback : public MatchFinder::MatchCallback {
public:
  void run(const MatchFinder::MatchResult &Result) override {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const Decl *D = Result.Nodes.getNodeAs<Decl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Trailing returns are disallowed");

      Diagnostics.Report(D->getLocStart(), ID);
    }
  }
};

NoTrailingReturnCallback NoTrailingReturn;

class NoTrailingReturnCheck : public ClangPluginCheck {
public:
  void add(ast_matchers::MatchFinder &Finder) override {
    // Functions which have trailing returns are disallowed.
    Finder.addMatcher(functionDecl(hasTrailingReturn()).bind("decl"), &NoTrailingReturn);
  }
};

}  // namespace

static ClangPluginRegistry::Add<NoTrailingReturnCheck> X(
    "no-trailing-return",
    "Disallow C++ trailing return");
