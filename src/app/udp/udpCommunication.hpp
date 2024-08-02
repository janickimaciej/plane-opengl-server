#pragma once

#include "app/playerData.hpp"
#include "app/udp/udpFrameType.hpp"
#include "common/airplaneTypeName.hpp"
#include "physics/playerInfo.hpp"
#include "physics/playerInput.hpp"
#include "physics/timestamp.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace App
{
	class UDPCommunication
	{
	public:
		UDPCommunication(int networkThreadPort, int physicsThreadPort);

		void sendInitResFrame(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestamp& clientTimestamp, int playerId);
		void broadcastControlFrame(const std::unordered_map<int, PlayerData>& playerDatas,
			const Physics::Timestamp& clientTimestamp, const Physics::Timestep& timestep,
			int playerId, const Physics::PlayerInput& playerInput);
		void broadcastStateFrame(const std::unordered_map<int, PlayerData>& playerDatas,
			const Physics::Timestep& timestep,
			const std::unordered_map<int, Physics::PlayerInfo>& playerInfos);

		bool receiveInitReqOrControlFrame(asio::ip::udp::endpoint& endpoint,
			Physics::Timestamp& clientTimestamp, UDPFrameType& udpFrameType,
			Common::AirplaneTypeName& airplaneTypeName, Physics::Timestep& timestep, int& playerId,
			Physics::PlayerInput& playerInput);

	private:
		asio::io_context m_networkThreadIOContext{};
		asio::ip::udp::socket m_networkThreadSocket;
		
		asio::io_context m_physicsThreadIOContext{};
		asio::ip::udp::socket m_physicsThreadSocket;

		bool receiveFrameWithTimeout(asio::ip::udp::endpoint& endpoint,
			std::function<bool(std::vector<std::uint8_t>, std::size_t)> frameHandler,
			const std::chrono::seconds& timeout);
		void setReceiveSocketTimeout(const std::chrono::duration<float>& timeout);
		static void completionHandler(std::shared_ptr<std::vector<std::uint8_t>>);
	};
};
