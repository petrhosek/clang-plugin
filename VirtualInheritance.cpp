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

AST_MATCHER(CXXRecordDecl, hasVirtualBaseClass) {
  return Node.hasDefinition() && (Node.getNumVBases() != 0);
}

}
}

namespace {

using namespace clang::ast_matchers;

class NoVirtualInheritanceDeclCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXRecordDecl *D = Result.Nodes.getNodeAs<CXXRecordDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Virtual inheritance is disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
    }
  }
};

class NoVirtualInheritanceStmtCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXConstructExpr *S = Result.Nodes.getNodeAs<CXXConstructExpr>("stmt")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Constructing a class which inherits a virtual base class is disallowed");
      Diagnostics.Report(S->getLocStart(), ID);
    }
  }
};

NoVirtualInheritanceDeclCallback NoVirtualInheritanceDecl;
NoVirtualInheritanceStmtCallback NoVirtualInheritanceStmt;

}  // namespace

void VirtualInheritanceAddMatchers(MatchFinder &Finder) {
  // Defining classes using virtual inheritance is disallowed.
  Finder.addMatcher(cxxRecordDecl(hasVirtualBaseClass()).bind("decl"), &NoVirtualInheritanceDecl);
  // Calling constructors of classes using virtual inheritance is disallowed.
  // To clarify, this matcher finds, in order:
  //  1) Calls to Constructors,
  //  2) The declarations of those constructors,
  //  3) The classes those constructors belong to,
  //  4) And matches the classes which have virtual base classes.
  Finder.addMatcher(
      cxxConstructExpr(hasDeclaration(
        cxxConstructorDecl(ofClass(
          cxxRecordDecl(hasVirtualBaseClass()))))).bind("stmt"), &NoVirtualInheritanceStmt);
}
