#pragma once

#include "physics/timestep.hpp"

namespace App
{
	struct KickQueueElement
	{
		int playerId{};
		Physics::Timestep lastFrameTimestep{};
		bool isDead{};
	};
};
