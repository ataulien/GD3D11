#pragma once

#include <wrl.h>
#include <exception>

class BasicTimer
{
private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_currentTime;
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_lastTime;
    float m_total;
    float m_delta;


public:
    BasicTimer()
    {
        if (!QueryPerformanceFrequency(&m_frequency))
        {
            throw std::exception();
        }

        Reset();
    }
    
    void Reset()
    {
		m_total = 0;
		m_delta = 1.0f / 60.0f;

		Update();

		m_startTime = m_currentTime;
    }
    
    void Update()
    {


        if (!QueryPerformanceCounter(&m_currentTime))
        {
            throw std::exception();
        }
        
        m_total = static_cast<float>(
            static_cast<double>(m_currentTime.QuadPart-m_startTime.QuadPart) /
            static_cast<double>(m_frequency.QuadPart)
            );
        
        if (m_lastTime.QuadPart==m_startTime.QuadPart)
        {
            // If the timer was just reset, report a time delta equivalent to 60Hz frame time.
            m_delta = 1.0f / 60.0f;
        }
        else
        {
            m_delta = static_cast<float>(
                static_cast<double>(m_currentTime.QuadPart-m_lastTime.QuadPart) /
                static_cast<double>(m_frequency.QuadPart)
                );
        }
        
        m_lastTime = m_currentTime;
    }
    


    float GetTotal() const
	{  
		return m_total;
	}
    
    float GetDelta() const
	{ 
		return m_delta;
	}
    
};