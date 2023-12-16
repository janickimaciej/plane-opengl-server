#include "app/command_line_argument.hpp"

namespace App
{
	std::size_t toSizeT(CommandLineArgument commandLineArgument)
	{
		return static_cast<std::size_t>(commandLineArgument);
	}
};
