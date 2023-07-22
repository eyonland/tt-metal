#pragma once

#include "tensor/tensor.hpp"
#include "tt_dnn/op_library/run_operation.hpp"

namespace tt {

namespace tt_metal {

// TODO: Accept parallelization
struct ConvOpParallelizationStrategy {
    enum Enum { MULTI_CORE = 0, MULTI_CORE_REUSE = 1, MULTI_CORE_REUSE_MCAST = 2, SINGLE_CORE = 3 };
    static const vector<Enum> all() { return { MULTI_CORE, MULTI_CORE_REUSE, MULTI_CORE_REUSE_MCAST, SINGLE_CORE }; }
};

struct Conv {
     // additional parameters
    const std::vector<int> conv_params;
    const uint32_t act_block_h_ntiles, act_block_w_ntiles, weight_block_w_ntiles, out_subblock_h_ntiles, out_subblock_w_ntiles, output_channels;

    Conv(uint32_t act_bh, uint32_t act_bw, uint32_t weight_bw, uint32_t out_sh, uint32_t out_sw, const std::vector<int>&c_params, uint32_t output_channels)
        : act_block_h_ntiles(act_bh),
          act_block_w_ntiles(act_bw),
          weight_block_w_ntiles(weight_bw),
          out_subblock_h_ntiles(out_sh),
          out_subblock_w_ntiles(out_sw),
          output_channels(output_channels),
          conv_params(c_params) {}

    operation::Hash compute_program_hash(const std::vector<Tensor> &input_tensors) const;
    void validate(const std::vector<Tensor>& input_tensors) const;
    std::vector<Shape> compute_output_shapes(const std::vector<Tensor>& input_tensors) const;
    std::vector<Tensor> create_output_tensors(const std::vector<Tensor>& input_tensors) const;
    operation::ProgramWithCallbacks create_program(const std::vector<Tensor>& input_tensors, std::vector<Tensor> &output_tensors) const;
    tt::stl::reflection::Attributes attributes() const;
};

Tensor conv(const Tensor& a, const Tensor &b, const vector<int> conv_params, uint32_t act_block_h_ntiles, uint32_t act_block_w_ntiles, uint32_t weight_block_w_ntiles,
             uint32_t out_subblock_h_ntiles, uint32_t out_subblock_w_ntiles, uint32_t output_channels);
operation::ProgramWithCallbacks conv_single_core(const Tensor& A, const Tensor& B, vector<int> conv_params, uint32_t act_block_h_ntiles, uint32_t act_block_w_ntiles, uint32_t weight_block_w_ntiles,
             uint32_t out_subblock_h_ntiles, uint32_t out_subblock_w_ntiles, uint32_t output_channels, Tensor& output); // Tilizes a, untilizes b

struct ConvWithAddressMap {

    // additional parameters
    const std::vector<int> conv_params;
    const uint32_t act_block_h_ntiles, act_block_w_ntiles, weight_block_w_ntiles, out_subblock_h_ntiles, out_subblock_w_ntiles, output_channels;

    ConvWithAddressMap(uint32_t act_bh, uint32_t act_bw, uint32_t weight_bw, uint32_t out_sh, uint32_t out_sw, const std::vector<int>&c_params, uint32_t output_channels)
        : act_block_h_ntiles(act_bh),
          act_block_w_ntiles(act_bw),
          weight_block_w_ntiles(weight_bw),
          out_subblock_h_ntiles(out_sh),
          out_subblock_w_ntiles(out_sw),
          output_channels(output_channels),
          conv_params(c_params) {}

    void validate(const std::vector<Tensor>& input_tensors) const;
    std::vector<Shape> compute_output_shapes(const std::vector<Tensor>& input_tensors) const;
    std::vector<Tensor> create_output_tensors(const std::vector<Tensor>& input_tensors) const;
    operation::ProgramWithCallbacks create_program(const std::vector<Tensor>& input_tensors, std::vector<Tensor> &output_tensors) const;
    tt::stl::reflection::Attributes attributes() const;
};

Tensor conv_with_address_map(const Tensor& a, const Tensor &b, const vector<int> conv_params, uint32_t act_block_h_ntiles, uint32_t act_block_w_ntiles, uint32_t weight_block_w_ntiles,
             uint32_t out_subblock_h_ntiles, uint32_t out_subblock_w_ntiles, uint32_t output_channels);
operation::ProgramWithCallbacks conv_with_address_map_single_core(const Tensor& A, const Tensor& B, vector<int> conv_params, uint32_t act_block_h_ntiles, uint32_t act_block_w_ntiles, uint32_t weight_block_w_ntiles,
             uint32_t out_subblock_h_ntiles, uint32_t out_subblock_w_ntiles, uint32_t output_channels, Tensor& output); // Tilizes a, untilizes b


}  // namespace tt_metal

}  // namespace tt
