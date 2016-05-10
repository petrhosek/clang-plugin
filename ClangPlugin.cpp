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


#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/MultiplexConsumer.h"

#include <string>

using namespace clang;

namespace clang {
namespace ast_matchers {

AST_MATCHER(FunctionDecl, hasOverloadedOperator) {
  return Node.isOverloadedOperator();
}

}
}

namespace {

using namespace clang::ast_matchers;

class NoOperatorOverloadingCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXMethodDecl *D = Result.Nodes.getNodeAs<CXXMethodDecl>("decl")) {
      QualType T = D->getType();
      SourceLocation Loc = D->getLocStart();

      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "operator overloading is disallowed %0");
      Diagnostics.Report(Loc, ID) << T;
    }
  }
};

NoOperatorOverloadingCallback Action;

class CheckRegistry {
public:
  CheckRegistry() {
    Finder.addMatcher(cxxMethodDecl(hasOverloadedOperator()).bind("decl"), &Action);
  }

  std::unique_ptr<ASTConsumer> makeASTConsumer() {
    return Finder.newASTConsumer();
  }

private:
  MatchFinder Finder;
};

CheckRegistry Registry;

class ClangPluginAction : public PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &Compiler, llvm::StringRef InFile) override {
    return Registry.makeASTConsumer();
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &ArgsArr) override {
    return true;
  }
};

}

static FrontendPluginRegistry::Add<ClangPluginAction> X(
    "system-c++",
    "System C++ dialect of the C++ programming language");

#ifdef LLVM_EXPORT_REGISTRY
LLVM_EXPORT_REGISTRY(FrontendPluginRegistry)
#endif
