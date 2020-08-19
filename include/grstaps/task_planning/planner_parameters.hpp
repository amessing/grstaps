#ifndef PLANNER_PARAMETERS_HPP
#define PLANNER_PARAMETERS_HPP

namespace grstaps
{
    struct PlannerParameters
    {
        float total_time;
        char *domainFileName;
        char *problemFileName;
        char *outputFileName;
        bool generateGroundedDomain;
        bool keepStaticData;
        bool noSAS;
        bool generateMutexFile;
        bool generateTrace;
        PlannerParameters()
            : total_time(0)
            , domainFileName(nullptr)
            , problemFileName(nullptr)
            , outputFileName(nullptr)
            , generateGroundedDomain(false)
            , keepStaticData(false)
            , noSAS(false)
            , generateMutexFile(false)
            , generateTrace(false)
        {}
    };
}  // namespace grstaps

#endif  // PLANNER_PARAMETERS_HPP
