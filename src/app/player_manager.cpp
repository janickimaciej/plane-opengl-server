#include "app/player_manager.hpp"

#include "app/udp/frame/state_frame.hpp"

#include <asio/asio.hpp>

namespace App
{
	int PlayerManager::getPlayerId(const asio::ip::udp::endpoint& endpoint) const
	{
		for (const std::pair<const int, PlayerData>& player : m_players)
		{
			if (player.second.endpoint == endpoint)
			{
				return player.first;
			}
		}
		return -1;
	}
	
	int PlayerManager::addNewPlayer(const asio::ip::udp::endpoint& endpoint)
	{
		int newPlayerId = getAvailableId();
		if (newPlayerId == -1)
		{
			return -1;
		}
		m_players.insert({newPlayerId, PlayerData{endpoint}});
		return newPlayerId;
	}

	const std::unordered_map<int, PlayerData>& PlayerManager::getPlayers() const
	{
		return m_players;
	}
	
	const asio::ip::udp::endpoint& PlayerManager::getEndpoint(int playerId) const
	{
		return m_players.at(playerId).endpoint;
	}

	int PlayerManager::getAvailableId()
	{
		int start = m_idCounter;
		while (m_players.contains(m_idCounter))
		{
			m_idCounter = (m_idCounter + 1) % static_cast<int>(maxPlayerCount);
			if (m_idCounter == start)
			{
				return -1;
			}
		}
		
		int availableId = m_idCounter;
		m_idCounter = (m_idCounter + 1) % static_cast<int>(maxPlayerCount);
		return availableId;
	}
};
