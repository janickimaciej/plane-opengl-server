#include "app/command_line_argument.hpp"
#include "app/exit_code.hpp"
#include "app/exit_signal.hpp"
#include "app/threads/network_thread.hpp"

#include <memory>
#include <string>

namespace App
{
	bool parseArguments(int argc, char** argv, int& networkThreadPort, int& physicsThreadPort);
};

int main(int argc, char** argv)
{
	using namespace App;

	int networkThreadPort{};
	int physicsThreadPort{};

	if (!parseArguments(argc, argv, networkThreadPort, physicsThreadPort))
	{
		return toInt(ExitCode::badArguments);
	}

	ExitSignal exitSignal{};
	std::unique_ptr<NetworkThread> networkThread =
		std::make_unique<NetworkThread>(exitSignal, networkThreadPort, physicsThreadPort);
	networkThread->start();

	return toInt(exitSignal.getExitCode());
};

namespace App
{
	bool parseArguments(int argc, char** argv, int& networkThreadPort, int& physicsThreadPort)
	{
		if (argc != argumentCount)
		{
			return false;
		}
		
		constexpr int minPortValue = 0;
		constexpr int maxPortValue = 1 << 16;

		networkThreadPort = std::stoi(argv[toSizeT(CommandLineArgument::networkThreadPort)]);
		if (networkThreadPort < minPortValue || networkThreadPort >= maxPortValue)
		{
			return false;
		}

		physicsThreadPort = std::stoi(argv[toSizeT(CommandLineArgument::physicsThreadPort)]);
		if (physicsThreadPort < minPortValue || physicsThreadPort >= maxPortValue)
		{
			return false;
		}

		return true;
	}
};
