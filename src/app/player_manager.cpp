#include "app/player_manager.hpp"

#include "common/config.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <utility>
#include <vector>

namespace App
{
	PlayerManager::PlayerManager()
	{
		for (KickQueue::iterator& iter : m_kickQueueIterators)
		{
			iter = m_kickQueue.end();
		}
	}

	int PlayerManager::getPlayerId(const asio::ip::udp::endpoint& endpoint) const
	{
		int playerId = -1;
		for (const std::pair<const int, PlayerData>& player : m_players)
		{
			if (player.second.endpoint == endpoint)
			{
				playerId = player.first;
				break;
			}
		}
		return playerId;
	}
	
	int PlayerManager::addNewPlayer(const asio::ip::udp::endpoint& endpoint,
		const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		int newPlayerId = getAvailableId();
		if (newPlayerId != -1)
		{
			m_players.insert({newPlayerId, PlayerData{endpoint}});
			m_kickQueue.push_back(KickQueueElement{newPlayerId, timestep, false});
			m_kickQueueIterators.at(static_cast<unsigned int>(newPlayerId)) =
				std::prev(m_kickQueue.end());
		}

		m_mutex.unlock();

		return newPlayerId;
	}

	void PlayerManager::bumpPlayer(int playerId, const Physics::Timestep& timestep)
	{
		if (m_kickQueueIterators.at(static_cast<unsigned int>(playerId))->isDead)
		{
			return;
		}
		m_kickQueue.erase(m_kickQueueIterators.at(static_cast<unsigned int>(playerId)));
		m_kickQueue.push_back(KickQueueElement{playerId, timestep});
		m_kickQueueIterators.at(static_cast<unsigned int>(playerId)) = std::prev(m_kickQueue.end());
	}

	std::vector<int> PlayerManager::kickPlayers(const Physics::Timestep& timestep)
	{
		m_mutex.lock();

		std::vector<int> inactivePlayerIds{};
		static const Physics::Timestep timeout{10, 0};
		while (!m_kickQueue.empty() && timestep > m_kickQueue.front().lastFrameTimestep + timeout)
		{
			int playerId = m_kickQueue.front().playerId;
			m_kickQueueIterators.at(static_cast<unsigned int>(playerId)) = m_kickQueue.end();
			m_kickQueue.erase(m_kickQueue.begin());
			m_players.erase(playerId);
			inactivePlayerIds.push_back(playerId);
		}

		m_mutex.unlock();

		return inactivePlayerIds;
	}

	std::unordered_map<int, PlayerData> PlayerManager::getPlayers() const
	{
		m_mutex.lock();

		std::unordered_map<int, PlayerData> players = m_players;

		m_mutex.unlock();

		return players;
	}

	void PlayerManager::killPlayer(int playerId)
	{
		m_mutex.lock();

		m_kickQueueIterators.at(playerId)->isDead = true;

		m_mutex.unlock();
	}

	int PlayerManager::getAvailableId()
	{
		int start = m_idCounter;
		while (m_players.contains(m_idCounter))
		{
			m_idCounter = (m_idCounter + 1) % static_cast<int>(Common::maxPlayerCount);
			if (m_idCounter == start)
			{
				return -1;
			}
		}
		
		int availableId = m_idCounter;
		m_idCounter = (m_idCounter + 1) % static_cast<int>(Common::maxPlayerCount);
		return availableId;
	}
};
