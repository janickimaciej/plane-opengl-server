#include "app/exitCode.hpp"

namespace App
{
	int toInt(ExitCode exitCode)
	{
		return static_cast<int>(exitCode);
	}
};
