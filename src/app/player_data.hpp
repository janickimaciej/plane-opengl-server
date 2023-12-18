#pragma once

#include <asio/asio.hpp>

namespace App
{
	struct PlayerData
	{
		asio::ip::udp::endpoint endpoint;
	};
};
