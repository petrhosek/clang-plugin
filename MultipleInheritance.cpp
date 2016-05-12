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

// Performance is not ideal.
// TODO(smklein): Memoization.
bool isCurrentClassInterface(const CXXRecordDecl *Node) {
  // Interfaces should have no fields.
  if (!Node->field_empty()) {
    return false;
  }

  // Interfaces should have exclusively pure methods.
  for (auto method : Node->methods()) {
    if (method->isUserProvided() && !method->isPure()) {
      return false;
    }
  }

  return true;
}

bool isInterface(const CXXRecordDecl *Node) {
  for (const auto &I : Node->bases()) {
    const RecordType *Ty = I.getType()->getAs<RecordType>();
    // Conservatively return false (not an interface) to prompt an error.
    if (!Ty)
      return false;
    CXXRecordDecl *Base = cast_or_null<CXXRecordDecl>(Ty->getDecl()->getDefinition());
    if (!Base)
      return false;
    if (!isInterface(Base))
      return false;
  }
  return isCurrentClassInterface(Node);
}

AST_MATCHER(CXXRecordDecl, hasMultipleConcreteBaseClasses) {
  if (!Node.hasDefinition())
    return false;

  int numConcrete = 0;
  for (const auto &I : Node.bases()) {
    const RecordType *Ty = I.getType()->getAs<RecordType>();
    if (!Ty)
      return false;
    CXXRecordDecl *Base = cast_or_null<CXXRecordDecl>(Ty->getDecl()->getDefinition());
    if (!Base)
      return false;
    if (!isInterface(Base))
      numConcrete++;
  }
  return numConcrete > 1;
}

}
}

namespace {

using namespace clang::ast_matchers;

class LimitMultipleInheritanceDeclCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXRecordDecl *D = Result.Nodes.getNodeAs<CXXRecordDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Multiple Inheritance of concrete classes is disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
      // TODO(smklein): Add note explaining "interface" vs "concrete", how many
      // allowed.
    }
  }
};

// TODO(smklein): One for decl, one for stmt.

LimitMultipleInheritanceDeclCallback LimitMultipleInheritanceDecl;

}  // namespace

void MultipleInheritanceAddMatchers(MatchFinder &Finder) {
  // Matches classes that inherit from a pure interface, at any point...
  //Finder.addMatcher(cxxRecordDecl(isDerivedFrom(cxxRecordDecl(hasOnlyPureFunctions()))).bind("decl"), &LimitMultipleInheritanceDecl);

  // Match declarations which inherit multiple concrete base classes.
  Finder.addMatcher(cxxRecordDecl(hasMultipleConcreteBaseClasses()).bind("decl"), &LimitMultipleInheritanceDecl);
}
