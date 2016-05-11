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

namespace {

using namespace clang::ast_matchers;

class NoThreadLocalDeclCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    DiagnosticsEngine &Diagnostics = Result.Context->getDiagnostics();

    if (const VarDecl *D = Result.Nodes.getNodeAs<VarDecl>("decl")) {
      unsigned ID = Diagnostics.getDiagnosticIDs()->getCustomDiagID(
        DiagnosticIDs::Error, "[system-c++] Thread local storage is disallowed");
      Diagnostics.Report(D->getLocStart(), ID);
    }
  }
};

NoThreadLocalDeclCallback NoThreadLocalDecl;

}  // namespace

void ThreadLocalAddMatchers(MatchFinder &Finder) {
  // Using thread-local storage is disallowed.
  Finder.addMatcher(varDecl(hasThreadStorageDuration()).bind("decl"), &NoThreadLocalDecl);
}
