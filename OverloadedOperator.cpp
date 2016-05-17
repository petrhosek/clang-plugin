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
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include <string>

using namespace clang;

namespace clang {
namespace ast_matchers {

AST_MATCHER(CXXMethodDecl, hasOverloadedOperator) {
  if (Node.isCopyAssignmentOperator() || Node.isMoveAssignmentOperator()) {
    return false;
  }
  return Node.isOverloadedOperator();
}

} // namespace ast_matchers
} // namespace clang

namespace {

using namespace clang::ast_matchers;

class NoOverloadedOperatorCallback : public MatchFinder::MatchCallback {
public:
  void run(const MatchFinder::MatchResult &Result) override {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXMethodDecl *D =
            Result.Nodes.getNodeAs<CXXMethodDecl>("decl")) {
      QualType T = D->getType();
      SourceLocation Loc = D->getLocStart();

      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
          DiagnosticIDs::Error,
          "[system-c++] Operator overloading is disallowed %0");
      Diagnostics.Report(Loc, ID) << T;
    }
  }
};

NoOverloadedOperatorCallback NoOverloadedOperator;

class NoOverloadedOperatorCheck : public ClangPluginCheck {
public:
  void add(ast_matchers::MatchFinder &Finder) override {
    Finder.addMatcher(cxxMethodDecl(hasOverloadedOperator()).bind("decl"),
                      &NoOverloadedOperator);
  }
};

} // namespace

static ClangPluginRegistry::Add<NoOverloadedOperatorCheck>
    X("no-overloaded-operator", "Disallow C++ operator overloading");
