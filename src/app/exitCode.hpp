#pragma once

namespace App
{
	enum class ExitCode
	{
		ok,
		badArguments
	};

	int toInt(ExitCode exitCode);
};
