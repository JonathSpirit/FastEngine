/*
 * Copyright 2026 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_C_SHADER_HPP_INCLUDED
#define _FGE_C_SHADER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/manager/shader_manager.hpp"

namespace fge
{

/**
 * \class Shader
 * \ingroup graphics
 * \brief This class is a wrapper for the shader manager to allow easy manipulation
 */
class FGE_API Shader : public manager::BaseDataAccessor<
                               manager::GlobalDataAccessorManagerInfo<shader::ShaderManager, &shader::gManager>,
                               manager::DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER>
{
public:
    using BaseDataAccessor::BaseDataAccessor;
    using BaseDataAccessor::operator=;
};

} // namespace fge

#endif // _FGE_C_SHADER_HPP_INCLUDED
