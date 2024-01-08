#pragma once

#include <cstddef>

namespace App
{
	inline constexpr int argumentCount = 4;

	enum class CommandLineArgument
	{
		programName,
		map,
		networkThreadPort,
		physicsThreadPort
	};

	std::size_t toSizeT(CommandLineArgument commandLineArgument);
};
