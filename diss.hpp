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
	DD_prefix = 0xDD,
	DJNZ = 0x10,
	ED_prefix = 0xED,
	FD_prefix = 0xFD,
	IX_prefix = 0xDD,
	IY_prefix = 0xFD,
	JP = 0xC3,
	JP_C = 0xDA,
	JP_HL = 0xE9,
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
	LD_BC_nn = 0x4B,
	LD_DE_nn = 0x5B,
	LD_HL_nn = 0x2A,
	ED_LD_HL_nn = 0x6B,
	LD_SP_nn = 0x7B,
	LD_IX_nn = 0x2A,
	LD_IY_nn = 0x2A,
	LD_nn_BC = 0x43,
	LD_nn_DE = 0x53,
	LD_nn_HL = 0x22,
	ED_LD_nn_HL = 63,
	LD_nn_SP = 0x73,
	LD_nn_IX = 0x22,
	LD_nn_IY = 0x22,
	RET = 0xC9,
	RETI = 0x4D,
	RETN = 0x45,
	RST00 = 0xC7,
	RST08 = 0xCF,
	RST10 = 0xD7,
	RST18 = 0xDF,
	RST20 = 0xE7,
	RST28 = 0xEF,
	RST30 = 0xF7,
	RST38 = 0xFF
};

struct diss_data
{
	uint16_t _start_addr = 0;
	const uint8_t* _start = nullptr;
	const uint8_t* _end = nullptr;
	const uint8_t* _curr = nullptr;
	uint16_t _curr_addr = 0;
	std::string_view _curr_inst;
	const uint8_t* next = nullptr;
	uint16_t next_addr = 0;
	lexertl::basic_string_token<uint32_t> _code;
	lexertl::basic_string_token<uint32_t> _dw;
	std::queue<uint16_t> _queue;

	bool contains(const uint16_t addr) const
	{
		return std::ranges::any_of(_code._ranges, [addr](const auto& pair)
			{
				return addr >= pair.first && addr < pair.second;
			});
	}
};
