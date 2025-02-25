//===-- abi_x86.cpp - x86 ABI description -----------------------*- C++ -*-===//
//
//                         LDC – the LLVM D compiler
//
// This file is distributed under the BSD-style LDC license:
//
// Copyright (c) 2007-2012 LDC Team.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of the LDC Team nor the names of its contributors may be
//       used to endorse or promote products derived from this software without
//       specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//===----------------------------------------------------------------------===//
//
// The ABI implementation used for 32 bit x86 targets.
//
//===----------------------------------------------------------------------===//


struct ABI_x86Layout : AbiLayout {

STATIC_INLINE bool is_complex_type(jl_datatype_t *dt)
{
    static jl_sym_t *Complex_sym = NULL;
    if (Complex_sym == NULL)
        Complex_sym = jl_symbol("Complex");
    return jl_is_datatype(dt) && dt->name->name == Complex_sym && dt->name->module == jl_base_module;
}

inline bool is_complex64(jl_datatype_t *dt) const
{
    return is_complex_type(dt) && jl_tparam0(dt) == (jl_value_t*)jl_float32_type;
}

inline bool is_complex128(jl_datatype_t *dt) const
{
    return is_complex_type(dt) && jl_tparam0(dt) == (jl_value_t*)jl_float64_type;
}

bool use_sret(jl_datatype_t *dt, LLVMContext &ctx) override
{
    size_t size = jl_datatype_size(dt);
    if (size == 0)
        return false;
    if (is_complex64(dt) || (jl_is_primitivetype(dt) && size <= 8))
        return false;
    return true;
}

bool needPassByRef(jl_datatype_t *dt, AttrBuilder &ab, LLVMContext &ctx, Type *Ty) override
{
    size_t size = jl_datatype_size(dt);
    if (is_complex64(dt) || is_complex128(dt) || (jl_is_primitivetype(dt) && size <= 8))
        return false;
#if JL_LLVM_VERSION < 120000
        ab.addAttribute(Attribute::ByVal);
#else
        ab.addByValAttr(Ty);
#endif
    return true;
}

Type *preferred_llvm_type(jl_datatype_t *dt, bool isret, LLVMContext &ctx) const override
{
    if (!isret)
        return NULL;
    // special case Complex{Float32} as a return type
    if (is_complex64(dt))
        return T_int64;
    return NULL;
}

};
