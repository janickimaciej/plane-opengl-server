#pragma once

#include "app/kick_queue_element.hpp"
#include "app/player_data.hpp"
#include "common/config.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <array>
#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace App
{
	class PlayerManager
	{
		using KickQueue = std::list<KickQueueElement>;

	public:
		PlayerManager();
		int getPlayerId(const asio::ip::udp::endpoint& endpoint) const;
		int addNewPlayer(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestep& timestep);
		void bumpPlayer(int playerId, const Physics::Timestep& timestep);
		std::vector<int> kickPlayers(const Physics::Timestep& timestep);
		std::unordered_map<int, PlayerData> getPlayers() const;
		void killPlayer(int playerId);

	private:
		std::unordered_map<int, PlayerData> m_players{};
		int m_idCounter = 0;
		KickQueue m_kickQueue{};
		std::array<KickQueue::iterator, Common::maxPlayerCount> m_kickQueueIterators{};
		mutable std::mutex m_mutex{};

		int getAvailableId();
	};
};
