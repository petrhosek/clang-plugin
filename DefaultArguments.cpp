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

#include "Matchers.h"

#include <string>

using namespace clang;

namespace clang {
namespace ast_matchers {

AST_MATCHER(ParmVarDecl, hasDefaultArgument) {
    return Node.hasDefaultArg();
}

}
}

namespace {

using namespace clang::ast_matchers;

class NoDefaultParametersStmtCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXDefaultArgExpr *S = Result.Nodes.getNodeAs<CXXDefaultArgExpr>("stmt")) {
      unsigned ErrorID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Calling functions which use default arguments is disallowed");
      unsigned NoteID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Note, "[system-c++] The default parameter was declared here:");

      Diagnostics.Report(S->getUsedLocation(), ErrorID);
      Diagnostics.Report(S->getParam()->getLocStart(), NoteID);
    }
  }
};

class NoDefaultParametersDeclCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const ParmVarDecl *D = Result.Nodes.getNodeAs<ParmVarDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Declaring functions which use default arguments is disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
    }
  }
};

NoDefaultParametersStmtCallback NoDefaultParametersStmt;
NoDefaultParametersDeclCallback NoDefaultParametersDecl;

}  // namespace

void DefaultArgumentsAddMatchers(MatchFinder &Finder) {
  // Calling a function which uses default arguments is disallowed.
  Finder.addMatcher(cxxDefaultArgExpr().bind("stmt"), &NoDefaultParametersStmt);
  // Declaring default parameters is disallowed.
  Finder.addMatcher(parmVarDecl(hasDefaultArgument()).bind("decl"), &NoDefaultParametersDecl);
}
