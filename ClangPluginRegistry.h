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

#ifndef LLVM_CLANG_PLUGIN_CLANGPLUGINREGISTRY_H
#define LLVM_CLANG_PLUGIN_CLANGPLUGINREGISTRY_H

#include "ClangPluginCheck.h"
#include "llvm/Support/Registry.h"

// Instantiated in ClangPlugin.cpp.
extern template class llvm::Registry<clang::ClangPluginCheck>;

namespace clang {

/// The frontend plugin registry.
typedef llvm::Registry<ClangPluginCheck> ClangPluginRegistry;

} // end namespace clang

#endif
