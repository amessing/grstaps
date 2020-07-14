#include "grstaps/task_planning/causal_link.hpp"

namespace grstaps
{
    CausalLink::CausalLink()
    {
    }

    CausalLink::CausalLink(TVariable var, TValue v, TTimePoint p1, TTimePoint p2)
    {
        varValue = (((TVarValue)v) << 16) + var;
        ordering = (((TOrdering)p2) << 16) + p1;
    }

    CausalLink::CausalLink(TVarValue vv, TTimePoint p1, TTimePoint p2)
    {
        varValue = vv;
        ordering = (((TOrdering)p2) << 16) + p1;
    }
}

