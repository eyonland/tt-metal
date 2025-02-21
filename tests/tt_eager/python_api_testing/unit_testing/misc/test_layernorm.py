# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

from loguru import logger

import pytest
import torch

import tt_lib as ttl

from models.utility_functions import pad_by_zero, torch2tt_tensor, comp_pcc


def ref_layernorm(x, gamma, beta, eps):
    return torch.nn.functional.layer_norm(x, x.shape[-1:], gamma, beta, eps)


def ref_rmsnorm(x, gamma, beta, eps):
    return x * torch.rsqrt(x.pow(2).mean(-1, keepdim=True) + eps) * gamma + beta


def run_layernorm_mix_precision_tests(test_id, in_dtype, cb_dtype, in0_mem_config, out_mem_config, device):
    epsf = 1e-2

    test_dims = ((1, 9, 384, 1024),)
    for test_shape in test_dims:
        in0 = torch.rand(test_shape) * 2 - 0.95
        in0_t = torch2tt_tensor(in0, device, tt_memory_config=in0_mem_config, tt_dtype=in_dtype)

        if test_id <= 5:
            in1 = torch.rand(test_shape) * 2 - 0.8
            in1_t = torch2tt_tensor(in1, device, tt_memory_config=in0_mem_config, tt_dtype=in_dtype)

        if test_id % 3 == 0:
            gamma = torch.ones(test_shape[3])
            beta = torch.zeros(test_shape[3])
        if test_id % 3 == 1:
            gamma = torch.rand(test_shape[3]) * 2 - 1
            beta = torch.zeros(test_shape[3])
        if test_id % 3 == 2:
            gamma = torch.rand(test_shape[3]) * 2 - 1
            beta = torch.rand(test_shape[3]) * 2.0 - 1.1

        gamma_t = pad_by_zero(gamma, device, in0_mem_config, cb_dtype)[0]
        beta_t = pad_by_zero(beta, device, in0_mem_config, cb_dtype)[0]

        if test_id == 0:
            ttz = ttl.tensor.add_layernorm(
                in0_t,
                in1_t,
                epsf,
                output_mem_config=out_mem_config,
            )
        if test_id == 1:
            ttz = ttl.tensor.add_layernorm(
                in0_t,
                in1_t,
                epsf,
                gamma_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 2:
            ttz = ttl.tensor.add_layernorm(
                in0_t,
                in1_t,
                epsf,
                gamma_t,
                beta_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 3:
            ttz = ttl.tensor.add_rmsnorm(
                in0_t,
                in1_t,
                epsf,
                output_mem_config=out_mem_config,
            )
        if test_id == 4:
            ttz = ttl.tensor.add_rmsnorm(
                in0_t,
                in1_t,
                epsf,
                gamma_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 5:
            ttz = ttl.tensor.add_rmsnorm(
                in0_t,
                in1_t,
                epsf,
                gamma_t,
                beta_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 6:
            ttz = ttl.tensor.layernorm(
                in0_t,
                epsf,
                output_mem_config=out_mem_config,
            )
        if test_id == 7:
            ttz = ttl.tensor.layernorm(
                in0_t,
                epsf,
                gamma_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 8:
            ttz = ttl.tensor.layernorm(
                in0_t,
                epsf,
                gamma_t,
                beta_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 9:
            ttz = ttl.tensor.rmsnorm(
                in0_t,
                epsf,
                output_mem_config=out_mem_config,
            )
        if test_id == 10:
            ttz = ttl.tensor.rmsnorm(
                in0_t,
                epsf,
                gamma_t,
                output_mem_config=out_mem_config,
            )
        if test_id == 11:
            ttz = ttl.tensor.rmsnorm(
                in0_t,
                epsf,
                gamma_t,
                beta_t,
                output_mem_config=out_mem_config,
            )

        tt_got_back = ttz.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

        pt_in = in0 + in1 if test_id <= 5 else in0
        if test_id <= 2 or 6 <= test_id <= 8:
            ref_fn = ref_layernorm
        else:
            ref_fn = ref_rmsnorm

        ref_lnorm = ref_fn(pt_in, gamma.flatten(), beta.flatten(), epsf)

        passing, output = comp_pcc(ref_lnorm, tt_got_back)

        assert passing, output


@pytest.mark.parametrize(
    "out_mem_config",
    (ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.L1),),
    ids=[
        "in0_L1",
    ],
)
@pytest.mark.parametrize(
    "in0_mem_config",
    (ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.L1),),
    ids=[
        "in0_L1",
    ],
)
@pytest.mark.parametrize(
    "cb_dtype",
    (ttl.tensor.DataType.BFLOAT16,),
    ids=["BFLOAT16"],
)
@pytest.mark.parametrize(
    "in_dtype",
    (
        ttl.tensor.DataType.BFLOAT16,
        ttl.tensor.DataType.BFLOAT8_B,
    ),
    ids=["BFLOAT16", "BFLOAT8_B"],
)
@pytest.mark.parametrize(
    "test_id",
    (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
    ids=[
        "add_LN",
        "add_LN_G",
        "add_LN_GB",
        "add_RMSN",
        "add_RMSN_G",
        "add_RMSN_GB",
        "LN",
        "LN_G",
        "LN_GB",
        "RMSN",
        "RMSN_G",
        "RMSN_GB",
    ],
)
def test_layernorm_mix_precision(test_id, in_dtype, cb_dtype, in0_mem_config, out_mem_config, device):
    run_layernorm_mix_precision_tests(test_id, in_dtype, cb_dtype, in0_mem_config, out_mem_config, device)
