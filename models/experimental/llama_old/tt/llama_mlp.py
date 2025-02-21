# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

import torch
from torch import nn
import tt_lib

from models.helper_funcs import Linear as TTLinear
from models.utility_functions import torch_to_tt_tensor_rm


class TtLlamaMLP(nn.Module):
    def __init__(
        self,
        device,
        state_dict,
        base_url,
        layer_num,
        hidden_size: int,
        intermediate_size: int,
        hidden_act: str,
    ):
        super().__init__()

        self.state_dict = state_dict
        self.device = device
        self.hidden_size = hidden_size
        self.intermediate_size = intermediate_size
        self.bias = None

        self.out_gate_proj = torch_to_tt_tensor_rm(
            self.state_dict[f"{base_url}.{layer_num}.mlp.gate_proj.weight"], self.device
        )
        self.out_down_proj = torch_to_tt_tensor_rm(
            self.state_dict[f"{base_url}.{layer_num}.mlp.down_proj.weight"], self.device
        )
        self.out_up_proj = torch_to_tt_tensor_rm(
            self.state_dict[f"{base_url}.{layer_num}.mlp.up_proj.weight"], self.device
        )

        if hidden_act == "silu":  # silu
            self.act_fn = tt_lib.tensor.silu

        self.gate_proj_linear = TTLinear(
            self.out_gate_proj.get_legacy_shape()[-1],
            self.out_gate_proj.get_legacy_shape()[-2],
            self.out_gate_proj,
            self.bias,
        )
        self.up_proj_linear = TTLinear(
            self.out_up_proj.get_legacy_shape()[-1],
            self.out_up_proj.get_legacy_shape()[-2],
            self.out_up_proj,
            self.bias,
        )
        self.down_proj_linear = TTLinear(
            self.out_down_proj.get_legacy_shape()[-1],
            self.out_down_proj.get_legacy_shape()[-2],
            self.out_down_proj,
            self.bias,
        )

    def forward(self, x):
        # gate proj
        gate = self.gate_proj_linear(x)
        # apply silu activation function
        gate = self.act_fn(gate)

        # up proj
        up = self.up_proj_linear(x)

        # product
        prod = tt_lib.tensor.mul(gate, up)

        # down
        hidden_states = self.down_proj_linear(prod)

        # return TT Tensor
        return hidden_states
