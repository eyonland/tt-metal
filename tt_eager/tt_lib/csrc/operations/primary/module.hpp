// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "transformers/module.hpp"
#include "tt_dnn/op_library/bmm/bmm_op.hpp"
#include "tt_dnn/op_library/moreh_clip_grad_norm/moreh_clip_grad_norm_op.hpp"
#include "tt_dnn/op_library/layernorm/layernorm_op.hpp"
#include "tt_dnn/op_library/moreh_adam/moreh_adam_op.hpp"
#include "tt_dnn/op_library/moreh_layernorm/moreh_layernorm_op.hpp"
#include "tt_dnn/op_library/moreh_layernorm_backward/moreh_layernorm_backward_op.hpp"
#include "tt_dnn/op_library/moreh_bmm/moreh_bmm_op.hpp"
#include "tt_dnn/op_library/moreh_bmm_backward/moreh_bmm_backward_op.hpp"
#include "tt_dnn/op_library/moreh_linear/moreh_linear_op.hpp"
#include "tt_dnn/op_library/moreh_linear_backward/moreh_linear_backward_op.hpp"
#include "tt_dnn/op_library/moreh_matmul/moreh_matmul_op.hpp"
#include "tt_dnn/op_library/moreh_matmul_backward/moreh_matmul_backward_op.hpp"
#include "tt_dnn/op_library/moreh_nll_loss/moreh_nll_loss_op.hpp"
#include "tt_dnn/op_library/moreh_nll_loss_backward/moreh_nll_loss_backward_op.hpp"
#include "tt_dnn/op_library/moreh_norm/moreh_norm_op.hpp"
#include "tt_dnn/op_library/moreh_norm_backward/moreh_norm_backward_op.hpp"
#include "tt_dnn/op_library/moreh_softmax/moreh_softmax_op.hpp"
#include "tt_dnn/op_library/moreh_softmax_backward/moreh_softmax_backward_op.hpp"
#include "tt_dnn/op_library/softmax/softmax_op.hpp"
#include "tt_dnn/op_library/moreh_sum/moreh_sum_op.hpp"
#include "tt_dnn/op_library/moreh_sum_backward/moreh_sum_backward_op.hpp"
#include "tt_dnn/op_library/moreh_cumsum/moreh_cumsum_op.hpp"
#include "tt_dnn/op_library/moreh_arange/moreh_arange_op.hpp"
#include "tt_dnn/op_library/moreh_sgd/moreh_sgd_op.hpp"
#include "tt_dnn/op_library/groupnorm/groupnorm_op.hpp"
#include "tt_dnn/op_library/moreh_groupnorm/moreh_groupnorm_op.hpp"
#include "tt_dnn/op_library/moreh_groupnorm_backward/moreh_groupnorm_backward_op.hpp"
#include "tt_dnn/op_library/moreh_mean/moreh_mean_op.hpp"
#include "tt_dnn/op_library/moreh_mean_backward/moreh_mean_backward_op.hpp"
#include "tt_dnn/op_library/moreh_getitem/moreh_getitem_op.hpp"

namespace py = pybind11;

