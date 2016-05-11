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

AST_MATCHER(CXXMethodDecl, hasOverloadedOperator) {
  if (Node.isCopyAssignmentOperator() || Node.isMoveAssignmentOperator()) {
    return false;
  }
  return Node.isOverloadedOperator();
}

AST_MATCHER(ParmVarDecl, hasDefaultArgument) {
  return Node.hasDefaultArg();
}

AST_MATCHER(CXXRecordDecl, hasVirtualBaseClass) {
  return Node.hasDefinition() && (Node.getNumVBases() != 0);
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
        DiagnosticIDs::Error, "[system-c++] operator overloading is disallowed %0");
      Diagnostics.Report(Loc, ID) << T;
    }
  }
};

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

class NoStaticallyConstructedObjectsCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const VarDecl *D = Result.Nodes.getNodeAs<VarDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Statically constructed objects are disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
    }
  }
};


NoOperatorOverloadingCallback NoOperatorOverloading;

NoDefaultParametersStmtCallback NoDefaultParametersStmt;
NoDefaultParametersDeclCallback NoDefaultParametersDecl;

NoVirtualInheritanceDeclCallback NoVirtualInheritanceDecl;
NoVirtualInheritanceStmtCallback NoVirtualInheritanceStmt;

NoStaticallyConstructedObjectsCallback NoStaticallyConstructedObjects;

class CheckRegistry {
public:
  CheckRegistry() {
    Finder.addMatcher(cxxMethodDecl(hasOverloadedOperator()).bind("decl"), &NoOperatorOverloading);

    // Calling a function which uses default arguments is disallowed.
    Finder.addMatcher(cxxDefaultArgExpr().bind("stmt"), &NoDefaultParametersStmt);
    // Declaring default parameters is disallowed.
    Finder.addMatcher(parmVarDecl(hasDefaultArgument()).bind("decl"), &NoDefaultParametersDecl);

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

    // Constructing objects which are stored statically is disallowed.
    Finder.addMatcher(
        varDecl(allOf(
            hasStaticStorageDuration(),
            hasDescendant(cxxConstructExpr()))).bind("decl"), &NoStaticallyConstructedObjects);
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
