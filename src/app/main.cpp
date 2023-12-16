#include "app/command_line_argument.hpp"
#include "app/exit_code.hpp"
#include "app/exit_signal.hpp"

#include <string>

namespace App
{
	bool parseArguments(int argc, char** argv, int& port);
};

int main(int argc, char** argv)
{
	using namespace App;

	int port{};

	if (!parseArguments(argc, argv, port))
	{
		return toInt(ExitCode::badArguments);
	}

	ExitSignal exitSignal{};
	// TODO: NetworkThread
	// TODO: start

	return toInt(exitSignal.getExitCode());
};

namespace App
{
	bool parseArguments(int argc, char** argv, int& port)
	{
		if (argc != argumentCount)
		{
			return false;
		}

		port = std::stoi(argv[toSizeT(CommandLineArgument::port)]);
		constexpr int minPortValue = 0;
		constexpr int maxPortValue = 1 << 16;
		if (port < minPortValue || port >= maxPortValue)
		{
			return false;
		}

		return true;
	}
};
