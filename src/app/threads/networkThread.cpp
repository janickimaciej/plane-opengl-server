#include "app/threads/networkThread.hpp"

#include "app/exitCode.hpp"
#include "app/exitSignal.hpp"
#include "app/threads/physicsThread.hpp"
#include "app/udp/udpCommunication.hpp"
#include "app/udp/udpFrameType.hpp"
#include "common/airplaneTypeName.hpp"
#include "common/config.hpp"
#include "common/mapName.hpp"
#include "common/terrains/maps/maps.hpp"
#include "physics/airplaneDefinitions.hpp"
#include "physics/notification.hpp"
#include "physics/playerInfo.hpp"
#include "physics/playerInput.hpp"
#include "physics/simulationBuffer.hpp"
#include "physics/simulationClock.hpp"
#include "physics/timestamp.hpp"

#include <asio/asio.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace App
{
	NetworkThread::NetworkThread(ExitSignal& exitSignal, Common::MapName mapName,
		int networkThreadPort, int physicsThreadPort) :
		m_exitSignal{exitSignal},
		m_simulationBuffer{-1, mapName},
		m_spawner{*Common::Terrains::maps[toSizeT(mapName)]},
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
			kickPlayers();

			asio::ip::udp::endpoint endpoint{};
			Physics::Timestamp clientTimestamp{};
			UDPFrameType udpFrameType{};
			Common::AirplaneTypeName airplaneTypeName{};
			Physics::Timestep timestep{};
			int playerId{};
			Physics::PlayerInput playerInput{};

			bool received = m_udpCommunication.receiveInitReqOrControlFrame(endpoint,
				clientTimestamp, udpFrameType, airplaneTypeName, timestep, playerId, playerInput);
			
			static constexpr Physics::Timestep frameAgeCutoffOffset{0,
				static_cast<unsigned int>(Common::stepsPerSecond * 0.9f)};
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

	void NetworkThread::kickPlayers()
	{
		Physics::Timestep timestep = m_simulationClock.getTime();	
		std::vector<int> kickedPlayers = m_playerManager.kickPlayers(timestep);
		if (!kickedPlayers.empty())
		{
			m_simulationBuffer.kickPlayers(kickedPlayers, timestep);
			m_notification.setNotification(timestep, false);
		}
	}

	void NetworkThread::handleInitReqFrame(const asio::ip::udp::endpoint& endpoint,
		const Physics::Timestamp& clientTimestamp, const Common::AirplaneTypeName& airplaneTypeName)
	{
		std::optional<int> playerId = m_playerManager.getPlayerId(endpoint);
		if (playerId)
		{
			m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, *playerId);
		}
		else
		{
			Physics::Timestep timestep = m_simulationClock.getTime();
			playerId = m_playerManager.addNewPlayer(endpoint, timestep);
			if (playerId)
			{
				Physics::PlayerInfo playerInfo
				{
					Physics::PlayerInput{},
					Physics::PlayerState
					{
						airplaneTypeName,
						Physics::airplaneDefinitions[Common::toSizeT(airplaneTypeName)].initialHP,
						m_spawner.spawn(airplaneTypeName)
					}
				};
				m_simulationBuffer.writeInitFrame(timestep, *playerId, playerInfo);
				m_notification.setNotification(timestep, false);
				m_udpCommunication.sendInitResFrame(endpoint, clientTimestamp, *playerId);
			}
		}
	}

	void NetworkThread::handleControlFrame(const Physics::Timestamp& clientTimestamp,
		const Physics::Timestep& timestep, int playerId, const Physics::PlayerInput& playerInput)
	{
		if (!m_playerManager.isPlayerIdValid(playerId))
		{
			return;
		}
		m_simulationBuffer.writeControlFrame(timestep, playerId, playerInput);
				m_notification.setNotification(timestep, false);
		m_udpCommunication.broadcastControlFrame(m_playerManager.getPlayers(),
			clientTimestamp, timestep, playerId, playerInput);
		m_playerManager.bumpPlayer(playerId, m_simulationClock.getTime());
	}
};
