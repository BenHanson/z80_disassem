#include "diss.hpp"

#include "../z80_assembler/data.hpp"
#include "../z80_assembler/disassem.hpp"
#include "../z80_assembler/enums.hpp"

#include <lexertl/memory_file.hpp>

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

static bool absolute(diss_data& diss)
{
	uint16_t addr = 0;

	addr = *(diss._curr + 1);
	addr |= *(diss._curr + 2) << 8;

	if (!diss.contains(addr))
		diss._queue.push(addr);

	return *diss._curr == std::bit_cast<uint8_t>(opcode::JP);
}

static bool relative(diss_data& diss)
{
	const char offset = *(diss._curr + 1);

	if (const uint16_t addr = (diss._curr_addr + 2 + offset) & 0xffff;
		!diss.contains(addr))
	{
		diss._queue.push(addr);
	}

	return *diss._curr == std::bit_cast<uint8_t>(opcode::JR);
}

static bool reg(diss_data& diss)
{
	switch (*diss._curr)
	{
	case 0xDD:
	case 0xFD:
		++diss._curr;
		break;
	default:
		break;
	}

	if (*diss._curr == 0xE9)
		throw std::runtime_error("Jump to register");

	return false;
}

static bool ret(diss_data&)
{
	return true;
}

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
	{ opcode::JP_HL, reg },
	{ opcode::JP_IX, reg },
	{ opcode::JP_IY, reg },
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
	{ opcode::RET, ret }
};

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
	std::string str;

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
			str = mnemonic(program, base::decimal, diss.next, relative::offset);
			//std::cout << diss._curr_addr << ": " << str << '\n';

			const uint32_t next_addr = ((diss.next - diss._start) + diss._start_addr) & 0xffffffff;

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
		lexertl::memory_file bytes(argv[1]);
		uint16_t start_addr = atoi(argv[2]) & 0xffff; // 25600 for Pyramania
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
		diss._queue.push(atoi(argv[3]) & 0xffff); // Entry point. 38400 for Pyramania
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
