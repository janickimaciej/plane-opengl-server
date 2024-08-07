#include "app/threads/physicsThread.hpp"

#include "app/exitSignal.hpp"
#include "app/playerManager.hpp"
#include "app/udp/udpCommunication.hpp"
#include "common/airplaneInfo.hpp"
#include "physics/notification.hpp"
#include "physics/playerInput.hpp"
#include "physics/simulationBuffer.hpp"
#include "physics/simulationClock.hpp"
#include "physics/timestep.hpp"

#include <thread>

namespace App
{
	PhysicsThread::PhysicsThread(ExitSignal& exitSignal,
		const Physics::SimulationClock& simulationClock,
		Physics::SimulationBuffer& simulationBuffer, Physics::Notification& notification,
		UDPCommunication& udpCommunication, PlayerManager& playerManager) :
		m_exitSignal{exitSignal},
		m_simulationClock{simulationClock},
		m_simulationBuffer{simulationBuffer},
		m_notification{notification},
		m_udpCommunication{udpCommunication},
		m_playerManager{playerManager},
		m_thread
		{
			[this]
			{
				this->start();
			}
		}
	{ }

	void PhysicsThread::join()
	{
		m_thread.join();
	}

	void PhysicsThread::start()
	{
		Physics::Timestep initialTimestep = m_simulationClock.getTime();

		m_simulationBuffer.update(initialTimestep);

		mainLoop(initialTimestep);
	}

	void PhysicsThread::mainLoop(const Physics::Timestep& initialTimestep)
	{
		Physics::Timestep timestep = initialTimestep;
		while (!m_exitSignal.shouldStop())
		{
			timestep = timestep.next();
			m_notification.getNotification(timestep);
			sleepIfFuture(timestep);

			m_simulationBuffer.update(timestep);

			if (timestep.step == 0)
			{
				std::unordered_map<int, Physics::PlayerInfo> playerInfos =
					m_simulationBuffer.getPlayerInfos(timestep);
				for (const std::pair<const int, Physics::PlayerInfo>& playerInfo : playerInfos)
				{
					if (playerInfo.second.state.hp == 0)
					{
						m_playerManager.killPlayer(playerInfo.first, timestep);
					}
				}
				m_udpCommunication.broadcastStateFrame(m_playerManager.getPlayers(), timestep,
					playerInfos);
			}
		}
	}

	void PhysicsThread::sleepIfFuture(const Physics::Timestep& timestep)
	{
		Physics::Timestep currentTimestep = m_simulationClock.getTime();
		while (currentTimestep < timestep)
		{
			currentTimestep = m_simulationClock.getTime();
		}
	}
};
