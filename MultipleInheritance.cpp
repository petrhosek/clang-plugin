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

namespace {

// Contains the identity of each named CXXRecord as an interface.  This is used
// to memoize lookup speeds and improve performance from O(N^2) to O(N), where N
// is the number of classes.
llvm::StringMap<bool> *InterfaceMap;

// Adds a node (by name) to the interface map, if it was not present in the map
// previously.
void addNodeToInterfaceMap(const CXXRecordDecl *Node, bool isInterface) {
  StringRef Name = Node->getIdentifier()->getName();
  InterfaceMap->insert(std::make_pair(Name, isInterface));
}

// Returns "true" if the boolean "isInterface" has been set to the
// interface status of the current Node. Return "false" if the
// interface status for the current node is not yet known.
bool getInterfaceStatus(const CXXRecordDecl *Node, bool *isInterface) {
  StringRef Name = Node->getIdentifier()->getName();
  if (InterfaceMap->count(Name)) {
    *isInterface = InterfaceMap->lookup(Name);
    return true;
  }
  return false;
}

}  // namespace

namespace clang {
namespace ast_matchers {

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
  // Short circuit the lookup if we have analyzed this record before.
  bool previousIsInterfaceResult;
  if (getInterfaceStatus(Node, &previousIsInterfaceResult)) {
    return previousIsInterfaceResult;
  }

  // To be an interface, all base classes must be interfaces as well.
  for (const auto &I : Node->bases()) {
    const RecordType *Ty = I.getType()->getAs<RecordType>();
    assert(Ty && "RecordType of base class is unknown");
    CXXRecordDecl *Base = cast<CXXRecordDecl>(Ty->getDecl()->getDefinition());
    if (!isInterface(Base)) {
      addNodeToInterfaceMap(Node, false);
      return false;
    }
  }
  bool currentClassIsInterface = isCurrentClassInterface(Node);
  addNodeToInterfaceMap(Node, currentClassIsInterface);
  return currentClassIsInterface;
}

AST_MATCHER(CXXRecordDecl, hasMultipleConcreteBaseClasses) {
  if (!Node.hasDefinition())
    return false;

  int numConcrete = 0;
  for (const auto &I : Node.bases()) {
    const RecordType *Ty = I.getType()->getAs<RecordType>();
    assert(Ty && "RecordType of base class is unknown");
    CXXRecordDecl *Base = cast<CXXRecordDecl>(Ty->getDecl()->getDefinition());
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
  virtual void run(const MatchFinder::MatchResult &Result) override {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const CXXRecordDecl *D = Result.Nodes.getNodeAs<CXXRecordDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Inheriting multiple classes which aren't pure virtual is disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
    }
  }

  void onStartOfTranslationUnit() override {
    InterfaceMap = new llvm::StringMap<bool>;
  }
  void onEndOfTranslationUnit() override {
    delete InterfaceMap;
  }
};

// TODO(smklein): Add a matcher for statements.

LimitMultipleInheritanceDeclCallback LimitMultipleInheritanceDecl;

class MultipleInheritanceCheck : public ClangPluginCheck {
public:
  void add(ast_matchers::MatchFinder &Finder) override {
    // Match declarations which inherit multiple concrete base classes.
    Finder.addMatcher(cxxRecordDecl(hasMultipleConcreteBaseClasses()).bind("decl"), &LimitMultipleInheritanceDecl);
  }
};

}  // namespace

static ClangPluginRegistry::Add<MultipleInheritanceCheck> X(
    "limit-multiple-inheritance",
    "Limit usage of Multiple Inheritance");
