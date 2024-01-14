#include "app/player_manager.hpp"

#include "common/config.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <optional>
#include <utility>
#include <vector>

namespace App
{
	std::optional<int> PlayerManager::getPlayerId(const asio::ip::udp::endpoint& endpoint) const
	{
		for (const std::pair<const int, PlayerData>& player : m_players)
		{
			if (player.second.endpoint == endpoint)
			{
				return player.first;
			}
		}
		return std::nullopt;
	}

	bool PlayerManager::isPlayerIdValid(int playerId) const
	{
		return m_players.contains(playerId);
	}
	
	std::optional<int> PlayerManager::addNewPlayer(const asio::ip::udp::endpoint& endpoint,
		const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		std::optional<int> newPlayerId = getAvailableId();
		if (newPlayerId)
		{
			static const Physics::Timestep timeout{10, 0};
			m_players.insert({*newPlayerId,
				PlayerData
				{
					endpoint,
					timestep + timeout,
					false
				}});
		}

		m_mutex.unlock();

		return newPlayerId;
	}

	void PlayerManager::bumpPlayer(int playerId, const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		if (!m_players.at(playerId).keepAliveLock)
		{
			static const Physics::Timestep timeout{10, 0};
			m_players.at(playerId).keepAliveTimestep = timestep + timeout;
		}

		m_mutex.unlock();
	}

	std::vector<int> PlayerManager::kickPlayers(const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		std::vector<int> kickedPlayerIds{};
		for (const std::pair<const int, PlayerData>& player : m_players)
		{
			if (timestep > player.second.keepAliveTimestep)
			{
				kickedPlayerIds.push_back(player.first);
			}
		}
		for (int kickedPlayerId : kickedPlayerIds)
		{
			m_players.erase(kickedPlayerId);
		}

		m_mutex.unlock();

		return kickedPlayerIds;
	}

	std::unordered_map<int, PlayerData> PlayerManager::getPlayers() const
	{
		m_mutex.lock();

		std::unordered_map<int, PlayerData> players = m_players;

		m_mutex.unlock();

		return players;
	}

	void PlayerManager::killPlayer(int playerId, const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		if (m_players.contains(playerId) && !m_players.at(playerId).keepAliveLock)
		{
			m_players.at(playerId).keepAliveLock = true;
			static const Physics::Timestep timeout{5, 0};
			m_players.at(playerId).keepAliveTimestep = timestep + timeout;
		}

		m_mutex.unlock();
	}

	std::optional<int> PlayerManager::getAvailableId()
	{
		int start = m_idCounter;
		while (m_players.contains(m_idCounter))
		{
			m_idCounter = (m_idCounter + 1) % static_cast<int>(Common::maxPlayerCount);
			if (m_idCounter == start)
			{
				return std::nullopt;
			}
		}
		
		int availableId = m_idCounter;
		m_idCounter = (m_idCounter + 1) % static_cast<int>(Common::maxPlayerCount);
		return availableId;
	}
};
