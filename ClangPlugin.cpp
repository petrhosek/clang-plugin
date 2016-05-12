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
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/MultiplexConsumer.h"

#include <string>
#include <vector>

using namespace clang;

template class llvm::Registry<ClangPluginCheck>;

ClangPluginCheck::~ClangPluginCheck() {}

namespace {

using namespace clang::ast_matchers;

class ClangPluginFactory {
public:
  typedef std::vector<std::unique_ptr<ClangPluginCheck>> CheckList;

  std::unique_ptr<ASTConsumer> makeASTConsumer() {
    for (ClangPluginRegistry::iterator it = ClangPluginRegistry::begin(),
                                       ie = ClangPluginRegistry::end();
         it != ie; ++it) {
      std::unique_ptr<ClangPluginCheck> Check = it->instantiate();
      Check->add(Finder);
      Checks.emplace_back(std::move(Check));
    }
    return Finder.newASTConsumer();
  }

private:
  CheckList Checks;
  MatchFinder Finder;
};

ClangPluginFactory Factory;

class ClangPluginAction : public PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &Compiler, llvm::StringRef InFile) override {
    return Factory.makeASTConsumer();
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
LLVM_EXPORT_REGISTRY(ClangPluginRegistry)
#endif
