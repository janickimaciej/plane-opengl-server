#pragma once

#include "app/player_data.hpp"
#include "common/config.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <array>
#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace App
{
	class PlayerManager
	{
	public:
		std::optional<int> getPlayerId(const asio::ip::udp::endpoint& endpoint) const;
		std::optional<int> addNewPlayer(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestep& timestep);
		void bumpPlayer(int playerId, const Physics::Timestep& timestep);
		std::vector<int> kickPlayers(const Physics::Timestep& timestep);
		std::unordered_map<int, PlayerData> getPlayers() const;
		void killPlayer(int playerId, const Physics::Timestep& timestep);

	private:
		std::unordered_map<int, PlayerData> m_players{};
		int m_idCounter = 0;
		mutable std::mutex m_mutex{};

		std::optional<int> getAvailableId();
	};
};
