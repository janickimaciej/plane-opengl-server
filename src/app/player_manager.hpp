#pragma once

#include "app/player_data.hpp"

#include <asio/asio.hpp>

#include <unordered_map>

namespace App
{
	class PlayerManager
	{
	public:
		int getPlayerId(const asio::ip::udp::endpoint& endpoint) const;
		int addNewPlayer(const asio::ip::udp::endpoint& endpoint);
		const std::unordered_map<int, PlayerData>& getPlayers() const;
		const asio::ip::udp::endpoint& getEndpoint(int playerId) const;

	private:
		std::unordered_map<int, PlayerData> m_players{};
		int m_idCounter = 0;

		int getAvailableId();
	};
};
