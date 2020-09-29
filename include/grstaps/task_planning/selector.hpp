#ifndef GRSTAPS_SELECTOR_HPP
#define GRSTAPS_SELECTOR_HPP

#include <vector>

#include "grstaps/task_planning/plan.hpp"
#include <unordered_map>

namespace grstaps
{
    class Successors;

    class SearchQueue
    {
       private:
        const static unsigned int INITIAL_PQ_CAPACITY = 8192;
        int index;
        std::unordered_map<uint32_t, uint32_t> plan_position;
        std::vector<Plan*> pq;

        void heapify(unsigned int gap);

       public:
        float best_h;  // For queue alternating
        bool improved_h;

        SearchQueue(int index);

        void add(Plan* p);

        Plan* poll();

        void remove(Plan* p);

        inline Plan* peek()
        {
            return pq[1];
        }

        inline int size()
        {
            return pq.size() - 1;
        }

        inline Plan* getPlanAt(unsigned int i)
        {
            return pq[i];
        }

        void clear();

        inline int getIndex()
        {
            return index;
        }
    };

    class Selector
    {
       private:
        std::vector<SearchQueue*> queues;
        int current_queue;
        Plan* overall_best_plan;  // Best hFF values found
        float overallBest;
        int iterations_without_improving;

       public:
        Selector();

        void addQueue(int qtype);

        Plan* poll();

        inline Plan* getPlanAt(unsigned int i)
        {
            return queues[0]->getPlanAt(i);
        }

        inline unsigned int size()
        {
            return queues[0]->size();
        }

        bool add(Plan* p);

        void exportTo(Selector* s);

        inline bool inPlateau(int plateauStart)
        {
            return iterations_without_improving >= plateauStart;
        }

        inline float getBestH()
        {
            return overallBest;
        }

        inline void setBestPlan(Plan* p)
        {
            iterations_without_improving = 0;
            overall_best_plan            = p;
            overallBest                  = p->h;
        }

        inline void setIterationsWithoutImproving(int n)
        {
            iterations_without_improving = n;
        }

        inline Plan* getBestPlan()
        {
            return overall_best_plan;
        }

        void clear();
    };

    class PlateauSelector
    {
       private:
        SearchQueue* q;  // Priority queue

       public:
        PlateauSelector(int qtype);

        inline unsigned int size()
        {
            return q->size();
        }

        Plan* poll();

        Plan* randomPoll();

        void add(Plan* p);

        void exportTo(Selector* s);
    };

    class QualitySelector
    {
       private:
        SearchQueue* q_ff;
        float best_quality;
        uint16_t num_actions;

       public:
        void initialize(float bestQualityFound, uint16_t numAct, Successors* suc);

        void setBestPlanQuality(float bestQualityFound, uint16_t numAct);

        inline unsigned int size()
        {
            return q_ff->size();
        }

        Plan* poll();

        void add(Plan* p);

        inline bool improves(Plan* p)
        {
            float distanceToBest = best_quality - p->gc;
            return (distanceToBest > EPSILON) || (distanceToBest >= 0 && p->g < num_actions);
        }
    };
}  // namespace grstaps

#endif  // GRSTAPS_SELECTOR_HPP
