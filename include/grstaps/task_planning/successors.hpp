#ifndef GRSTAPS_SUCCESSORS_HPP
#define GRSTAPS_SUCCESSORS_HPP

#include <vector>

#include "grstaps/task_planning/causal_link.hpp"
#include "grstaps/task_planning/evaluator.hpp"
#include "grstaps/task_planning/memoization.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
#define INITAL_MATRIX_SIZE    400
#define MATRIX_INCREASE        200

    class Linearizer;
    class Plan;

    class PlanEffect
    {
    public:
        std::vector <TTimePoint> timePoints;    // Points of time where this effect is produced
        // Dividing the time point by 2 we get the number of the step in the plan (each step has two time points: start and end)
        unsigned int iteration;                // The information is valid only if the iteration matches with the current one

        void add(TTimePoint time, unsigned int iteration);
    };

    class VarChange
    {
    public:
        std::vector <TValue> values;            // Value that the variable takes
        std::vector <TTimePoint> timePoints;    // Points of time where this effect is produced
        unsigned int iteration;                // The information is valid only if the iteration matches with the current one

        void add(TValue v, TTimePoint time, unsigned int iteration);
    };

    class Threat
    {
    public:
        TTimePoint p1;
        TTimePoint p2;
        TTimePoint tp;
        TVariable var;

        Threat(TTimePoint c1, TTimePoint c2, TTimePoint p, TVariable v);
    };

    class PlanBuilder
    {
    private:
        std::vector <TTimePoint> prevPoints;    // For internal calculations
        std::vector <TTimePoint> nextPoints;    // For internal calculations
        Linearizer* linearizer;

    public:
        SASAction* action;                    // New action added
        unsigned int currentPrecondition;
        unsigned int currentEffect;
        unsigned int setPrecondition;
        std::vector <CausalLink> causalLinks;
        std::vector<unsigned int> numOrderingsAdded;
        std::vector <TOrdering> orderings;
        TTimePoint lastTimePoint;
        std::vector<unsigned int> openCond;

        PlanBuilder(SASAction* a, Linearizer* linearizer, TStep lastStep);

        bool addLink(SASCondition* c, TTimePoint p1, TTimePoint p2);

        bool addLink(TVarValue vv, TTimePoint p1, TTimePoint p2);

        bool addOrdering(TTimePoint p1, TTimePoint p2);

        void removeLastLink();

        void removeLastOrdering();

        Plan* generatePlan(Plan* basePlan, uint32_t idPlan);
    };

    class Successors
    {
    private:
        SASTask* task;
        bool forceAtEndConditions;
        unsigned int numVariables;                            // Number of variables
        unsigned int numActions;                            // Number of grounded actions
        Plan* basePlan;                                        // Base plan
        PlanEffect** planEffects;                            // Plan effects: (var, value) -> PlanEffect
        VarChange* varChanges;                                // Variable changes: var -> VarChange
        TStep newStep;                                        // New step to add as successor
        std::vector<Plan*>* successors;                        // Vector to return the sucessor plans
        std::vector <TTimePoint> prevPoints;                    // For internal calculations
        std::vector <TTimePoint> nextPoints;                    // For internal calculations
        uint32_t idPlan;                                    // Plan counter
        Linearizer linearizer;                                // Linearizes plans to schedule them in time and compute heuristics
        Evaluator evaluator;
        //TState* basePlanState;
        Memoization memoization;
        bool filterRepeatedStates;
        std::vector<unsigned int> checkedAction;
        unsigned int currentIteration;
        bool helpfulActions;

        inline bool visitedAction(SASAction* a)
        { return checkedAction[a->index] == currentIteration; }

        inline void setVisitedAction(SASAction* a)
        { checkedAction[a->index] = currentIteration; }

        void computeBasePlanEffects();                        // Fill the planEffects matrix with the effects produced by the base plan
        void fullActionCheck(SASAction* a);

        void fullActionSupportCheck(PlanBuilder* pb);

        inline bool supportedAction(const SASAction* a)
        {
            for(unsigned int i = 0; i < a->startCond.size(); i++)
            {
                if(!supportedCondition(a->startCond[i]))
                {
                    return false;
                }
            }
            for(unsigned int i = 0; i < a->overCond.size(); i++)
            {
                if(!supportedCondition(a->overCond[i]))
                {
                    return false;
                }
            }
            if(forceAtEndConditions)
            {
                for(unsigned int i = 0; i < a->endCond.size(); i++)
                {
                    if(!supportedCondition(a->endCond[i]))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        inline bool supportedCondition(const SASCondition& c)
        {
            return planEffects[c.var][c.value].iteration == linearizer.getIteration();
        }

        void fullCondtionSupportCheck(PlanBuilder* pb,
                                      SASCondition* c,
                                      TTimePoint condPoint,
                                      bool overAll,
                                      bool canLeaveOpen);

        void generateSuccessor(PlanBuilder* pb);

        void reuseAction(Plan* plan);

        void computeSuccessorsThroughBrotherPlans();

        void computeSuccessorsSupportedByLastActions();

        void computeSolutionSuccessors();

        void checkTheatsBetweenCausalLinkInBasePlanWithNewAction(PlanBuilder* pb);

        unsigned int addActionSupport(PlanBuilder* pb,
                                      TVariable var,
                                      TValue value,
                                      TTimePoint effectTime,
                                      TTimePoint startTimeNewAction);

        void solveThreats(PlanBuilder* pb, std::vector <Threat>* threats);

        bool postprocessPlan(Plan* p);

        void addSuccessor(Plan* p);

        void solveBasePlanOpenConditionIfPossible(unsigned int condNumber, PlanBuilder* pb);

        bool mutexPoints(TTimePoint p1, TTimePoint p2, TVariable var, PlanBuilder* pb);

        inline SASCondition* getRequiredValue(TTimePoint p, SASAction* a, TVariable var)
        {
            std::vector <SASCondition>* cond = (p & 1) == 0 ? &(a->startCond) : &(a->endCond);
            for(unsigned int i = 0; i < cond->size(); i++)
            {
                if((*cond)[i].var == var)
                { return &((*cond)[i]); }
            }
            cond = &(a->overCond);    // Check over-all conditions then
            for(unsigned int i = 0; i < cond->size(); i++)
            {
                if((*cond)[i].var == var)
                { return &((*cond)[i]); }
            }
            return nullptr;
        }

        inline SASCondition* getEffectValue(TTimePoint p, SASAction* a, TVariable var)
        {
            std::vector <SASCondition>* eff = (p & 1) == 0 ? &(a->startEff) : &(a->endEff);
            for(unsigned int i = 0; i < eff->size(); i++)
            {
                if((*eff)[i].var == var)
                { return &((*eff)[i]); }
            }
            return nullptr;
        }

        bool goalsSupported(TState* s);

        bool actionSupported(SASAction* a, TState* s);

        void checkContradictoryEffects(PlanBuilder* pb);

        void checkContradictoryEffects(PlanBuilder* pb, SASCondition* c, TTimePoint effPoint);

        bool meetDeadlines();

    public:
        Plan* solution;

        void initialize(TState* state,
                        SASTask* task,
                        bool forceAtEndConditions,
                        bool filterRepeatedStates,
                        std::vector<SASAction*>* tilActions);

        ~Successors();

        void computeSuccessors(Plan* base, std::vector<Plan*>* suc);

        void computeSuccessorsConcurrent(Plan* base, std::vector<Plan*>* suc);

        inline bool unsorted(TTimePoint p1, TTimePoint p2)
        {
            return !linearizer.existOrder(p1, p2) && !linearizer.existOrder(p2, p1);
        }

        bool informativeLandmarks();

        void clearMemoization();

        void clear();

        void evaluate(Plan* p);

        void printState(Plan* p);

        TState* getFrontierState(Plan* p);

        bool getForceAtEndConditions()
        { return forceAtEndConditions; }

        std::vector<SASAction*>* getTILActions()
        { return evaluator.getTILActions(); }

        void setPriorityGoals(std::vector <TVarValue>* priorityGoals)
        { evaluator.setPriorityGoals(priorityGoals); }
    };
}

#endif //GRSTAPS_SUCCESSORS_HPP
