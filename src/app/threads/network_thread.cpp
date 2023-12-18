#include "app/threads/network_thread.hpp"

#include "app/exit_code.hpp"
#include "app/exit_signal.hpp"
#include "app/threads/physics_thread.hpp"
#include "app/udp/udp_communication.hpp"
#include "app/udp/udp_frame_type.hpp"
#include "common/airplane_type_name.hpp"
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

namespace App
{
	NetworkThread::NetworkThread(ExitSignal& exitSignal, int networkThreadPort,
		int physicsThreadPort) :
		m_exitSignal{exitSignal},
		m_udpCommunication{networkThreadPort, physicsThreadPort}
	{ }

	void NetworkThread::start()
	{
		PhysicsThread physicsThread{m_exitSignal, m_simulationClock, m_simulationBuffer,
			m_notification, m_udpCommunication, m_playerManager};
		mainLoop();
		physicsThread.join();
	}

	void NetworkThread::mainLoop()
	{
		while (!m_exitSignal.shouldStop())
		{
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
				static_cast<unsigned int>(Physics::framesPerSecond * 0.9f)};
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
				playerId = m_playerManager.getPlayerId(endpoint);
				if (playerId != -1)
				{
					m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, playerId);
				}
				else
				{
					playerId = m_playerManager.addNewPlayer(endpoint);
					if (playerId != -1)
					{
						Common::State newPlayerState{};
						newPlayerState.position = glm::vec3{0, 250, 500};
						newPlayerState.velocity = glm::vec3{0, 0, -100};
						Physics::PlayerInfo newPlayerInfo
						{
							Physics::PlayerInput{},
							Physics::PlayerState
							{
								100,
								newPlayerState,
								airplaneTypeName
							}
						};
						timestep = m_simulationClock.getTime();
						m_simulationBuffer.writeInitFrame(timestep, playerId, newPlayerInfo);
						m_notification.setNotification(timestep, false);
						m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, playerId);
					}
				}
			}
			else if (udpFrameType == UDPFrameType::control && timestep > m_frameCutoff)
			{
				m_simulationBuffer.writeControlFrame(timestep, playerId, playerInput);
				m_notification.setNotification(timestep, false);
				for (const std::pair<const int, PlayerData>& player : m_playerManager.getPlayers())
				{
					m_udpCommunication.sendControlFrame(m_playerManager.getEndpoint(player.first),
						clientTimestamp, timestep, playerId, playerInput);
				}
			}
		}
	}
};
