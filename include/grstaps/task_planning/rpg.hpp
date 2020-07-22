#ifndef GRSTAPS_RPG_HPP
#define GRSTAPS_RPG_HPP

#include "grstaps/task_planning/priority_queue.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class SASAction;
    class SASCondition;
    class SASTask;
    class TState;

    class RPGCondition : public PriorityQueueItem {
    public:
        TVariable var;
        TValue value;
        int level;
        RPGCondition(TVariable v, TValue val, int l) {
            var = v;
            value = val;
            level = l;
        }
        inline int compare(PriorityQueueItem* other) {
            return ((RPGCondition*)other)->level - level;
        }
    };

    class RPGVarValue {
    public:
        TVariable var;
        TValue value;
        RPGVarValue(TVariable var, TValue value);
    };

    class RPG {
    private:
        SASTask* task;
        bool forceAtEndConditions;
        std::vector< std::vector<int> > literalLevels;
        std::vector<int> actionLevels;
        unsigned int numLevels;
        std::vector<RPGVarValue>* lastLevel;
        std::vector<RPGVarValue>* newLevel;
        std::vector<TVarValue> reachedValues;

        void initialize();
        void addEffects(SASAction* a);
        void addEffect(TVariable var, TValue value);
        void expand();
        void addSubgoals(std::vector<TVarValue>* goals, PriorityQueue* openConditions);
        void addSubgoal(TVariable var, TValue value, PriorityQueue* openConditions);
        void addSubgoals(SASAction* a, PriorityQueue* openConditions);
        uint16_t getDifficulty(SASAction* a);
        uint16_t getDifficulty(SASCondition* c);
        uint16_t getDifficultyWithPermanentMutex(SASAction* a);
        void addTILactions(std::vector<SASAction*>* tilActions);
        void addUsefulAction(SASAction* a, std::vector<SASAction*>* usefulActions);
        uint16_t computeHeuristic(bool mutex, PriorityQueue* openConditions);
        void resetReachedValues();

    public:
        std::vector<SASAction*> relaxedPlan;

        RPG(const std::vector< std::vector<TValue> > &varValues, SASTask* task, bool forceAtEndConditions,
            std::vector<SASAction*>* tilActions);
        RPG(TState* state, SASTask* task, bool forceAtEndConditions, std::vector<SASAction*>* tilActions);
        bool isExecutable(SASAction* a);
        uint16_t evaluate(bool mutex);
        uint16_t evaluate(TVarValue goal, bool mutex);
        uint16_t evaluate(std::vector<TVarValue>* goals, bool mutex);
        bool isReachable(TVariable v, TValue val) { return literalLevels[v][val] < MAX_INT32; }
    };
}

#endif //GRSTAPS_RPG_HPP
