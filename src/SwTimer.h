#ifndef _SW_TIMER_H_
#define _SW_TIMER_H_

#include <cstdint>

#define S_TO_MS(x)   (x * 1000U)
#define MIN_TO_MS(x) (S_TO_MS(x * 60U))

class SwDownTimer 
{
    public:
        SwDownTimer(uint32_t timerLength, uint32_t timeStepLength) : m_timerPause(false),
                                                                     m_timerLength(timerLength),
                                                                     m_timeStepLength(timeStepLength), 
                                                                     m_timerCount(m_timerLength) {}
        SwDownTimer() : SwDownTimer(0U, 0U) {}
        ~SwDownTimer() {}

        uint32_t getCount() const
        {
            return m_timerCount;
        }

        bool isExpired() const
        {
            return (m_timerCount == 0U);
        }

        void reset()
        {
            m_timerCount = m_timerLength;
        }

        void reset(uint32_t timerLength)
        {
            m_timerLength = timerLength;
            m_timerCount = m_timerLength;
        }

        void pause()
        {
            m_timerPause = true;
        }

        void resume()
        {
            m_timerPause = false;
        }

        void expire()
        {
            m_timerCount = 0U;
        }

        bool updateAndCheckTimer()
        {
            return updateAndCheckTimer(m_timeStepLength);
        }

        bool updateAndCheckTimer(uint32_t timeStepLength)
        {
            if (!m_timerPause)
            {
                if (m_timerCount > m_timerLength)
                {
                    m_timerCount = m_timerLength;
                }
                
                if (m_timerCount < m_timeStepLength)
                {
                    m_timerCount = 0U;
                }
                else
                {
                    m_timerCount -= m_timeStepLength;
                }
            }
            
            return (m_timerCount == 0U);
        }

    private:
        bool m_timerPause;
        uint32_t m_timerLength;
        uint32_t m_timeStepLength;
        uint32_t m_timerCount;
};


class SwUpTimer 
{
    public:
        SwUpTimer(uint32_t timerLength, uint32_t timeStepLength) : m_timerPause(false),
                                                                   m_timerLength(timerLength),
                                                                   m_timeStepLength(timeStepLength), 
                                                                   m_timerCount(0U) {}
        SwUpTimer() : SwUpTimer(0U, 0U) {}
        ~SwUpTimer() {}

        uint32_t getCount() const
        {
            return m_timerCount;
        }

        bool isExpired() const
        {
            return (m_timerCount == m_timerLength);
        }

        void reset()
        {
            m_timerCount = 0U;
        }

        void pause()
        {
            m_timerPause = true;
        }

        void resume()
        {
            m_timerPause = false;
        }

        void reset(uint32_t timerLength)
        {
            m_timerCount = 0U;
            m_timerLength = timerLength;
        }

        void setMaxLength(uint32_t timerLength)
        {
            m_timerLength = timerLength;
        }

        bool updateAndCheckTimer()
        {
            return updateAndCheckTimer(m_timeStepLength);
        }

        bool updateAndCheckTimer(uint32_t timeStepLength)
        {
            if (!m_timerPause)
            {
                if (m_timerCount >= m_timerLength)
                {
                    m_timerCount = m_timerLength;
                } else if ((m_timerLength - m_timerCount) < m_timeStepLength)
                {
                    m_timerCount = m_timerLength;
                }
                else
                {
                    m_timerCount += m_timeStepLength;
                }
            }
            
            return (m_timerCount == m_timerLength);
        }

    private:
        bool m_timerPause;
        uint32_t m_timerLength;
        uint32_t m_timeStepLength;
        uint32_t m_timerCount;
};


#endif  // _SW_TIMER_H_