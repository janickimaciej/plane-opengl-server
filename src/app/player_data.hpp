#pragma once

#include "physics/timestep.hpp"

#include <asio/asio.hpp>

namespace App
{
	struct PlayerData
	{
		asio::ip::udp::endpoint endpoint;
		Physics::Timestep keepAliveTimestep{};
		bool keepAliveLock{};
	};
};
