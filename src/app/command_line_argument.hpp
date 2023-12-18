#pragma once

#include <cstddef>

namespace App
{
	inline constexpr int argumentCount = 3;

	enum class CommandLineArgument
	{
		programName,
		networkThreadPort,
		physicsThreadPort
	};

	std::size_t toSizeT(CommandLineArgument commandLineArgument);
};
