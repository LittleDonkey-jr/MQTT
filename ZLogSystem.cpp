#include "ZLogSystem.h"
LogSystem* LogSystem::m_log_system = nullptr;
std::mutex LogSystem::m_mtx;


LogSystem* LogSystem::getInstance()
{
	if (m_log_system == nullptr)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		if (m_log_system == nullptr)
		{
			m_log_system = new LogSystem();
		}
	}
	return m_log_system;
}

void LogSystem::setLogLevel(loglevel level) {
	m_level = level;
}

void LogSystem::log_Info(const char* format, ...)
{
	if (m_level <= level_info)
	{
		va_list vaList;
		va_start(vaList, format); // 初始化可变参数列表

		// 使用vsnprintf格式化并写入缓冲区
		int m = vsnprintf(this->BeginWrite(), this->WritableBytes(), format, vaList);
		va_end(vaList); // 清理可变参数列表

		if (m < this->WritableBytes())
		{
			std::cout << GREEN << m_buffer << RESET;
		}
		else
		{
			std::cout << RED << "m_buffer overflow" << RESET << std::endl;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
	}
}

void LogSystem::log_Warning(const char* format, ...)
{
	if (m_level <= level_warning)
	{
		va_list vaList;
		va_start(vaList, format); // 初始化可变参数列表

		// 使用vsnprintf格式化并写入缓冲区
		int m = vsnprintf(this->BeginWrite(), this->WritableBytes(), format, vaList);
		va_end(vaList); // 清理可变参数列表

		if (m < this->WritableBytes())
		{
			std::cout << BLUE << m_buffer << RESET;
		}
		else
		{
			std::cout << RED << "m_buffer overflow" << RESET << std::endl;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
	}
}

void LogSystem::log_Error(const char* format, ...)
{
	if (m_level <= level_error)
	{
		va_list vaList;
		va_start(vaList, format); // 初始化可变参数列表

		// 使用vsnprintf格式化并写入缓冲区
		int m = vsnprintf(this->BeginWrite(), this->WritableBytes(), format, vaList);
		va_end(vaList); // 清理可变参数列表

		if (m < this->WritableBytes())
		{
			std::cout << RED << m_buffer << RESET;
		}
		else
		{
			std::cout << RED << "m_buffer overflow" << RESET << std::endl;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
	}
}