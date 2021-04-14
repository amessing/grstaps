#ifndef GRSTAPS_LINEARIZER_HPP
#define GRSTAPS_LINEARIZER_HPP

#include <string>
#include <unordered_map>

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/priority_queue.hpp"
#include "grstaps/task_planning/utils.hpp"

#include <nlohmann/json.hpp>

namespace grstaps
{
    class LandmarkCheck;

    class LandmarkHeuristic;
    class TState;

#define INITAL_MATRIX_SIZE    400
#define MATRIX_INCREASE        200

    class LinearStep
    {
    public:
        double time;
        std::string actionName;
        double duration;

        LinearStep(double t, std::string n, double d)
        {
            time = t;
            actionName = n;
            duration = d;
        }

        bool operator<(const LinearStep& s)
        {
            return time < s.time;
        }
    };

    class ScheduledPoint : public PriorityQueueItem
    {
    public:
        TTimePoint p;
        double time;
        Plan* plan;

        ScheduledPoint(TTimePoint tp, double t, Plan* pl)
        {
            p = tp;
            time = t;
            plan = pl;
        }

        virtual inline int compare(PriorityQueueItem* other)
        {
            double otherTime = ((ScheduledPoint*)other)->time;
            if(time < otherTime)
            { return -1; }
            else if(time > otherTime)
            { return 1; }
            else
            { return 0; }
        }
    };

    class Linearizer
    {
    private:
        SASTask* task;
        Plan* plan;                                            // Current plan
        Plan* basePlan;                                        // Current base plan
        std::vector<Plan*> basePlanComponents;                // The base plan is made up by incremental components, which are stored in this vector
        std::vector<std::vector<unsigned int>> matrix;    // Orders between time points in the current plan
        unsigned int iteration;                                // Current iteration
        double* time;                                        // Starting time of each time step (for computing the frontier state)
        double* duration;                                    // Duration of the actions in the plan
        std::vector<LandmarkCheck*> openNodes;                // For hLand calculation
        TState* initialState;
        std::unordered_map<double, TTimePoint> numericMutex;
        PriorityQueue pq;

        void computeBasePlanSubcomponents(Plan* base);        // Fills the basePlanComponents vector
        void computeOrderMatrix();                            // Computes the order relationships among time points
        void resizeMatrix();                                // Makes the order matrix larger
        unsigned int topologicalOrder(TTimePoint orig,
                                      std::vector<TTimePoint>* linearOrder,
                                      unsigned int pos,
                                      std::vector<bool>* visited);

        double computeActionDuration(TStep step, TState* state);

        TState* copyInitialState(SASTask* task);

        void initializeTimeArray(unsigned int numTimeSteps);

        //bool schedulePlan(std::vector<TTimePoint>* linearOrder, unsigned int numTimeSteps, TState* state, LandmarkHeuristic* hLand);
        void updateState(TTimePoint p, SASAction* a, TState* state, double dur);

        bool checkNumericConditions(TTimePoint p, SASAction* a, TState* state, double dur);

        //bool checkActionDelayNeeded();
        bool checkTopologicalOrder(std::vector<TTimePoint>* linearOrder);

        void initializeOpenNodes(LandmarkHeuristic* hLand);

        bool findOpenNode(LandmarkCheck* l);

        bool checkNumericMutexWithStartPoint(TTimePoint p, TTimePoint prev, SASAction* a);

        void initialPlanSchedule(std::vector<TTimePoint>* linearOrder, unsigned int numTimeSteps);

        void updateNumState(TTimePoint p, SASAction* a, float* numState, double dur);

        void fixScheduledTimesForGoal(unsigned int numTimeSteps,
                                      TState* state,
                                      bool* repeat,
                                      bool* invalidPlan,
                                      std::vector<TTimePoint>* linearOrder);

        void fixScheduledTimes(unsigned int numTimeSteps,
                               TState* state,
                               bool* repeat,
                               bool* invalidPlan,
                               std::vector<TTimePoint>* linearOrder);

        bool delayStartTimePointKeepingEnd(ScheduledPoint* p);

        bool delayTimePoints(ScheduledPoint* p);

        bool checkNumericMutex(TTimePoint p1, SASAction* a1, TTimePoint p2, SASAction* a2);

        void checkUnsatisfiedConditions(double currentTime, TState* state, std::vector<TTimePoint>* unsatisfiedNumCond);

        void computeAchievedLandmarks(unsigned int numTimeSteps, LandmarkHeuristic* hLand);

        inline SASAction* getAction(TStep step)
        {
            return step < basePlanComponents.size() ? basePlanComponents[step]->action : plan->action;
        }

        inline SASAction* getLastAction()
        {
            return plan != nullptr ? plan->action : basePlanComponents.back()->action;
        }

        inline Plan* getPlan(TStep step)
        {
            return step < basePlanComponents.size() ? basePlanComponents[step] : plan;
        }

        bool checkValidInitialSchedule(std::vector<TTimePoint>* linearOrder);

        bool checkNumericMutex(std::vector<SASNumericEffect>* e1, std::vector<SASNumericEffect>* e2);

        bool checkNumericMutex(std::vector<SASNumericCondition>* c, std::vector<SASNumericEffect>* e);

        bool variableInCondition(TVariable v, SASNumericCondition* c);

        bool variableInNumericExpression(TVariable v, SASNumericExpression* e);

        bool checkSolution(unsigned int numTimeSteps);

        bool checkConditions(std::vector<SASCondition>* c, TState* state);

        bool checkNumericConditions(std::vector<SASNumericCondition>* c, TState* state, double dur);

        void updateState(std::vector<SASCondition>* e, TState* state);

        void updateState(std::vector<SASNumericEffect>* e, TState* state, double dur);

        friend class TaskPlanner;

    public:
        double makespan;
        //bool debug = false;

        Linearizer();

        void setInitialState(TState* initialState, SASTask* task);

        void setCurrentBasePlan(Plan* plan);

        inline void setCurrentPlan(Plan* plan)
        {
            this->plan = plan;
        }

        inline bool checkIteration(unsigned int it)
        {
            return it == iteration;
        }

        inline bool existOrder(TTimePoint t1, TTimePoint t2)
        {
            return matrix[t1][t2] == iteration;
        }

        //inline bool existOrderExt(TTimePoint t1, TTimePoint t2) { return matrix[t1][t2] == iteration || t1 == 1; }
        inline void clearOrder(TTimePoint t1, TTimePoint t2)
        {
            matrix[t1][t2] = 0;
        }

        inline void setOrder(TTimePoint t1, TTimePoint t2)
        {
            matrix[t1][t2] = iteration;
        }

        inline unsigned int numComponents()
        {
            return basePlanComponents.size();
        }

        inline Plan* getComponent(unsigned int i)
        {
            return basePlanComponents[i];
        }

        inline unsigned int getIteration()
        {
            return iteration;
        }

        void topologicalOrder(std::vector<TTimePoint>* linearOrder);

        TState* linearize(unsigned int numActions, unsigned int numTimeSteps, SASTask* task, LandmarkHeuristic* hLand);

        TState* getFrontierState(SASTask* task, LandmarkHeuristic* hLand); //, double* timeNewStep);
        std::string planToPDDL(Plan* p, SASTask* task);
        nlohmann::json scheduleAsJson(Plan* p, SASTask* task);
    };
}
#endif //GRSTAPS_LINEARIZER_HPP
