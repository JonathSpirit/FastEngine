/*
 * Copyright 2024 Guillaume Guillet
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

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    #pragma GCC diagnostic ignored "-Wparentheses"
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif //__GNUC__

#define VMA_IMPLEMENTATION
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VMA_NULLABLE
#define VMA_NOT_NULL
#define VMA_NULLABLE_NON_DISPATCHABLE
#include "vk_mem_alloc.h"

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif //__GNUC__
