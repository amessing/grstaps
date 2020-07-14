#include "grstaps/task_planning/selector.hpp"

namespace grstaps
{
    /*******************************************/
/* SearchQueue                             */
/*******************************************/

    SearchQueue::SearchQueue(int index)
    {
        this->index = index;
        pq.reserve(INITIAL_PQ_CAPACITY);
        pq.push_back(nullptr);    // Position 0 empty
        plan_position.reserve(INITIAL_PQ_CAPACITY);
        best_h = FLOAT_INFINITY;
        improved_h = true;
    }

// Adds a new plan to the list of open nodes
    void SearchQueue::add(Plan* p)
    {
        unsigned int gap = pq.size();
        uint32_t parent;
        pq.push_back(nullptr);
        while(gap > 1 && p->compare(pq[gap >> 1], index) < 0)
        {
            parent = gap >> 1;
            plan_position[pq[parent]->id] = gap;
            pq[gap] = pq[parent];
            gap = parent;
        }
        pq[gap] = p;
        plan_position[p->id] = gap;
    }

// Removes and returns the best plan in the queue of open nodes
    Plan* SearchQueue::poll()
    {
        Plan* best = pq[1];
        if(pq.size() > 2)
        {
            pq[1] = pq.back();
            plan_position[pq[1]->id] = 1;
            pq.pop_back();
            heapify(1);
        }
        else if(pq.size() > 1)
        {
            pq.pop_back();
        }
        return best;
    }

// Repairs the order in the priority queue
    void SearchQueue::heapify(unsigned int gap)
    {
        Plan* aux = pq[gap];
        unsigned int child = gap << 1;
        while(child < pq.size())
        {
            if(child != pq.size() - 1 && pq[child + 1]->compare(pq[child], index) < 0)
            {
                child++;
            }
            if(pq[child]->compare(aux, index) < 0)
            {
                plan_position[pq[child]->id] = gap;
                pq[gap] = pq[child];
                gap = child;
                child = gap << 1;
            }
            else
            { break; }
        }
        pq[gap] = aux;
        plan_position[aux->id] = gap;
    }

    void SearchQueue::remove(Plan* p)
    {
        uint32_t k = plan_position[p->id], parent;
        Plan* ult = pq.back();
        pq.pop_back();
        if(ult->compare(p, index) < 0)
        {
            while(k > 1 && ult->compare(pq[k >> 1], index) < 0)
            {
                parent = k >> 1;
                plan_position[pq[parent]->id] = k;
                pq[k] = pq[parent];
                k = parent;
            }
            pq[k] = ult;
            plan_position[ult->id] = k;
        }
        else
        {
            pq[k] = ult;
            plan_position[ult->id] = k;
            heapify(k);
        }
    }

    void SearchQueue::clear()
    {
        plan_position.clear();
        pq.clear();
        pq.push_back(nullptr);    // Position 0 empty
    }


/*******************************************/
/* Selector                               */
/*******************************************/

    Selector::Selector()
    {
        current_queue = 0;
        overall_best_plan = nullptr;
        overallBest = FLOAT_INFINITY;
        iterations_without_improving = 0;
    }

    void Selector::addQueue(int qtype)
    {
        queues.push_back(new SearchQueue(qtype));
    }

    bool Selector::add(Plan* p)
    {
        SearchQueue* q = queues[current_queue];
        float ph = p->getH(q->getIndex());
        if(ph < q->best_h)
        {
            q->improved_h = true;
            q->best_h = ph;
        }
        for(unsigned int i = 0; i < queues.size(); i++)
        {
            queues[i]->add(p);
        }
        if(p->h < overallBest)
        {
            iterations_without_improving = 0;
            overallBest = p->h;
            overall_best_plan = p;
            //cout << "[" << q->getIndex() << "]" << overallBest << endl;
            return true;
        }
        return false;
    }

    void Selector::exportTo(Selector* s)
    {
        SearchQueue* q = queues[0];
        unsigned int n = q->size();
        for(unsigned int i = 1; i <= n; i++)
        {
            s->add(q->getPlanAt(i));
        }
    }

// Removes and returns the best plan in the queue of open nodes
    Plan* Selector::poll()
    {
        SearchQueue* q = queues[current_queue];
        if(!q->improved_h)
        {
            if(++current_queue >= (int)queues.size())
            { current_queue = 0; }
            q = queues[current_queue];
        }
        Plan* next = q->poll();
        for(unsigned int i = 0; i < queues.size(); i++)
        {
            if((int)i != current_queue)
            {
                queues[i]->remove(next);
            }
        }
        q->improved_h = false;
        iterations_without_improving++;
        return next;
    }

    void Selector::clear()
    {
        for(unsigned int i = 0; i < queues.size(); i++)
        {
            queues[i]->clear();
        }
    }

/*******************************************/
/* Plateau Selector                        */
/*******************************************/

    PlateauSelector::PlateauSelector(int qtype)
    {
        q = new SearchQueue(qtype);
    }

// Removes and returns the best plan in the queue of open nodes
    Plan* PlateauSelector::poll()
    {
        return q->poll();
    }

    Plan* PlateauSelector::randomPoll()
    {
        int n = 1 + (rand() % (q->size()));
        Plan* next = q->getPlanAt(n);
        q->remove(next);
        return next;
    }

    void PlateauSelector::add(Plan* p)
    {
        q->add(p);
    }

    void PlateauSelector::exportTo(Selector* s)
    {
        unsigned int n = q->size();
        for(unsigned int i = 1; i <= n; i++)
        {
            s->add(q->getPlanAt(i));
        }
    }

/*******************************************/
/* Quality Selector                        */
/*******************************************/

    void QualitySelector::initialize(float bestQualityFound, uint16_t numAct, Successors* suc)
    {
        setBestPlanQuality(bestQualityFound, numAct);
        q_ff = new SearchQueue(SEARCH_G_HLAND_HFF + SEARCH_PLATEAU);
    }

    void QualitySelector::setBestPlanQuality(float bestQualityFound, uint16_t numAct)
    {
        best_quality = bestQualityFound;
        num_actions = numAct;
    }

    Plan* QualitySelector::poll()
    {
        Plan* next = q_ff->poll();
        while(!improves(next))
        {
            if(q_ff->size() == 0)
            { return nullptr; }
            next = q_ff->poll();
        }
        return next;
    }

    void QualitySelector::add(Plan* p)
    {
        if(improves(p))
        {
            q_ff->add(p);
        }
    }
}

