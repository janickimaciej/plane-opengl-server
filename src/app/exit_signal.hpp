#pragma once

#include "app/exit_code.hpp"

#include <atomic>
#include <mutex>
#include <semaphore>

namespace App
{
	class ExitSignal
	{
	public:
		bool shouldStop() const;
		void exit(ExitCode exitCode);
		ExitCode getExitCode() const;

	private:
		std::atomic<bool> m_exiting = false;
		ExitCode m_exitCode{};
		std::mutex m_mutex{};
	};
};
