#include "app/threads/network_thread.hpp"

#include "app/exit_code.hpp"
#include "app/exit_signal.hpp"
#include "app/threads/physics_thread.hpp"
#include "app/udp/udp_communication.hpp"
#include "app/udp/udp_frame_type.hpp"
#include "common/airplane_type_name.hpp"
#include "common/config.hpp"
#include "physics/notification.hpp"
#include "physics/player_info.hpp"
#include "physics/player_input.hpp"
#include "physics/simulation_buffer.hpp"
#include "physics/simulation_clock.hpp"
#include "physics/timestamp.hpp"

#include <asio/asio.hpp>

#include <atomic>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace App
{
	NetworkThread::NetworkThread(ExitSignal& exitSignal, int networkThreadPort,
		int physicsThreadPort) :
		m_exitSignal{exitSignal},
		m_udpCommunication{networkThreadPort, physicsThreadPort}
	{ }

	void NetworkThread::start()
	{
		m_frameCutoff = m_simulationClock.getTime();
		PhysicsThread physicsThread{m_exitSignal, m_simulationClock, m_simulationBuffer,
			m_notification, m_udpCommunication, m_playerManager};
		mainLoop();
		physicsThread.join();
	}

	void NetworkThread::mainLoop()
	{
		while (!m_exitSignal.shouldStop())
		{
			kickInactivePlayers();

			asio::ip::udp::endpoint endpoint{};
			Physics::Timestamp clientTimestamp{};
			UDPFrameType udpFrameType{};
			Common::AirplaneTypeName airplaneTypeName{};
			Physics::Timestep timestep{};
			int playerId{};
			Physics::PlayerInput playerInput{};

			bool received = m_udpCommunication.receiveInitReqOrControlFrame(endpoint,
				clientTimestamp, udpFrameType, airplaneTypeName, timestep, playerId, playerInput);
			
			constexpr Physics::Timestep frameAgeCutoffOffset{0,
				static_cast<unsigned int>(Common::framesPerSecond * 0.9f)};
			Physics::Timestep frameAgeCutoff = m_simulationClock.getTime() - frameAgeCutoffOffset;
			if (m_frameCutoff < frameAgeCutoff)
			{
				m_frameCutoff = frameAgeCutoff;
			}

			if (!received)
			{
				continue;
			}

			if (udpFrameType == UDPFrameType::initReq)
			{
				handleInitReqFrame(endpoint, clientTimestamp, airplaneTypeName);
			}
			else if (udpFrameType == UDPFrameType::control && timestep > m_frameCutoff)
			{
				handleControlFrame(clientTimestamp, timestep, playerId, playerInput);
			}
		}
	}

	void NetworkThread::kickInactivePlayers()
	{
		Physics::Timestep timestep = m_simulationClock.getTime();	
		std::vector<int> inactivePlayers = m_playerManager.kickInactivePlayers(timestep);
		if (!inactivePlayers.empty())
		{
			m_simulationBuffer.removeInactivePlayers(inactivePlayers, timestep);
			m_notification.setNotification(timestep, false);
		}
	}

	void NetworkThread::handleInitReqFrame(const asio::ip::udp::endpoint& endpoint,
		const Physics::Timestamp& clientTimestamp, const Common::AirplaneTypeName& airplaneTypeName)
	{
		int playerId = m_playerManager.getPlayerId(endpoint);
		if (playerId != -1)
		{
			m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, playerId);
		}
		else
		{
			Physics::Timestep timestep = m_simulationClock.getTime();
			playerId = m_playerManager.addNewPlayer(endpoint, timestep);
			if (playerId != -1)
			{
				Common::State state{};
				state.position = glm::vec3{0, 500, 5000};
				state.velocity = glm::vec3{0, 0, -100};
				Physics::PlayerInfo playerInfo
				{
					Physics::PlayerInput{},
					Physics::PlayerState
					{
						100,
						state,
						airplaneTypeName
					}
				};
				m_simulationBuffer.writeInitFrame(timestep, playerId, playerInfo);
				m_notification.setNotification(timestep, false);
				m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, playerId);
			}
		}
	}

	void NetworkThread::handleControlFrame(const Physics::Timestamp& clientTimestamp,
		const Physics::Timestep& timestep, int playerId, const Physics::PlayerInput& playerInput)
	{
		m_simulationBuffer.writeControlFrame(timestep, playerId, playerInput);
				m_notification.setNotification(timestep, false);
		m_udpCommunication.broadcastControlFrame(m_playerManager.getPlayers(),
			clientTimestamp, timestep, playerId, playerInput);
		m_playerManager.bumpPlayer(playerId, m_simulationClock.getTime());
	}
};
