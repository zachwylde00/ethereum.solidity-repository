/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <evmone/evmone.h>
#include <evmc/helpers.hpp>
#include <evmc/evmc.hpp>
#include <intx/intx.hpp>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace external
{
namespace evmone
{
class BlockchainState: public evmc::Host
{
public:
	evmc_address last_accessed_account = {};
	std::unordered_map<evmc_bytes32, evmc_bytes32> storage;
	bool storage_cold = true;
	evmc_tx_context tx_context = {};
	std::basic_string<uint8_t> log_data;
	std::vector<evmc_bytes32> log_topics;
	evmc_address selfdestruct_beneficiary = {};
	evmc_bytes32 blockhash = {};
	bool exists = false;
	intx::uint256 balance = {};
	std::basic_string<uint8_t> extcode = {};
	evmc_message call_msg = {};  ///< Recorded call message.
	evmc_result call_result = {};

	bool account_exists(const evmc_address& addr) noexcept override
	{
		last_accessed_account = addr;
		return exists;
	}

	evmc_bytes32 get_storage(const evmc_address&, const evmc_bytes32& key) noexcept override
	{
		return storage[key];
	}

	evmc_storage_status set_storage(
			const evmc_address&, const evmc_bytes32& key, const evmc_bytes32& value) noexcept override
	{
		auto& old = storage[key];

		evmc_storage_status status;
		if (old == value)
			status = EVMC_STORAGE_UNCHANGED;
		else if (is_zero(old))
			status = EVMC_STORAGE_ADDED;
		else if (is_zero(value))
			status = EVMC_STORAGE_DELETED;
		else if (storage_cold)
			status = EVMC_STORAGE_MODIFIED;
		else
			status = EVMC_STORAGE_MODIFIED_AGAIN;

		old = value;
		return status;
	}

	evmc_uint256be get_balance(const evmc_address& addr) noexcept override
	{
		last_accessed_account = addr;
		evmc_uint256be b = {};
		intx::be::store(b.bytes, balance);
		return b;
	}

	size_t get_code_size(const evmc_address& addr) noexcept override
	{
		last_accessed_account = addr;
		if (extcode.size() == 0)
			return 1;
		return extcode.size();
	}

	evmc_bytes32 get_code_hash(const evmc_address& addr) noexcept override
	{
		last_accessed_account = addr;
		auto hash = evmc_bytes32{};
		std::fill(std::begin(hash.bytes), std::end(hash.bytes), uint8_t{0xee});
		return hash;
	}

	size_t copy_code(const evmc_address& addr, size_t code_offset, uint8_t* buffer_data,
	                 size_t buffer_size) noexcept override
	{
		last_accessed_account = addr;
		const auto n = std::min(buffer_size, extcode.size());
		if (n > 0)
			std::copy_n(&extcode[code_offset], buffer_size, buffer_data);
		return n;
	}

	void selfdestruct(const evmc_address&, const evmc_address& beneficiary) noexcept override
	{
		selfdestruct_beneficiary = beneficiary;
	}

	evmc::result call(const evmc_message& msg) noexcept override
	{
		call_msg = msg;
		return evmc::result{call_result};
	}

	evmc_tx_context get_tx_context() noexcept override { return tx_context; }

	evmc_bytes32 get_block_hash(int64_t) noexcept override { return blockhash; }

	void emit_log(const evmc_address&, const uint8_t* data, size_t data_size,
	              const evmc_bytes32 topics[], size_t topics_count) noexcept override
	{
		log_data.assign(data, data_size);
		log_topics.clear();
		log_topics.reserve(topics_count);
		std::copy_n(topics, topics_count, std::back_inserter(log_topics));
	}
};

class EvmOneVM: public BlockchainState
{
public:
	explicit EvmOneVM(evmc_revision const& _evmcRevision = EVMC_PETERSBURG);
	void execute(std::basic_string<uint8_t> _rawByteCode, std::string _hexInput = {});
	void execute(std::string _hexCode, std::string _hexInput = {});
	void execute(const evmc_message& _m, std::basic_string<uint8_t> _rawByteCode);
	void execute(int64_t _gas, std::basic_string<uint8_t> _rawByteCode, std::string _hexInput = {});
	void execute(int64_t _gas, std::string _code, std::string _input);
	static std::basic_string<uint8_t> fromHex(std::string _hex);

private:
	evmc_instance* m_vm;
	evmc_revision m_rev;
};
}
}