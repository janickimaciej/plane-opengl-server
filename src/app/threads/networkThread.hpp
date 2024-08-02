#pragma once

#include "app/exitSignal.hpp"
#include "app/playerManager.hpp"
#include "app/udp/udpCommunication.hpp"
#include "common/mapName.hpp"
#include "physics/notification.hpp"
#include "physics/simulationBuffer.hpp"
#include "physics/simulationClock.hpp"
#include "physics/spawner.hpp"

#include <unordered_map>

namespace App
{
	class NetworkThread
	{
	public:
		NetworkThread(ExitSignal& exitSignal, Common::MapName mapName, int networkThreadPort,
			int physicsThreadPort);
		void start();

	private:
		ExitSignal& m_exitSignal;

		Physics::SimulationClock m_simulationClock{};
		Physics::SimulationBuffer m_simulationBuffer;
		Physics::Spawner m_spawner;

		Physics::Notification m_notification{m_simulationClock};
		Physics::Timestep m_frameCutoff{};
		UDPCommunication m_udpCommunication;

		PlayerManager m_playerManager{};

		void mainLoop();

		void kickPlayers();
		void handleInitReqFrame(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestamp& clientTimestamp,
			const Common::AirplaneTypeName& airplaneTypeName);
		void handleControlFrame(const Physics::Timestamp& clientTimestamp,
			const Physics::Timestep& timestep, int playerId,
			const Physics::PlayerInput& playerInput);
	};
};
