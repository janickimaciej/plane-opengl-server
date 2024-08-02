#pragma once

#include "app/exitSignal.hpp"
#include "app/playerManager.hpp"
#include "app/udp/udpCommunication.hpp"
#include "physics/notification.hpp"
#include "physics/simulationBuffer.hpp"
#include "physics/simulationClock.hpp"
#include "physics/timestep.hpp"

#include <thread>

namespace App
{
	class PhysicsThread
	{
	public:
		PhysicsThread(ExitSignal& exitSignal, const Physics::SimulationClock& simulationClock,
			Physics::SimulationBuffer& simulationBuffer, Physics::Notification& notification,
			UDPCommunication& udpCommunication, PlayerManager& playerManager);
		void join();

	private:
		std::thread m_thread;
		ExitSignal& m_exitSignal;

		const Physics::SimulationClock& m_simulationClock;
		Physics::SimulationBuffer& m_simulationBuffer;
		Physics::Notification& m_notification;

		UDPCommunication& m_udpCommunication;

		PlayerManager& m_playerManager;

		void start();
		void mainLoop(const Physics::Timestep& initialTimestep);
		void sleepIfFuture(const Physics::Timestep& timestep);
	};
};
