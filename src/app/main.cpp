#include "app/command_line_argument.hpp"
#include "app/exit_code.hpp"
#include "app/exit_signal.hpp"
#include "app/threads/network_thread.hpp"
#include "common/map_name.hpp"

#include <memory>
#include <string>

namespace App
{
	bool parseArguments(int argc, char** argv, Common::MapName& mapName, int& networkThreadPort,
		int& physicsThreadPort);
};

int main(int argc, char** argv)
{
	using namespace App;

	Common::MapName mapName{};
	int networkThreadPort{};
	int physicsThreadPort{};

	if (!parseArguments(argc, argv, mapName, networkThreadPort, physicsThreadPort))
	{
		return toInt(ExitCode::badArguments);
	}

	ExitSignal exitSignal{};
	std::unique_ptr<NetworkThread> networkThread =
		std::make_unique<NetworkThread>(exitSignal, mapName, networkThreadPort, physicsThreadPort);
	networkThread->start();

	return toInt(exitSignal.getExitCode());
};

namespace App
{
	bool parseArguments(int argc, char** argv, Common::MapName& mapName,
		int& networkThreadPort, int& physicsThreadPort)
	{
		if (argc != argumentCount)
		{
			return false;
		}

		int mapNameIndex = std::stoi(argv[toSizeT(CommandLineArgument::map)]);
		if (mapNameIndex < 0 || mapNameIndex >= Common::mapCount)
		{
			return false;
		}
		mapName = static_cast<Common::MapName>(mapNameIndex);
		
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
