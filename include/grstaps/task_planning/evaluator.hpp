#ifndef GRSTAPS_EVALUATOR_HPP
#define GRSTAPS_EVALUATOR_HPP

#include <vector>

#include "grstaps/task_planning/landmark_heuristic.hpp"

namespace grstaps
{
    class Plan;
    class SASAction;
    class SASTask;
    class TState;

    class Evaluator
    {
    private:
        SASTask* task;
        LandmarkHeuristic landmarks;
        std::vector<SASAction*>* tilActions;
        bool forceAtEndConditions;
        std::vector <TVarValue>* priorityGoals;

    public:
        void initialize(TState* state, SASTask* task, std::vector<SASAction*>* a, bool forceAtEndConditions);

        void evaluate(Plan* p, TState* state, float makespan, bool helpfulActions);

        inline LandmarkHeuristic* getLandmarkHeuristic()
        {
            return &landmarks;
        }

        float evaluateWithoutContext(TState* state);

        bool informativeLandmarks();

        float evaluateCG(TState* state);

        std::vector<SASAction*>* getTILActions()
        {
            return tilActions;
        }

        void setPriorityGoals(std::vector <TVarValue>* priorityGoals)
        {
            this->priorityGoals = priorityGoals;
        }
    };
}

#endif //GRSTAPS_EVALUATOR_HPP
