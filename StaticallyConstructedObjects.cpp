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

using namespace clang::ast_matchers;

class NoStaticallyConstructedObjectsCallback : public MatchFinder::MatchCallback {
public:
  void run(const MatchFinder::MatchResult &Result) override {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const VarDecl *D = Result.Nodes.getNodeAs<VarDecl>("decl")) {
      unsigned ErrorID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Statically constructed objects are disallowed");
      unsigned NoteID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Note, "[system-c++] If possible, use a constexpr constructor instead");
      Diagnostics.Report(D->getLocStart(), ErrorID);
      Diagnostics.Report(D->getLocStart(), NoteID);
    }
  }
};

NoStaticallyConstructedObjectsCallback NoStaticallyConstructedObjects;

class NoStaticallyConstructedObjectsCheck : public ClangPluginCheck {
public:
  void add(ast_matchers::MatchFinder &Finder) override {
    // Constructing objects which are stored statically is disallowed.
    Finder.addMatcher(
        varDecl(allOf(
            // Match statically stored objects...
            hasStaticStorageDuration(),
            // ... which have C++ constructors...
            hasDescendant(cxxConstructExpr(unless(
            // ... that are not constexpr.
                hasDeclaration(cxxConstructorDecl(isConstexpr()))
            ))))).bind("decl"), &NoStaticallyConstructedObjects);
  }
};

}  // namespace

static ClangPluginRegistry::Add<NoStaticallyConstructedObjectsCheck> X(
    "no-statically-constructed-objects",
    "Disallow statically constructed C++ objects");
