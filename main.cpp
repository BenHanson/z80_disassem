#include "diss.hpp"

#include "../z80_assembler/data.hpp"
#include "../z80_assembler/disassem.hpp"
#include "../z80_assembler/enums.hpp"

#include <lexertl/memory_file.hpp>

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

// Forward declare functions referenced by g_actions
bool absolute(diss_data& diss);
bool extended(diss_data& diss);
bool reg(diss_data& diss);
bool relative(diss_data& diss);
bool ret(diss_data&);
bool rst(diss_data& diss);

const std::map<opcode, bool (*)(diss_data&)> g_actions =
{
	{ opcode::CALL, absolute },
	{ opcode::CALL_C, absolute },
	{ opcode::CALL_M, absolute },
	{ opcode::CALL_NC, absolute },
	{ opcode::CALL_NZ, absolute },
	{ opcode::CALL_P, absolute },
	{ opcode::CALL_PE, absolute },
	{ opcode::CALL_PO, absolute },
	{ opcode::CALL_Z, absolute },
	{ opcode::DJNZ, relative },
	{ opcode::ED_prefix, extended },
	{ opcode::JP_HL, reg },
	{ opcode::IX_prefix, reg },
	{ opcode::IY_prefix, reg },
	{ opcode::JP, absolute },
	{ opcode::JP_C, absolute },
	{ opcode::JP_M, absolute },
	{ opcode::JP_NC, absolute },
	{ opcode::JP_NZ, absolute },
	{ opcode::JP_P, absolute },
	{ opcode::JP_PE, absolute },
	{ opcode::JP_PO, absolute },
	{ opcode::JP_Z, absolute },
	{ opcode::JR, relative },
	{ opcode::JR_C, relative },
	{ opcode::JR_NC, relative },
	{ opcode::JR_NZ, relative },
	{ opcode::JR_Z, relative },
	{ opcode::RET, ret },
	{ opcode::RST00, rst },
	{ opcode::RST08, rst },
	{ opcode::RST10, rst },
	{ opcode::RST18, rst },
	{ opcode::RST20, rst },
	{ opcode::RST28, rst },
	{ opcode::RST30, rst },
	{ opcode::RST38, rst }
};

static bool absolute(diss_data& diss)
{
	uint16_t addr = 0;

	addr = *(diss._curr + 1);
	addr |= *(diss._curr + 2) << 8;

	if (!diss.contains(addr))
		diss._queue.push(addr);

	// JP unconditionally terminates current block
	return *diss._curr == std::bit_cast<uint8_t>(opcode::JP);
}

static bool extended(diss_data& diss)
{
	++diss._curr;

	switch (static_cast<opcode>(*diss._curr))
	{
	case opcode::RETI:
	case opcode::RETN:
		// Unconditionally terminates current block
		return true;
	default:
		break;
	}

	return false;
}

static bool reg(diss_data& diss)
{
	switch (static_cast<opcode>(*diss._curr))
	{
	case opcode::IX_prefix:
	case opcode::IY_prefix:
		++diss._curr;
		break;
	default:
		break;
	}

	if (static_cast<opcode>(*diss._curr) == opcode::JP_HL)
		std::cerr << std::format("Warning: Jump to register {}: {}\n",
			diss._curr_addr,
			diss._curr_inst);

	// Need to emulate CPU in order to jump to an address specified
	// by a register, so for now return false as we don't support that
	return false;
}

static bool relative(diss_data& diss)
{
	const char offset = *(diss._curr + 1);

	if (const uint16_t addr = (diss._curr_addr + 2 + offset) & 0xffff;
		!diss.contains(addr))
	{
		diss._queue.push(addr);
	}

	// JR unconditionally terminates current block
	return *diss._curr == std::bit_cast<uint8_t>(opcode::JR);
}

static bool ret(diss_data&)
{
	// Unconditionally terminates current block
	return true;
}

static bool rst(diss_data& diss)
{
	// .sna files do not include ROM, so don't attempt to
	// disassemble low addresses (RST addresses are single byte)

	// RST performs a type of CALL
	return false;
}

static const uint8_t* address(uint16_t addr, const char* bytes)
{
	// sna does not hold ROM code
	addr -= 16384;
	// 27 bytes file header
	addr += 27;
	return std::bit_cast<const uint8_t*>(&bytes[addr]);
}

static void scan_code(const lexertl::memory_file& bytes, const program& program,
	diss_data& diss)
{
	std::string instr;

	for (; !diss._queue.empty(); diss._queue.pop())
	{
		bool stop = false;

		diss._curr = address(diss._queue.front(), bytes.data());

		while (!stop)
		{
			diss._curr_addr = ((diss._curr - diss._start) + diss._start_addr) & 0xffff;

			if (diss._curr_addr < diss._start_addr || diss.contains(diss._curr_addr))
			{
				// Ignore ROM routines
				stop = true;
				continue;
			}

			diss.next = diss._curr;
			instr = mnemonic(program, base::decimal, diss.next, relative::offset);
			//std::cout << diss._curr_addr << ": " << instr << '\n';

			const uint32_t next_addr = ((diss.next - diss._start) + diss._start_addr) & 0xffffffff;

			diss._curr_inst = instr;
			diss.next_addr = next_addr & 0xffff;
			diss._blocks.insert(std::make_pair(diss._curr_addr, next_addr > 65535 ?
				65536 :
				diss.next_addr));

			if (auto iter = g_actions.find(static_cast<opcode>(*diss._curr));
				iter != g_actions.end() &&
				iter->second(diss))
			{
				// opcode terminates this block
				stop = true;
				continue;
			}

			diss._curr = diss.next;
		}
	}
}

int main(int argc, const char* argv[])
{
	if (argc != 4)
	{
		std::cout << "z80_disassem <pathname (asm)> <start of code> <entry point>\n";
		return 1;
	}

	try
	{
		auto pathname = argv[1];
		lexertl::memory_file bytes(pathname);

		if (!bytes.data())
			throw std::runtime_error(std::format("Failed to load {}",
				pathname));

		uint16_t start_addr = atoi(argv[2]) & 0xffff; // 25600 for Pyramania
		auto entry_point = atoi(argv[3]);
		diss_data diss
		{
			start_addr,
			address(start_addr, bytes.data()),
			address(0xFFFF, bytes.data()) + 1
		};
		data data;
		uint32_t last = start_addr;

		data._program._org = diss._start_addr;
		data._program._memory.assign(diss._start, diss._end);
		// Entry point. 38400 for Pyramania
		diss._queue.push(entry_point & 0xffff);
		scan_code(bytes, data._program, diss);

		for (const auto& [first, second] : diss._blocks._ranges)
		{
			if (last < first)
			{
				data._program._mem_type.
					emplace_back(program::block::type::db, first - last);
			}

			data._program._mem_type.
				emplace_back(program::block::type::code, second - first);
			last = second;
		}

		if (last < 65536)
			data._program._mem_type.emplace_back(program::block::type::db,
				65536 - last);

		dump(data._program, base::decimal, relative::absolute);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << '\n';
	}
}