namespace tt {
namespace operations {
namespace primary {

void py_module(py::module& m_primary) {
    auto m_transformers = m_primary.def_submodule("transformers", "Primary transformers operations");
    transformers::py_module(m_transformers);

    py::class_<MatmulProgramConfig>(m_primary, "MatmulProgramConfig")
        .def("__repr__", [](const MatmulProgramConfig& config) { return fmt::format("{}", config); });

    py::class_<MatmulDefaultProgramConfig>(m_primary, "MatmulDefaultProgramConfig")
        .def(py::init<>())
        .def("__repr__", [](const MatmulDefaultProgramConfig& config) { return fmt::format("{}", config); });

    py::class_<MatmulMultiCoreReuseProgramConfig>(m_primary, "MatmulMultiCoreReuseProgramConfig")
        .def(
            py::init<CoreCoord, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>(),
            py::kw_only(),
            py::arg("compute_with_storage_grid_size"),
            py::arg("in0_block_w").noconvert(),
            py::arg("out_subblock_h").noconvert(),
            py::arg("out_subblock_w").noconvert(),
            py::arg("per_core_M").noconvert(),
            py::arg("per_core_N").noconvert())
        .def("__repr__", [](const MatmulMultiCoreReuseProgramConfig& config) { return fmt::format("{}", config); });

    py::class_<MatmulMultiCoreReuseMultiCastProgramConfig>(m_primary, "MatmulMultiCoreReuseMultiCastProgramConfig")
        .def(
            py::init<
                CoreCoord,
                std::size_t,
                std::size_t,
                std::size_t,
                std::size_t,
                std::size_t,
                bool,
                std::optional<UnaryWithParam>>(),
            py::kw_only(),
            py::arg("compute_with_storage_grid_size"),
            py::arg("in0_block_w").noconvert(),
            py::arg("out_subblock_h").noconvert(),
            py::arg("out_subblock_w").noconvert(),
            py::arg("per_core_M").noconvert(),
            py::arg("per_core_N").noconvert(),
            py::arg("transpose_mcast").noconvert(),
            py::arg("fused_activation"))
        .def_readwrite("fused_activation", &MatmulMultiCoreReuseMultiCastProgramConfig::fused_activation)
        .def("__repr__", [](const MatmulMultiCoreReuseMultiCastProgramConfig& config) {
            return fmt::format("{}", config);
        });

    py::class_<MatmulMultiCoreReuseMultiCast1DProgramConfig>(m_primary, "MatmulMultiCoreReuseMultiCast1DProgramConfig")
        .def(
            py::init<
                CoreCoord,
                std::size_t,
                std::size_t,
                std::size_t,
                std::size_t,
                std::size_t,
                bool,
                std::optional<UnaryWithParam>,
                bool>(),
            py::kw_only(),
            py::arg("compute_with_storage_grid_size"),
            py::arg("in0_block_w").noconvert(),
            py::arg("out_subblock_h").noconvert(),
            py::arg("out_subblock_w").noconvert(),
            py::arg("per_core_M").noconvert(),
            py::arg("per_core_N").noconvert(),
            py::arg("fuse_batch").noconvert(),
            py::arg("fused_activation"),
            py::arg("mcast_in0").noconvert())
        .def_readwrite("fused_activation", &MatmulMultiCoreReuseMultiCast1DProgramConfig::fused_activation)
        .def("__repr__", [](const MatmulMultiCoreReuseMultiCast1DProgramConfig& config) {
            return fmt::format("{}", config);
        });

    m_primary.def(
        "get_mcast_1d_config",
        &bmm_op_utils::get_mcast_1d_config,
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::arg("fuse_batch").noconvert() = false,
        py::arg("fused_activation") = std::nullopt,
        py::arg("mcast_in0").noconvert() = true,
        py::arg("out_sharded").noconvert() = false);

    // TODO(arakhmati):
    // delete
    // redundant
    // matmul
    // overrides
    // by
    // figuring
    // out
    // how
    // to
    // pass
    // in
    // MatmulProgramConfig
    // (which
    // is
    // a
    // std::variant)
    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           const MatmulDefaultProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(input_tensor_a, input_tensor_b, program_config, out_mem_config, output_dtype, compute_kernel_config, untilize_out);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("program_config").noconvert() = MatmulDefaultProgramConfig(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "program_config",    "",                                                       "MatmulDefaultProgramConfig",          "",                                                               "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           const MatmulMultiCoreReuseProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(input_tensor_a, input_tensor_b, program_config, out_mem_config, output_dtype, compute_kernel_config, untilize_out);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("program_config").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "program_config",    "",                                                       "MatmulMultiCoreReuseProgramConfig",          "",                                                               "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           std::optional<const Tensor> bias,
           const MatmulDefaultProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(
                input_tensor_a, input_tensor_b, bias, program_config, out_mem_config, output_dtype, compute_kernel_config, untilize_out);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("program_config").noconvert() = MatmulDefaultProgramConfig(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "bias",              "Bias to add",                                            "Tensor",                                     "Tensor of shape [1, 1, 1, N]",                                   "Yes"
                "program_config",    "",                                                       "MatmulDefaultProgramConfig", "",                                                               "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           std::optional<const Tensor> bias,
           const MatmulMultiCoreReuseProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(
                input_tensor_a,
                input_tensor_b,
                bias,
                program_config,
                out_mem_config,
                output_dtype,
                compute_kernel_config,
                untilize_out
                );
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("program_config").noconvert() = MatmulDefaultProgramConfig(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "bias",              "Bias to add",                                            "Tensor",                                     "Tensor of shape [1, 1, 1, N]",                                   "Yes"
                "program_config",    "",                                                       "MatmulDefaultProgramConfig", "",                                                               "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           std::optional<const Tensor> bias,
           const MatmulMultiCoreReuseMultiCastProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(
                input_tensor_a, input_tensor_b, bias, program_config, out_mem_config, output_dtype, compute_kernel_config, untilize_out);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("program_config").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "bias",              "Bias to add",                                            "Tensor",                                     "Tensor of shape [1, 1, 1, N]",                                   "Yes"
                "program_config",    "",                                                       "MatmulMultiCoreReuseMultiCastProgramConfig", "",                                                               "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           std::optional<const Tensor> bias,
           const MatmulMultiCoreReuseMultiCast1DProgramConfig& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config,
           const bool untilize_out
           ) {
            return matmul(
                input_tensor_a, input_tensor_b, bias, program_config, out_mem_config, output_dtype, compute_kernel_config, untilize_out);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("program_config").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "bias",              "Bias to add",                                            "Tensor",                                     "Tensor of shape [1, 1, 1, N]",                                   "Yes"
                "program_config",    "",                                                       "MatmulMultiCoreReuseMultiCast1DProgramConfig", "",                                                             "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    m_primary.def(
        "matmul_1d",
        [](const Tensor& input_tensor_a,
           const Tensor& input_tensor_b,
           std::optional<const Tensor> bias,
           const std::optional<MatmulMultiCoreReuseMultiCast1DProgramConfig>& program_config,
           const MemoryConfig& out_mem_config,
           std::optional<DataType> output_dtype,
           std::optional<DeviceComputeKernelConfig> compute_kernel_config
           ) {
            return matmul_1d(
                input_tensor_a, input_tensor_b, bias, program_config, out_mem_config, output_dtype, compute_kernel_config);
        },
        py::arg("input_tensor_a").noconvert(),
        py::arg("input_tensor_b").noconvert(),
        py::kw_only(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("program_config").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("compute_kernel_config").noconvert() = std::nullopt,
        R"doc(
            Perform a matrix multiplication ``input_tensor_a x input_tensor_b``.

            .. csv-table::
                :header: "Argument", "Description", "Data type", "Valid range", "Required"

                "input_tensor_a",    "First tensor to multiply",                               "Tensor",                                     "Tensor of shape [B_a, C_a, M, K]",                               "Yes"
                "input_tensor_b",    "Second tensor to multiply",                              "Tensor",                                     "Tensor of shape [B_b, C_b, K, N]",                               "Yes"
                "bias",              "Bias to add",                                            "Tensor",                                     "Tensor of shape [1, 1, 1, N]",                                   "Yes"
                "program_config",    "",                                                       "MatmulMultiCoreReuseMultiCast1DProgramConfig", "Config will be automatically determined if not passed",        "Yes"
                "output_mem_config", "Layout of tensor in TT Accelerator device memory banks", "MemoryConfig",                               "Default is interleaved in DRAM",                                 "No"
                "output_dtype",      "Output Data Type",                                       "DataType",                                   "By default it will be set to the data type of `input_tensor_a`", "No"
        )doc");

    py::class_<LayerNormDefaultProgramConfig>(m_primary, "LayerNormDefaultProgramConfig")
        .def(py::init<>());

    py::class_<LayerNormInterleavedMultiCoreProgramConfig>(m_primary, "LayerNormInterleavedMultiCoreProgramConfig")
        .def(
            py::init<MathFidelity, DataType, DataType>(),
            py::kw_only(),
            py::arg("math_fidelity").noconvert() = MathFidelity::HiFi4,
            py::arg("im_data_format").noconvert(),
            py::arg("out_data_format").noconvert()
        );

    py::class_<LayerNormShardedMultiCoreProgramConfig>(m_primary, "LayerNormShardedMultiCoreProgramConfig")
        .def(
            py::init<CoreCoord, std::size_t, std::size_t, std::size_t, MathFidelity, DataType, DataType, bool>(),
            py::kw_only(),
            py::arg("compute_with_storage_grid_size"),
            py::arg("subblock_w").noconvert(),
            py::arg("block_h").noconvert(),
            py::arg("block_w").noconvert(),
            py::arg("math_fidelity").noconvert() = MathFidelity::HiFi4,
            py::arg("im_data_format").noconvert(),
            py::arg("out_data_format").noconvert(),
            py::arg("inplace").noconvert())
        .def(
            "__repr__", [](const LayerNormShardedMultiCoreProgramConfig& config) { return fmt::format("{}", config); });

    m_primary.def(
        "layernorm",
        tt::operations::primary::layernorm,
        py::arg("input").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("program_config").noconvert() = LayerNormDefaultProgramConfig{},
        R"doc(
            Performs a layernorm operation on the last tensor dimension with optional fused with post-multiplication and addition via W-bcast.
        )doc");

    m_primary.def(
        "add_layernorm",
        tt::operations::primary::add_layernorm,
        py::arg("a").noconvert(),
        py::arg("b").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("program_config").noconvert() = LayerNormDefaultProgramConfig{},
        R"doc(
            Performs a layernorm(a+b)*gamma + beta operation.
        )doc");

    m_primary.def(
        "rmsnorm",
        tt::operations::primary::rmsnorm,
        py::arg("input").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("program_config").noconvert() = LayerNormDefaultProgramConfig{},
        R"doc(
            Performs a rmsnorm operation on the last tensor dimension with optional fused with post-multiplication and addition via W-bcast.
        )doc");

    m_primary.def(
        "add_rmsnorm",
        tt::operations::primary::add_rmsnorm,
        py::arg("a").noconvert(),
        py::arg("b").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("program_config").noconvert() = LayerNormDefaultProgramConfig{},
        R"doc(
            Performs a rmsnorm(a+b)*gamma + beta operation.
        )doc");

    // moreh_adam
    m_primary.def(
        "moreh_adam",
        &moreh_adam,
        py::arg("param").noconvert(),
        py::arg("grad").noconvert(),
        py::arg("exp_avg").noconvert(),
        py::arg("exp_avg_sq").noconvert(),
        py::arg("lr").noconvert(),
        py::arg("beta1").noconvert(),
        py::arg("beta2").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("weight_decay").noconvert(),
        py::arg("step").noconvert(),
        py::arg("amsgrad").noconvert(),
        py::arg("max_exp_avg_sq").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_adam operation.
        )doc");

    // moreh_clip_grad_norm
    m_primary.def(
        "moreh_clip_grad_norm_",
        &moreh_clip_grad_norm,
        py::arg("inputs").noconvert(),
        py::arg("max_norm").noconvert(),
        py::arg("norm_type").noconvert() = 2.0f,
        py::arg("error_if_nonfinite").noconvert() = false,
        py::kw_only(),
        py::arg("total_norm").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_clip_grad_norm operation.
    )doc");

    m_primary.def(
        "moreh_bmm",
        &moreh_bmm,
        py::arg("input").noconvert(),
        py::arg("mat2").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_bmm operation.
    )doc");
    m_primary.def(
        "moreh_bmm_backward",
        &moreh_bmm_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input").noconvert(),
        py::arg("mat2").noconvert(),
        py::arg("input_grad").noconvert() = std::nullopt,
        py::arg("mat2_grad").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_bmm_backward operation.
    )doc");
    m_primary.def(
        "moreh_linear",
        &moreh_linear,
        py::arg("input").noconvert(),
        py::arg("weight").noconvert(),
        py::arg("bias").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_linear operation.
    )doc");
    m_primary.def(
        "moreh_linear_backward",
        &moreh_linear_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input").noconvert(),
        py::arg("weight").noconvert(),
        py::arg("input_grad").noconvert() = std::nullopt,
        py::arg("weight_grad").noconvert() = std::nullopt,
        py::arg("bias_grad").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_linear_backward operation.
    )doc");

    // moreh_matmul
    m_primary.def(
        "moreh_matmul",
        &moreh_matmul,
        py::arg("input_a").noconvert(),
        py::arg("input_b").noconvert(),
        py::kw_only(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("transpose_input_a").noconvert() = false,
        py::arg("transpose_input_b").noconvert() = false,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a moreh_matmul operation.");

    // moreh_matmul_backward
    m_primary.def(
        "moreh_matmul_backward",
        &moreh_matmul_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input_a").noconvert(),
        py::arg("input_b").noconvert(),
        py::arg("input_a_grad").noconvert() = std::nullopt,
        py::arg("input_b_grad").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        "Performs a moreh_matmul_backward operation.
    )doc");

    // moreh_nll_loss
    m_primary.def(
        "moreh_nll_loss",
        &moreh_nll_loss,
        py::arg("input_tensor").noconvert(),
        py::arg("target_tensor").noconvert(),
        py::arg("weight_tensor").noconvert(),
        py::arg("divisor_tensor").noconvert(),
        py::arg("output_tensor").noconvert(),
        py::arg("ignore_index").noconvert(),
        py::arg("reduction_mean").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a nll_loss operation. Returns an output tensor.");

    // moreh_nll_loss_backward
    m_primary.def(
        "moreh_nll_loss_backward",
        &moreh_nll_loss_backward,
        py::arg("input_tensor").noconvert(),
        py::arg("target_tensor").noconvert(),
        py::arg("weight_tensor").noconvert(),
        py::arg("divisor_tensor").noconvert(),
        py::arg("output_grad_tensor").noconvert(),
        py::arg("input_grad_tensor").noconvert(),
        py::arg("ignore_index").noconvert(),
        py::arg("reduction_mean").noconvert(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a nll_loss_backward operation. Returns an output tensor.");

    // moreh_norm
    m_primary.def(
        "moreh_norm",
        &moreh_norm,
        py::arg("input").noconvert(),
        py::arg("p").noconvert() = 2.0f,
        py::arg("dim").noconvert() = std::nullopt,
        py::kw_only(),
        py::arg("output").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a moreh_norm operation.");

    // moreh_norm_backward
    m_primary.def(
        "moreh_norm_backward",
        &moreh_norm_backward,
        py::arg("input").noconvert(),
        py::arg("output").noconvert(),
        py::arg("output_grad").noconvert(),
        py::arg("p").noconvert() = 2.0f,
        py::kw_only(),
        py::arg("input_grad").noconvert() = std::nullopt,
        py::arg("input_grad_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a moreh_norm_backward operation.");

    m_primary.def(
        "moreh_layernorm",
        &moreh_layernorm,
        py::arg("input").noconvert(),
        py::arg("normalized_dims").noconvert(),
        py::arg("eps").noconvert() = 1e-5f,
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::kw_only(),
        py::arg("mean").noconvert() = std::nullopt,
        py::arg("rstd").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a moreh_layernorm operation.");
    m_primary.def(
        "moreh_layernorm_backward",
        &moreh_layernorm_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input").noconvert(),
        py::arg("mean").noconvert(),
        py::arg("rstd").noconvert(),
        py::arg("normalized_dims").noconvert(),
        py::kw_only(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("input_grad").noconvert() = std::nullopt,
        py::arg("gamma_grad").noconvert() = std::nullopt,
        py::arg("beta_grad").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a moreh_layernorm_backward operation.");
    // softmax
    m_primary.def(
        "softmax_in_place",
        &softmax_in_place,
        py::arg("input_tensor").noconvert(),
        py::arg("program_config").noconvert() = transformers::SoftmaxDefaultProgramConfig{},
        "Performs a softmax operation on the last tensor dimension. Returns a reference to the input tensor modified "
        "in place.");

    py::enum_<MorehSoftmaxOpParallelizationStrategy>(m_primary, "MorehSoftmaxOpParallelizationStrategy")
        .value("NONE", MorehSoftmaxOpParallelizationStrategy::NONE)
        .value("SMALL_W", MorehSoftmaxOpParallelizationStrategy::SMALL_W)
        .value("SMALL_H", MorehSoftmaxOpParallelizationStrategy::SMALL_H)
        .value("LARGE_W", MorehSoftmaxOpParallelizationStrategy::LARGE_W)
        .value("LARGE_H", MorehSoftmaxOpParallelizationStrategy::LARGE_H)
        .value("LARGE_C", MorehSoftmaxOpParallelizationStrategy::LARGE_C);
    py::enum_<MorehSoftmaxBackwardOpParallelizationStrategy>(m_primary, "MorehSoftmaxBackwardOpParallelizationStrategy")
        .value("NONE", MorehSoftmaxBackwardOpParallelizationStrategy::NONE)
        .value("SMALL_W", MorehSoftmaxBackwardOpParallelizationStrategy::SMALL_W)
        .value("SMALL_H", MorehSoftmaxBackwardOpParallelizationStrategy::SMALL_H)
        .value("LARGE_W", MorehSoftmaxBackwardOpParallelizationStrategy::LARGE_W)
        .value("LARGE_H", MorehSoftmaxBackwardOpParallelizationStrategy::LARGE_H)
        .value("LARGE_C", MorehSoftmaxBackwardOpParallelizationStrategy::LARGE_C);
    m_primary.def(
        "moreh_softmax",
        &moreh_softmax,
        py::arg("input_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a softmax operation. Returns an output tensor.");
    m_primary.def(
        "moreh_softmax_backward",
        &moreh_softmax_backward,
        py::arg("output_tensor").noconvert(),
        py::arg("output_grad_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("input_grad_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxBackwardOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a softmax backward operation. Returns an input grad tensor.");
    m_primary.def(
        "moreh_softmin",
        &moreh_softmin,
        py::arg("input_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a softmin operation. Returns an output tensor.");
    m_primary.def(
        "moreh_softmin_backward",
        &moreh_softmin_backward,
        py::arg("output_tensor").noconvert(),
        py::arg("output_grad_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("input_grad_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxBackwardOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a softmin backward operation. Returns an input grad tensor.");

    m_primary.def(
        "moreh_logsoftmax",
        &moreh_logsoftmax,
        py::arg("input_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a logsoftmax operation. Returns an output tensor.");

    m_primary.def(
        "moreh_logsoftmax_backward",
        &moreh_logsoftmax_backward,
        py::arg("output_tensor").noconvert(),
        py::arg("output_grad_tensor").noconvert(),
        py::arg("dim").noconvert(),
        py::arg("input_grad_tensor").noconvert() = std::nullopt,
        py::arg("strategy").noconvert() = MorehSoftmaxBackwardOpParallelizationStrategy::NONE,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a logsoftmax backward operation. Returns an input grad tensor.");

    m_primary.def(
        "moreh_sum",
        &moreh_sum,
        py::arg("input").noconvert(),
        py::arg("output").noconvert(),
        py::kw_only(),
        py::arg("dims").noconvert() = std::vector<int64_t>(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs sum operation. Returns an output tensor.");
    m_primary.def(
        "moreh_sum_backward",
        &moreh_sum_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input_grad").noconvert(),
        "Performs sum backward operation. Returns an input_grad tensor.");
    m_primary.def(
        "moreh_cumsum",
        &moreh_cumsum,
        py::arg("input").noconvert(),
        py::arg("output").noconvert(),
        py::kw_only(),
        py::arg("dim").noconvert(),
        "Performs cumsum operation. Returns an output tensor.");
    m_primary.def(
        "moreh_cumsum_backward",
        &moreh_cumsum_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input_grad").noconvert(),
        py::kw_only(),
        py::arg("dim").noconvert(),
        "Performs cumsum backward operation. Returns an input_grad tensor.");

    m_primary.def(
        "moreh_arange",
        &moreh_arange,
        py::arg("start"),
        py::arg("end"),
        py::arg("step"),
        py::arg("any").noconvert(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("untilize_out").noconvert() = false,
        py::arg("output_dtype").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs an arange operation. Returns an output tensor.");

    m_primary.def(
        "moreh_sgd",
        &moreh_sgd,
        py::arg("param_in").noconvert(),
        py::arg("grad").noconvert(),
        py::arg("momentum_buffer_in").noconvert() = std::nullopt,
        py::arg("param_out").noconvert(),
        py::arg("momentum_buffer_out").noconvert() = std::nullopt,
        py::arg("lr").noconvert(),
        py::arg("momentum").noconvert(),
        py::arg("dampening").noconvert(),
        py::arg("weight_decay").noconvert(),
        py::arg("nesterov").noconvert(),
        py::arg("momentum_initialized").noconvert(),
        "Performs a SGD operation.");

    py::class_<GroupNormShardedMultiCoreProgramConfig>(m_primary, "GroupNormShardedMultiCoreProgramConfig")
        .def(
            py::init<CoreCoord, MathFidelity, DataType, DataType, bool>(),
            py::kw_only(),
            py::arg("compute_with_storage_grid_size"),
            py::arg("math_fidelity").noconvert() = MathFidelity::HiFi4,
            py::arg("im_data_format").noconvert() = DataType::BFLOAT16,
            py::arg("out_data_format").noconvert() = DataType::BFLOAT16,
            py::arg("inplace").noconvert() = false
        );

    m_primary.def(
        "groupnorm",
        &groupnorm,
        py::arg("input").noconvert(),
        py::arg("num_groups").noconvert(),
        py::arg("eps").noconvert(),
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("program_config").noconvert() = GroupNormShardedMultiCoreProgramConfig{},
        R"doc(
            Performs a groupnorm operation, returna a output tensor the same shape as input.
        )doc");

    // moreh_groupnorm
    m_primary.def(
        "moreh_groupnorm",
        &moreh_groupnorm,
        py::arg("input").noconvert(),
        py::arg("num_groups").noconvert(),
        py::arg("eps").noconvert() = 1e-5f,
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("beta").noconvert() = std::nullopt,
        py::kw_only(),
        py::arg("are_needed_outputs").noconvert() = std::vector<bool>{true, false, false},
        py::arg("output").noconvert() = std::nullopt,
        py::arg("mean").noconvert() = std::nullopt,
        py::arg("rstd").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("mean_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("rstd_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        Performs a moreh_groupnorm operation.
    )doc");

    // moreh_groupnorm_backward
    m_primary.def(
        "moreh_groupnorm_backward",
        &moreh_groupnorm_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input").noconvert(),
        py::arg("mean").noconvert(),
        py::arg("rstd").noconvert(),
        py::arg("num_groups").noconvert(),
        py::kw_only(),
        py::arg("are_needed_outputs").noconvert() = std::vector<bool>{true, true, true},
        py::arg("gamma").noconvert() = std::nullopt,
        py::arg("input_grad").noconvert() = std::nullopt,
        py::arg("gamma_grad").noconvert() = std::nullopt,
        py::arg("beta_grad").noconvert() = std::nullopt,
        py::arg("input_grad_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("gamma_grad_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        py::arg("beta_grad_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        R"doc(
        Performs a moreh_groupnorm_backward operation.
    )doc");

    m_primary.def(
        "moreh_mean",
        &moreh_mean,
        py::arg("input").noconvert(),
        py::arg("output").noconvert(),
        py::kw_only(),
        py::arg("dims").noconvert() = std::vector<int64_t>(),
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs mean operation. Returns an output tensor.");
    m_primary.def(
        "moreh_mean_backward",
        &moreh_mean_backward,
        py::arg("output_grad").noconvert(),
        py::arg("input_grad").noconvert(),
        "Performs mean backward operation. Returns an input_grad tensor.");

    m_primary.def(
        "moreh_getitem",
        &moreh_getitem,
        py::arg("input_tensor").noconvert(),
        py::arg("index_tensors").noconvert(),
        py::arg("index_dims").noconvert(),
        py::arg("output_tensor").noconvert() = std::nullopt,
        py::arg("output_mem_config").noconvert() = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
        "Performs a getitem operation. Returns an output tensor.");
}

}  // namespace
   // primary
}  // namespace
   // operations
}  // namespace
   // tt
