#pragma once

#include <cstddef>

namespace App
{
	inline constexpr int argumentCount = 2;

	enum class CommandLineArgument
	{
		programName,
		port
	};

	std::size_t toSizeT(CommandLineArgument commandLineArgument);
};
