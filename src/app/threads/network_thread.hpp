#pragma once

#include "app/exit_signal.hpp"
#include "app/player_manager.hpp"
#include "app/udp/udp_communication.hpp"
#include "common/map_name.hpp"
#include "physics/notification.hpp"
#include "physics/simulation_buffer.hpp"
#include "physics/simulation_clock.hpp"

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

		Physics::Notification m_notification{m_simulationClock};
		Physics::Timestep m_frameCutoff{};
		UDPCommunication m_udpCommunication;

		PlayerManager m_playerManager{};

		void mainLoop();

		void kickInactivePlayers();
		void handleInitReqFrame(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestamp& clientTimestamp,
			const Common::AirplaneTypeName& airplaneTypeName);
		void handleControlFrame(const Physics::Timestamp& clientTimestamp,
			const Physics::Timestep& timestep, int playerId,
			const Physics::PlayerInput& playerInput);
	};
};
