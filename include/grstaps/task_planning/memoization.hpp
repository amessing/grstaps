#ifndef GRSTAPS_MEMOIZATION_HPP
#define GRSTAPS_MEMOIZATION_HPP

#include <unordered_map>

#include "grstaps/task_planning/linearizer.hpp"

namespace grstaps
{
    class Plan;
    class SASTask;
    class TState;

    class Memoization
    {
    private:
        SASTask* task;
        TState* initialState;
        std::unordered_map<uint64_t, std::vector<Plan*>*> memo;
        Linearizer linearizer;

        bool sameState(TState* state, Plan* pc);

    public:
        Memoization();

        void initialize(SASTask* task);

        bool isRepeatedState(Plan* p, TState* state);

        void clear();
    };
}
#endif //GRSTAPS_MEMOIZATION_HPP
