#include "app/commandLineArgument.hpp"

namespace App
{
	std::size_t toSizeT(CommandLineArgument commandLineArgument)
	{
		return static_cast<std::size_t>(commandLineArgument);
	}
};
