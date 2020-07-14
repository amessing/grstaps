#include "grstaps/task_planning/evaluator.hpp"

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/rpg.hpp"

namespace grstaps
{
    void Evaluator::evaluate(Plan* p, TState* state, float makespan, bool helpfulActions) {
        p->hLand = landmarks.countUncheckedNodes();
        RPG rpg(state, task, forceAtEndConditions, tilActions);
        p->h = rpg.evaluate(task->hasPermanentMutexAction());
        if (priorityGoals != nullptr) {
            p->hAux = rpg.evaluate(priorityGoals, task->hasPermanentMutexAction());
        }
    }

    void Evaluator::initialize(TState* state, SASTask* task, std::vector<SASAction*>* a, bool forceAtEndConditions) {
        this->task = task;
        this->forceAtEndConditions = forceAtEndConditions;
        tilActions = a;
        if (state == nullptr) landmarks.initialize(task, a);
        else landmarks.initialize(state, task, a);
    }

    bool Evaluator::informativeLandmarks() {
        return landmarks.getNumInformativeNodes() > 0;
    }


    float Evaluator::evaluateCG(TState* state) {
        return 0;
    }
}
