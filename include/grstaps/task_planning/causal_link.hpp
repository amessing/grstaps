#ifndef GRSTAPS_CAUSAL_LINK_HPP
#define GRSTAPS_CAUSAL_LINK_HPP

// local
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    /**
     * Container for a causal link
     */
    class CausalLink
    {
    public:
        TOrdering ordering;                    // New orderings (first time point [lower 16 bits] -> second time point [higher 16 bits])
        TVarValue varValue;                    // Variable [lower 16 bits] = value (higher 16 bits)
        inline TTimePoint firstPoint()
        {
            return ordering & 0xFFFF;
        }

        inline TTimePoint secondPoint()
        {
            return ordering >> 16;
        }

        inline TVariable getVar()
        {
            return varValue & 0xFFFF;
        }

        inline TValue getValue()
        {
            return varValue >> 16;
        }

        CausalLink();

        CausalLink(TVariable var, TValue v, TTimePoint p1, TTimePoint p2);

        CausalLink(TVarValue vv, TTimePoint p1, TTimePoint p2);
    };
}

#endif //GRSTAPS_CAUSAL_LINK_HPP
