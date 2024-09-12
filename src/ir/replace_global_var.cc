/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file src/ir/replace_global_var.cc
 * \brief IRModule transform to replace GlobalVar instances across any IR type.
 */

#include <tvm/ir/replace_global_var.h>

#include <vector>

namespace tvm {
namespace transform {

IRModule ReplaceGlobalVar(IRModule mod, Map<GlobalVar, GlobalVar> replacements) {
  std::vector<GlobalVar> to_remove;
  IRModule updates;

  const auto& vtable = GlobalVarReplacer::vtable();

  for (const auto& [old_gvar, old_func] : mod->functions) {
    auto new_gvar = replacements.Get(old_gvar).value_or(old_gvar);
    auto new_func = vtable(old_func, replacements);

    if (!new_gvar.same_as(old_gvar)) {
      to_remove.push_back(old_gvar);
    }
    if (!old_gvar.same_as(new_gvar) || !old_func.same_as(new_func)) {
      updates->Add(new_gvar, new_func);
    }
  }

  if (to_remove.size() || updates->functions.size()) {
    auto write_ptr = mod.CopyOnWrite();
    for (const auto& old_gvar : to_remove) {
      write_ptr->Remove(old_gvar);
    }
    write_ptr->Update(updates);
  }
  return mod;
}

TVM_REGISTER_GLOBAL("transform.ReplaceGlobalVar").set_body_typed(ReplaceGlobalVar);

}  // namespace transform
}  // namespace tvm