#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <stdint.h>

struct DataPoint_1D
{
    float x;
    float y;
};

class Table1D
{
public:
    Table1D(const DataPoint_1D* dataPoints, const uint32_t tableLen) :
           m_table(dataPoints),
           m_tableLen(tableLen)
           {}

    float Lookup(const float x)
    {
        const float xStart {m_table[0U].x};
        const float xEnd {m_table[m_tableLen - 1U].x};

        if (x < xStart)
        {
            m_startIdx = 0;
            return m_table[0U].y;
        }
        else if (x > xEnd)
        {
            m_startIdx = (m_tableLen - 2U);
            return m_table[m_tableLen - 1U].y;;
        }

        int32_t pt1_idx = m_startIdx;
        int32_t pt2_idx = pt1_idx + 1;
        const bool searchTowardsEnd = m_table[pt1_idx].x < x;

        while (pt1_idx >= 0 && pt2_idx < m_tableLen)
        {
            if (m_table[pt1_idx].x < x && m_table[pt2_idx].x > x)
            {
                // x is now between data points 1 and 2
                break;
            }

            if (searchTowardsEnd)
            {
                pt1_idx++;
                pt2_idx++;
            }
            else
            {
                pt1_idx--;
                pt2_idx--;
            }
        }

        m_startIdx = pt1_idx;
        const float interpolatedRaw = Interpolate1D(&m_table[pt1_idx], &m_table[pt2_idx], x);

        const float minLimit = min( m_table[m_tableLen - 1U].y, m_table[0U].y );
        const float maxLimit = max( m_table[m_tableLen - 1U].y, m_table[0U].y );
        return constrain(interpolatedRaw, minLimit, maxLimit);
    }

    float Interpolate1D(const DataPoint_1D* pt1, const DataPoint_1D* pt2, float x)
    {
        const float dx {(pt2->x) - (pt1->x)};
        const float dy {(pt2->y) - (pt1->y)};

        // printf("Interp %f between (%f, %f) and (%f, %f): %f\n", x, pt1->x, pt1->y, pt2->x, pt2->y, (((x - pt1->x) / dx) * dy) + pt1->y);

        if (dx == 0.0F)
        {
            return pt2->y;
        }
        else
        {
            return  (((x - pt1->x) / dx) * dy) + pt1->y;
        }
    }

private:
    const DataPoint_1D* m_table;
    const uint32_t m_tableLen;

    int32_t m_startIdx {0};
};


#endif // LOOKUPTABLE_H