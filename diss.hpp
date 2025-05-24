#pragma once

#include <lexertl/string_token.hpp>

#include <algorithm>
#include <cstdint>
#include <queue>

enum class opcode : uint8_t
{
	CALL = 0xCD,
	CALL_C = 0xDC,
	CALL_M = 0xFC,
	CALL_NC = 0xD4,
	CALL_NZ = 0xC4,
	CALL_P = 0xF4,
	CALL_PE = 0xEC,
	CALL_PO = 0xE4,
	CALL_Z = 0xCC,
	DJNZ = 0x10,
	JP_HL = 0xE9,
	JP_IX = 0xDD,
	JP_IY = 0xFD,
	JP = 0xC3,
	JP_C = 0xDA,
	JP_M = 0xFA,
	JP_NC = 0xD2,
	JP_NZ = 0xC2,
	JP_P = 0xF2,
	JP_PE = 0xEA,
	JP_PO = 0xE2,
	JP_Z = 0xCA,
	JR = 0x18,
	JR_C = 0x38,
	JR_NC = 0x30,
	JR_NZ = 0x20,
	JR_Z = 0x28,
	RET = 0xC9
};

struct diss_data
{
	uint16_t _start_addr = 0;
	const uint8_t* _start = nullptr;
	const uint8_t* _end = nullptr;
	const uint8_t* _curr = nullptr;
	uint16_t _curr_addr = 0;
	const uint8_t* next = nullptr;
	uint16_t next_addr = 0;
	lexertl::basic_string_token<uint32_t> _blocks;
	std::queue<uint16_t> _queue;

	bool contains(const uint16_t addr) const
	{
		return std::ranges::any_of(_blocks._ranges, [addr](const auto& pair)
			{
				return addr >= pair.first && addr < pair.second;
			});
	}
};
