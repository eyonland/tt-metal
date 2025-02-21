# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

import torch
from torch import nn
import tt_lib
import ttnn
import copy
from models.utility_functions import torch2tt_tensor, pad_by_zero, tt2torch_tensor, nearest_32
from models.demos.llama2_70b.tt.llama_model_optimized import TtLlamaModel_optimized as TtLlamaModel
from pathlib import Path


class TtLlamaModelForGeneration:
    def __init__(self, reference_model, pcie_devices, n_devices, model_config, n_layers, batch, emulated=False):
        ## Get devices
        devices = pcie_devices[:n_devices]
        if emulated:
            device = devices[0]
            devices = [device for _ in range(n_devices)]  # Emulate fracturing on N chips
            cache_path = Path("/proj_sw/user_dev/llama-data-cache/weights-cache")
        else:
            cache_path = model_config["DEFAULT_CACHE_PATH"]

        ## Get state dict
        reference_model.eval()
        state_dict = reference_model.state_dict()
        base_url = "layers"
        configuration = copy.deepcopy(reference_model.params)
        n_heads = configuration.n_heads
        n_kv_heads = configuration.n_kv_heads
        hidden_dim = configuration.dim
        head_dim = hidden_dim // n_heads

        # Cache Weights setup
        if n_layers == None:
            n_layers = 80

        if n_layers >= 40 and n_devices == 4:
            n_layers_per_group = 20
            assert n_layers % n_layers_per_group == 0
        else:
            n_layers_per_group = None

        # TT model -------------------------------------------------------------
        self.tt_model = TtLlamaModel(
            devices,
            state_dict,
            base_url,
            n_layers,
            model_config,
            configuration,
            batch,
            n_layers_per_group=n_layers_per_group,
            emulated=emulated,
            cache_path=cache_path,
        )
        self.params = configuration
        self.devices = devices

        for device in devices:
            tt_lib.device.Synchronize(device)

        del reference_model
        del state_dict

    def forward(self, tokens: torch.Tensor, start_pos: int, *args, **kwargs):
        tt_inp_emb, start_pos, rot_mat, attn_mask = self.tt_model.prepare_inputs(tokens, start_pos)

        tt_out = self.tt_model(
            tt_inp_emb,
            rot_mat,
            start_pos,
            attn_mask,
        )

        del tt_inp_emb
        del rot_mat
        del attn_mask

        for device in self.devices:
            tt_lib.device.Synchronize(device)

        assert isinstance(tt_out, list)  # tt_out should be fractured on N devices
        assert len(tt_out) == len(self.devices)

        tt_outs = [tt2torch_tensor(o) for o in tt_out]
        tt_out = torch.cat(tt_outs, dim=-1)
        tt_out = tt_out[..., : self.params.vocab_size]
        tt_out = tt_out.permute(2, 1, 0, 3).squeeze().unsqueeze(1)  # [batch, 1, vocab_size]
        tt_out = tt_out.float()
        return tt_out
