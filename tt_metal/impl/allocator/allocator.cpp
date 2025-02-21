// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "tt_metal/impl/allocator/allocator.hpp"
#include "tt_metal/impl/allocator/algorithms/free_list.hpp"
#include "tt_metal/impl/buffers/buffer.hpp"
#include "tt_metal/common/math.hpp"
#include "tt_metal/detail/util.hpp"
#include "tt_metal/hostdevcommon/common_runtime_address_map.h"
#include "third_party/magic_enum/magic_enum.hpp"

namespace tt {

namespace tt_metal {

namespace allocator {
#if defined(TRACY_ENABLE)
static char const *get_memory_pool_name(BufferType buffer_type) {
    switch (buffer_type) {
        case BufferType::DRAM: return "DRAM";
        case BufferType::L1: return "L1";
        case BufferType::SYSTEM_MEMORY: return "SYSTEM_MEMORY";
        default: return "UNKNOWN";
    }
}
#endif

void BankManager::init_allocator(uint64_t size_bytes, uint64_t offset) {
    this->allocator_ = std::make_unique<FreeList>(
        size_bytes,
        offset,
        this->min_allocation_size_bytes_,
        ADDRESS_ALIGNMENT,
        FreeList::SearchPolicy::FIRST
    );
}

void validate_num_banks(uint32_t num_banks, const BufferType &buffer_type) {
    bool is_pow2_num_banks = num_banks && (!(num_banks & (num_banks - 1)));
    // Dataflow API does not have a working implementation of generic modulo to determine bank_id for interleaved address gen
    // For non pow2 num banks, special cases need to be added to avoid falling back to generic implementation.
    // See https://github.com/tenstorrent-metal/tt-metal/issues/3321
    bool custom_mod_bank_id_calculation_exists = (num_banks == 12 or num_banks == 94 or num_banks == 124);
    bool valid_num_banks = (is_pow2_num_banks or custom_mod_bank_id_calculation_exists);
    if (not valid_num_banks) {
        TT_THROW("Invalid number of memory banks for {}. Num banks must be power of 2 or have a dedicated modulo implementation", magic_enum::enum_name(buffer_type));
    }
}

BankManager::BankManager(const BufferType &buffer_type, const std::vector<int64_t> &bank_offsets, uint64_t size_bytes, uint64_t alloc_offset) : buffer_type_(buffer_type) {
    unsigned int bank_id = 0;
    for (const auto bank_offset : bank_offsets) {
        this->bank_id_to_bank_offset_.insert({bank_id, bank_offset});
        bank_id++;
    }
    this->interleaved_address_limit_ = 0;
    validate_num_banks(this->bank_id_to_bank_offset_.size(), this->buffer_type_);
    this->init_allocator(size_bytes, alloc_offset);
}

BankManager::BankManager(const BufferType &buffer_type, const std::unordered_map<uint32_t, int64_t> &bank_id_to_bank_offset, uint64_t size_bytes, uint64_t interleaved_address_limit, uint64_t alloc_offset) : buffer_type_(buffer_type), bank_id_to_bank_offset_(bank_id_to_bank_offset), interleaved_address_limit_(interleaved_address_limit) {
    validate_num_banks(this->bank_id_to_bank_offset_.size(), this->buffer_type_);
    this->init_allocator(size_bytes, alloc_offset);
}

uint32_t BankManager::num_banks() const {
    return this->bank_id_to_bank_offset_.size();
}

uint32_t BankManager::bank_size() const {
    uint64_t max_size_bytes_u64 = this->allocator_->max_size_bytes();
    if (max_size_bytes_u64 > std::numeric_limits<uint32_t>::max()) {
        TT_THROW("Bank size {} overflows uint32_t", max_size_bytes_u64);
    }
    uint32_t max_size_bytes = (uint32_t)max_size_bytes_u64;
    return max_size_bytes;
}

int64_t BankManager::bank_offset(uint32_t bank_id) const {
    this->validate_bank_id(bank_id);
    return this->bank_id_to_bank_offset_.at(bank_id);
}

void BankManager::validate_bank_id(uint32_t bank_id) const {
    TT_FATAL(this->bank_id_to_bank_offset_.find(bank_id) != this->bank_id_to_bank_offset_.end(), "Expected bank {} to be tracked!", bank_id);
}

uint64_t BankManager::allocate_buffer(uint32_t size, uint32_t page_size, bool bottom_up, CoreCoord compute_grid_size, std::optional<uint32_t> num_shards) {
    uint32_t num_banks = this->num_banks();
    bool is_sharded = false;
    if(num_shards.has_value()){
        auto num_compute_banks = compute_grid_size.x * compute_grid_size.y;
        is_sharded = true;
        TT_FATAL(num_shards.value() <= num_compute_banks, "Expected number of shards to be less than or equal to total number of L1 banks in compute cores");
        num_banks = num_shards.value();
    }
    // Each page needs to be at a 32B aligned address
    uint32_t size_per_bank = tt::tt_metal::detail::SizeBytesPerBank(size, page_size, num_banks);
    uint64_t address_limit = 0;
    if(!is_sharded and this->buffer_type_ == BufferType::L1) {
        address_limit = this->interleaved_address_limit_;
        TT_FATAL(address_limit > 0);
    }
    auto address = this->allocator_->allocate(size_per_bank, bottom_up, address_limit);
    if (not address.has_value()) {
        TT_THROW("Out of Memory: Not enough space to allocate {} B {} buffer across {} banks, where each bank needs to store {} B", size, magic_enum::enum_name(this->buffer_type_), num_banks, size_per_bank);
    }
#if defined(TRACY_ENABLE)
    TracyAllocN(reinterpret_cast<void const *>(address.value()), size_per_bank, get_memory_pool_name(buffer_type_));
#endif
    allocated_buffers_.insert(address.value());
    return address.value();
}

void BankManager::deallocate_buffer(uint64_t address) {
#if defined(TRACY_ENABLE)
    TracyFreeN(reinterpret_cast<void const *>(address), get_memory_pool_name(buffer_type_));
#endif
    this->allocator_->deallocate(address);
}

void BankManager::deallocate_all(){
    for (uint64_t addr : this->allocated_buffers_)
    {
        this->allocator_->deallocate(addr);
    }
}


void BankManager::clear() {
    this->allocator_->clear();
}

BankManager::~BankManager() {
    deallocate_all();
    allocated_buffers_.clear();
    bank_id_to_bank_offset_.clear();
    this->allocator_.reset(nullptr);
}

BankManager&& BankManager::operator=(BankManager&& that) {
    buffer_type_ = that.buffer_type_;
    allocated_buffers_ = that.allocated_buffers_;
    bank_id_to_bank_offset_ = that.bank_id_to_bank_offset_;
    allocator_.reset( that.allocator_.release() );
    interleaved_address_limit_ = that.interleaved_address_limit_;
    return std::move(*this);
}

std::optional<uint64_t> BankManager::lowest_occupied_address(uint32_t bank_id) const {
    auto lowest_address = this->allocator_->lowest_occupied_address();
    if (not lowest_address.has_value()) {
        return lowest_address;
    }
    auto adjusted_abs_addr = lowest_address.value() + this->bank_offset(bank_id);
    return adjusted_abs_addr;
}

Statistics BankManager::get_statistics() const {
    return this->allocator_->get_statistics();
}

void BankManager::dump_blocks(std::ofstream &out) const {
    this->allocator_->dump_blocks(out);
}

void init_one_bank_per_channel(Allocator &allocator, const AllocatorConfig &alloc_config) {
    // Space up to DRAM_UNRESERVED_BASE is reserved for DRAM write barrier
    uint64_t offset_bytes = static_cast<uint64_t>(DRAM_UNRESERVED_BASE);
    uint32_t dram_bank_size = alloc_config.dram_bank_size - DRAM_UNRESERVED_BASE;
    std::vector<int64_t> bank_offsets (alloc_config.num_dram_channels);
    for (uint32_t channel_id = 0; channel_id < alloc_config.num_dram_channels; channel_id++) {
        bank_offsets.at(channel_id) = static_cast<int32_t>(alloc_config.dram_bank_offsets.at(channel_id));
    }
    allocator.dram_manager = BankManager(BufferType::DRAM, bank_offsets, dram_bank_size, offset_bytes);
    for (uint32_t bank_id = 0; bank_id < alloc_config.num_dram_channels; bank_id++) {
        allocator.bank_id_to_dram_channel.insert({bank_id, bank_id});
        allocator.dram_channel_to_bank_ids.insert({bank_id, {bank_id}});
    }
}

void init_one_bank_per_l1(Allocator &allocator, const AllocatorConfig &alloc_config) {
    uint32_t num_l1_banks = alloc_config.worker_grid_size.y * alloc_config.worker_grid_size.x;
    // Space up to L1_UNRESERVED_BASE is reserved for risc binaries, kernel args, debug and perf monitoring tools
    uint64_t offset_bytes = static_cast<uint64_t>(L1_UNRESERVED_BASE);
    uint32_t l1_bank_size = alloc_config.worker_l1_size - L1_UNRESERVED_BASE;
    std::vector<int64_t> bank_offsets (num_l1_banks, 0);
    allocator.l1_manager = BankManager(BufferType::L1, bank_offsets, l1_bank_size, offset_bytes);

    uint32_t bank_id = 0;
    for (uint32_t y = 0; y < alloc_config.worker_grid_size.y; y++) {
        for (uint32_t x = 0; x < alloc_config.worker_grid_size.x; x++) {
            CoreCoord logical_core = CoreCoord{x, y};
            allocator.bank_id_to_logical_core.insert({bank_id, logical_core});
            allocator.logical_core_to_bank_ids.insert({logical_core, {bank_id}});
            bank_id++;
        }
    }
}

uint32_t num_banks(const Allocator &allocator, const BufferType &buffer_type) {
    switch (buffer_type) {
        case BufferType::DRAM: return allocator.dram_manager.num_banks();
        case BufferType::L1: return allocator.l1_manager.num_banks();
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
    return 0;
}

uint32_t bank_size(const Allocator &allocator, const BufferType &buffer_type) {
    switch (buffer_type) {
        case BufferType::DRAM: return allocator.dram_manager.bank_size();
        case BufferType::L1: return allocator.l1_manager.bank_size();
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
    return 0;
}

uint32_t dram_channel_from_bank_id(const Allocator &allocator, uint32_t bank_id) {
    TT_ASSERT(allocator.bank_id_to_dram_channel.find(bank_id) != allocator.bank_id_to_dram_channel.end());
    return allocator.bank_id_to_dram_channel.at(bank_id);
}

CoreCoord logical_core_from_bank_id(const Allocator &allocator, uint32_t bank_id) {
    TT_ASSERT(allocator.bank_id_to_logical_core.find(bank_id) != allocator.bank_id_to_logical_core.end());
    return allocator.bank_id_to_logical_core.at(bank_id);
}

int32_t l1_bank_offset_from_bank_id(const Allocator &allocator, uint32_t bank_id) {
    return allocator.l1_manager.bank_offset(bank_id);
}

int32_t dram_bank_offset_from_bank_id(const Allocator &allocator, uint32_t bank_id) {
    return allocator.dram_manager.bank_offset(bank_id);
}

const std::vector<uint32_t> &bank_ids_from_dram_channel(const Allocator &allocator, uint32_t dram_channel) {
    if (allocator.dram_channel_to_bank_ids.find(dram_channel) == allocator.dram_channel_to_bank_ids.end()) {
        TT_THROW("No DRAM bank exists for DRAM channel {}", dram_channel);
    }
    return allocator.dram_channel_to_bank_ids.at(dram_channel);
}

const std::vector<uint32_t> &bank_ids_from_logical_core(const Allocator &allocator, const CoreCoord &logical_core) {
    if (allocator.logical_core_to_bank_ids.find(logical_core) == allocator.logical_core_to_bank_ids.end()) {
        TT_THROW("No L1 bank exists for core {}", logical_core.str());
    }
    return allocator.logical_core_to_bank_ids.at(logical_core);
}

Statistics get_statistics(const Allocator &allocator, const BufferType &buffer_type) {
    Statistics stats;
    switch (buffer_type) {
        case BufferType::DRAM: return allocator.dram_manager.get_statistics();
        case BufferType::L1: return allocator.l1_manager.get_statistics();
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
    return stats;
}

void dump_memory_blocks(const Allocator &allocator, const BufferType &buffer_type, std::ofstream &out) {
    switch (buffer_type) {
        case BufferType::DRAM: allocator.dram_manager.dump_blocks(out);
        break;
        case BufferType::L1: allocator.l1_manager.dump_blocks(out);
        break;
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
}

std::optional<uint64_t> lowest_occupied_l1_address(const Allocator &allocator, uint32_t bank_id) {
    return allocator.l1_manager.lowest_occupied_address(bank_id);
}

uint64_t base_alloc(const AllocatorConfig &config, BankManager &bank_manager, uint64_t size, uint64_t page_size, bool bottom_up, std::optional<uint32_t> num_shards) {
    return bank_manager.allocate_buffer(size, page_size, bottom_up, config.compute_grid_size, num_shards);
}

uint64_t allocate_buffer(Allocator &allocator, uint32_t size, uint32_t page_size, const BufferType &buffer_type, bool bottom_up, std::optional<uint32_t> num_shards) {
    uint64_t address = 0;
    switch (buffer_type) {
        case BufferType::DRAM: return allocator.descriptor.dram.alloc(allocator.config, allocator.dram_manager, size, page_size, bottom_up, std::nullopt);
        case BufferType::L1: return allocator.descriptor.l1.alloc(allocator.config, allocator.l1_manager, size, page_size, bottom_up, num_shards);
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
    return address;
}

void deallocate_buffer(Allocator &allocator, uint64_t address, const BufferType &buffer_type) {
    switch (buffer_type) {
        case BufferType::DRAM:
            allocator.dram_manager.deallocate_buffer(address);
        break;
        case BufferType::L1:
            allocator.l1_manager.deallocate_buffer(address);
        break;
        default: {
            TT_THROW("Unsupported buffer type!");
        }
    }
}

void deallocate_buffers(Allocator &allocator) {
    allocator.dram_manager.deallocate_all();
    allocator.l1_manager.deallocate_all();
}

void clear(Allocator &allocator) {
    allocator.dram_manager.clear();
    allocator.l1_manager.clear();
}

}  // namespace allocator

Allocator::Allocator(const AllocatorConfig &alloc_config, const allocator::AllocDescriptor &alloc_descriptor) : config(alloc_config), descriptor(alloc_descriptor) {
    // TODO: add validation for allocator_descriptor?
    this->descriptor.dram.init(*this, alloc_config);
    this->descriptor.l1.init(*this, alloc_config);
    // assert that bank managers have been initialized?
    TT_ASSERT(not bank_id_to_dram_channel.empty() and not dram_channel_to_bank_ids.empty());
    TT_ASSERT(not bank_id_to_logical_core.empty() and not bank_id_to_logical_core.empty());
}

Allocator::~Allocator() {
    reset();
}

void Allocator::reset() {
    bank_id_to_dram_channel.clear();
    dram_channel_to_bank_ids.clear();
    bank_id_to_logical_core.clear();
    logical_core_to_bank_ids.clear();

    dram_manager.clear();
    l1_manager.clear();
    config.reset();
}

void AllocatorConfig::reset() {
    dram_bank_offsets.clear();
    core_type_from_noc_coord_table.clear();
    worker_log_to_physical_routing_x.clear();
    worker_log_to_physical_routing_y.clear();
    l1_bank_remap.clear();
}

}  // namespace tt_metal

}  // namespace tt
