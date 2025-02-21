// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "llk_math_eltwise_unary_sfpu_init.h"
#include "llk_math_eltwise_unary_sfpu_0_param.h"
#include "ckernel_sfpu_rsqrt.h"

namespace ckernel {

// New LLK SFPU APIs

template <bool APPROXIMATE>
inline void llk_math_eltwise_unary_sfpu_rsqrt_init() {
    llk_math_eltwise_unary_sfpu_init<APPROXIMATE>();
}

template <bool APPROXIMATE, DstSync Dst = DstSync::SyncFull>
inline void llk_math_eltwise_unary_sfpu_rsqrt(uint dst_index, int vector_mode = (int)VectorMode::RC) {
    // APPROXIMATE = true -> approximate fast mode
    //               false -> high precision mode
    // The algorithm uses Newton's method based on no.of iteration better approximation can be calculated

    // if (APPROXIMATE) {
    //     llk_math_eltwise_unary_sfpu_0_param<APPROXIMATE, Dst>
    //                         (ckernel::sfpu::calculate_rsqrt<APPROXIMATE, 4, 10>,
    //                         ckernel::sfpu::calculate_rsqrt<APPROXIMATE, 4, 10>,
    //                         dst_index, vector_mode);
    // } else {
        llk_math_eltwise_unary_sfpu_0_param<APPROXIMATE, Dst>
                            (ckernel::sfpu::calculate_rsqrt<APPROXIMATE, 4, 25>,
                            ckernel::sfpu::calculate_rsqrt<APPROXIMATE, 4, 25>,
                            dst_index, vector_mode);
    // }
}

}
