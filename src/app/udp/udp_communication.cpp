#include "app/udp/udp_communication.hpp"

#include "app/player_data.hpp"
#include "app/udp/udp_frame_type.hpp"
#include "app/udp/udp_serializer.hpp"
#include "common/airplane_type_name.hpp"
#include "physics/player_info.hpp"
#include "physics/player_input.hpp"
#include "physics/timestamp.hpp"
#include "physics/timestep.hpp"

#include <asio/asio.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace App
{
	UDPCommunication::UDPCommunication(int networkThreadPort, int physicsThreadPort) :
		m_networkThreadSocket{m_networkThreadIOContext,
			asio::ip::udp::endpoint{asio::ip::udp::v4(),
			static_cast<asio::ip::port_type>(networkThreadPort)}},
		m_physicsThreadSocket{m_physicsThreadIOContext,
			asio::ip::udp::endpoint{asio::ip::udp::v4(),
			static_cast<asio::ip::port_type>(physicsThreadPort)}}
	{ }

	void UDPCommunication::sendInitResFrame(const asio::ip::udp::endpoint& endpoint,
		const Physics::Timestamp& clientTimestamp, int playerId)
	{
		std::vector<std::uint8_t> buffer{};
		UDPSerializer::serializeInitResFrame(clientTimestamp, Physics::Timestamp::systemNow(),
			playerId, buffer);

		m_networkThreadSocket.send_to(asio::buffer(buffer), endpoint);
	}

	void UDPCommunication::broadcastControlFrame(
		const std::unordered_map<int, PlayerData>& playerDatas,
		const Physics::Timestamp& clientTimestamp, const Physics::Timestep& timestep, int playerId,
		const Physics::PlayerInput& playerInput)
	{
		std::vector<std::uint8_t> buffer{};
		UDPSerializer::serializeControlFrame(clientTimestamp, Physics::Timestamp::systemNow(),
			timestep, playerId, playerInput, buffer);

		for (const std::pair<const int, PlayerData>& player : playerDatas)
		{
			m_networkThreadSocket.send_to(asio::buffer(buffer), player.second.endpoint);
		}
	}

	void UDPCommunication::broadcastStateFrame(
		const std::unordered_map<int, PlayerData>& playerDatas, const Physics::Timestep& timestep,
		const std::unordered_map<int, Physics::PlayerInfo>& playerInfos)
	{
		std::vector<std::uint8_t> buffer{};
		UDPSerializer::serializeStateFrame(timestep, playerInfos, buffer);

		for (const std::pair<const int, PlayerData>& player : playerDatas)
		{
			m_physicsThreadSocket.send_to(asio::buffer(buffer), player.second.endpoint);
		}
	}

	bool UDPCommunication::receiveInitReqOrControlFrame(asio::ip::udp::endpoint& endpoint,
		Physics::Timestamp& clientTimestamp, UDPFrameType& udpFrameType,
		Common::AirplaneTypeName& airplaneTypeName, Physics::Timestep& timestep, int& playerId,
		Physics::PlayerInput& playerInput)
	{
		static constexpr std::chrono::seconds timeout(10);
		return receiveFrameWithTimeout
		(
			endpoint,
			[&clientTimestamp, &udpFrameType, &airplaneTypeName, &timestep, &playerId, &playerInput]
			(std::vector<std::uint8_t> buffer, std::size_t receivedSize)
			{
				if (buffer.size() == 0)
				{
					return false;
				}
				if (buffer[0] == toUInt8(UDPFrameType::initReq))
				{
					udpFrameType = UDPFrameType::initReq;
					std::vector<std::uint8_t> receivedBuffer(buffer.begin(),
						buffer.begin() + static_cast<int>(receivedSize));
					UDPSerializer::deserializeInitReqFrame(receivedBuffer, clientTimestamp,
						airplaneTypeName);
					return true;
				}
				else if (buffer[0] == toUInt8(UDPFrameType::control))
				{
					udpFrameType = UDPFrameType::control;
					std::vector<std::uint8_t> receivedBuffer(buffer.begin(),
						buffer.begin() + static_cast<int>(receivedSize));
					Physics::Timestamp serverTimestamp{};
					UDPSerializer::deserializeControlFrame(receivedBuffer, clientTimestamp,
						serverTimestamp, timestep, playerId, playerInput);
					return true;
				}
				return false;
			},
			timeout
		);
	}

	bool UDPCommunication::receiveFrameWithTimeout(asio::ip::udp::endpoint& endpoint,
		std::function<bool(std::vector<std::uint8_t>, std::size_t)> frameHandler,
		const std::chrono::seconds& timeout)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point expiration = now + timeout;
		
		static std::vector<std::uint8_t> buffer(maxFrameSize);
		while (now < expiration)
		{
			std::chrono::duration<float> remainder = expiration - now;
			setReceiveSocketTimeout(remainder);

			try
			{
				std::size_t receivedSize =
					m_networkThreadSocket.receive_from(asio::buffer(buffer), endpoint);
				if (frameHandler(buffer, receivedSize))
				{
					return true;
				}
			}
			catch (std::exception&)
			{ }

			now = std::chrono::steady_clock::now();
		}
		return false;
	}

	void UDPCommunication::setReceiveSocketTimeout(const std::chrono::duration<float>& timeout)
	{
		m_networkThreadSocket.set_option(
			asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>
			{
				static_cast<int>
				(
					std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()
				)
			});
	}

	void UDPCommunication::completionHandler(std::shared_ptr<std::vector<std::uint8_t>>)
	{ }
};
