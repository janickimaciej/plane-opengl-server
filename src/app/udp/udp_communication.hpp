#pragma once

#include "app/udp/udp_frame_type.hpp"
#include "common/airplane_type_name.hpp"
#include "physics/player_info.hpp"
#include "physics/player_input.hpp"
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
		void sendControlFrame(const asio::ip::udp::endpoint& endpoint,
			const Physics::Timestamp& clientTimestamp, const Physics::Timestep& timestep,
			int playerId, const Physics::PlayerInput& playerInput);
		void sendStateFrame(const asio::ip::udp::endpoint& endpoint,
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
