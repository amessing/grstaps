#include "grstaps/task_planning/linearizer.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "grstaps/task_planning/landmark_heuristic.hpp"
#include "grstaps/task_planning/state.hpp"

namespace grstaps
{
    //#define PLAN_DEB	110

    /********************************************************/
    /* CLASS: Linearizer                                    */
    /* Manages the orderings in a plan, computes the        */
    /* linearization and check the duration an numeric      */
    /* constraints.                                         */
    /********************************************************/

    // Initializes the linearizer
    Linearizer::Linearizer()
    {
        matrix.resize(INITAL_MATRIX_SIZE);
        for(unsigned int i = 0; i < INITAL_MATRIX_SIZE; i++)
        {
            matrix[i].resize(INITAL_MATRIX_SIZE, 0);
        }
        iteration = 0;
    }

    void Linearizer::setInitialState(TState* initialState, SASTask* task)
    {
        this->initialState = initialState;
        this->task         = task;
    }

    // Sets the current plan
    void Linearizer::setCurrentBasePlan(Plan* plan)
    {
        iteration++;
        basePlan = plan;
        if(iteration == MAX_UNSIGNED_INT)
        {
            // Reset matrix (maximum number of iterations reached)
            iteration = 1;
            for(unsigned int i = 0; i < matrix.size(); i++)
            {
                for(unsigned int j = 0; j < matrix[i].size(); j++)
                {
                    matrix[i][j] = 0;
                }
            }
        }
        computeBasePlanSubcomponents(basePlan);
        computeOrderMatrix();
    }

    // Fills the basePlanComponents vector
    void Linearizer::computeBasePlanSubcomponents(Plan* base)
    {
        if(base == nullptr)
        {
            basePlanComponents.clear();
        }
        else
        {
            computeBasePlanSubcomponents(base->parentPlan);
            basePlanComponents.push_back(base);
        }
    }

    // Computes the order relationships among time points
    void Linearizer::computeOrderMatrix()
    {
        unsigned int newStep = basePlanComponents.size();  // Steps start by 0
        TTimePoint lastPoint = stepToEndPoint(newStep);
        if(lastPoint >= matrix.size())
        {
            resizeMatrix();
        }
        for(unsigned int i = 0; i < basePlanComponents.size(); i++)
        {
            if(basePlanComponents[i]->action != nullptr)
            {
                matrix[stepToStartPoint(i)][stepToEndPoint(i)] =
                    iteration;  // Start point of the step always before the end point of the step
#ifdef DEBUG_STATE_ON
                cout << "BMT: " << stepToStartPoint(i) << " - >" << stepToEndPoint(i) << endl;
#endif
            }
        }
        matrix[lastPoint - 1][lastPoint] = iteration;  // The same for the new step to be added
        for(unsigned int i = 0; i < basePlanComponents.size(); i++)
        {
            Plan& p = *(basePlanComponents[i]);
            for(unsigned int j = 0; j < p.orderings.size(); j++)
            {  // Plan orderings
                matrix[firstPoint(p.orderings[j])][secondPoint(p.orderings[j])] = iteration;
#ifdef DEBUG_STATE_ON
                cout << "BMO: " << firstPoint(p.orderings[j]) << " - >" << secondPoint(p.orderings[j]) << endl;
#endif
            }
            if(i > 0)
            {
                matrix[1][i << 1]       = iteration;  // Orderings with the initial step
                matrix[1][(i << 1) + 1] = iteration;
#ifdef DEBUG_STATE_ON
                cout << "BMF: " << 1 << " - >" << i * 2 << " and " << i * 2 + 1 << endl;
#endif
            }
        }
    }

    // Makes the order matrix larger
    void Linearizer::resizeMatrix()
    {
        unsigned int newSize = matrix.size() + MATRIX_INCREASE;
        matrix.resize(newSize);
        for(unsigned int i = 0; i < newSize; i++)
        {
            matrix[i].resize(newSize, 0);
        }
    }

    // Linearizes the plan. Returns the result in the linearOrder vector
    void Linearizer::topologicalOrder(std::vector<TTimePoint>* linearOrder)
    {
        unsigned int size = linearOrder->size();
        // cout << "SIZE = " << size << endl;
        std::vector<bool> visited(size, false);
        topologicalOrder(1, linearOrder, size - 1, &visited);
    }

    // Recursively linearizes the plan
    unsigned int Linearizer::topologicalOrder(TTimePoint orig,
                                              std::vector<TTimePoint>* linearOrder,
                                              unsigned int pos,
                                              std::vector<bool>* visited)
    {
        (*visited)[orig] = true;
        for(unsigned int i = 2; i <= linearOrder->size(); i++)
        {
            if(existOrder(orig, i))
            {
                if(!((*visited)[i]))
                {
                    // cout << orig << " -> " << i << endl;
                    pos = topologicalOrder(i, linearOrder, pos, visited);
                }
            }
        }
        (*linearOrder)[pos--] = orig;
        return pos;
    }

    // Returns the frontier state for the current plan given a valid topological order
    // Returns nullptr if there are unsolvable constraints (numerical or temporal)
    TState* Linearizer::getFrontierState(SASTask* task, LandmarkHeuristic* hLand)
    {  //, double* timeNewStep) {
        unsigned int numActions = basePlanComponents.size();
        if(plan != nullptr)
        {
            numActions++;
        }
        unsigned int numTimeSteps = numActions << 1;  // linearOrder->size() + 1;
        TState* state             = linearize(numActions, numTimeSteps, task, hLand);
        if(state != nullptr)
        {
            // if (timeNewStep != nullptr) *timeNewStep = time[numTimeSteps - 1];
            delete[] time;
            delete[] duration;
        }
        return state;
    }

    // Returns a string representation of the plan in PDDL format
    std::string Linearizer::planToPDDL(Plan* p, SASTask* task)
    {
        setCurrentBasePlan(p);
        setCurrentPlan(nullptr);
        unsigned int numActions   = basePlanComponents.size();
        unsigned int numTimeSteps = numActions << 1;
        double makespan           = 0;
        int numPlanActions        = 0;
        TState* state             = linearize(numActions, numTimeSteps, task, nullptr);
        bool lastActionIsGoal     = p->action != nullptr && p->action->isGoal;
        unsigned int last         = lastActionIsGoal ? numActions - 1 : numActions;
        std::vector<LinearStep> linearSteps;
        if(state != nullptr)
        {
            for(unsigned int i = 1; i < last; i++)
            {
                SASAction* a   = getAction(i);
                double endTime = time[i << 1] + duration[i];
                if(endTime > makespan)
                {
                    makespan = endTime;
                }
                if(a->name[0] != '#')
                {  // Fictitious actions
                    linearSteps.emplace_back(time[i << 1], a->name, duration[i]);
                    numPlanActions++;
                }
            }
            delete state;
            delete[] time;
            delete[] duration;
        }
        std::ostringstream oss;
        oss << std::setprecision(3) << std::fixed;
        std::sort(linearSteps.begin(), linearSteps.end());
        for(unsigned int i = 0; i < linearSteps.size(); i++)
        {
            LinearStep& s = linearSteps[i];
            oss << s.time << ": (" << s.actionName << ") [" << s.duration << "]" << std::endl;
        }
        oss << ";Makespan: " << (int)makespan << std::endl;
        oss << ";Actions:  " << numPlanActions << std::endl;
        return oss.str();
    }

    // Linearizes the plan and returns the frontier state. Starting time of the time-points are
    // stores in "time" array and action durations in the "duration" array. If there is no valid
    // action schedule, nullptr is returned
    TState* Linearizer::linearize(unsigned int numActions,
                                  unsigned int numTimeSteps,
                                  SASTask* task,
                                  LandmarkHeuristic* hLand)
    {
        bool invalidPlan = false;
        std::vector<TTimePoint> linearOrder(numTimeSteps);
        SASAction* lastAction = getLastAction();
        bool isSolution       = lastAction->isGoal;
        topologicalOrder(&linearOrder);
        initializeTimeArray(numTimeSteps);  // Store in an array time[t] the time for each time point t in the plan
        duration = new double[numActions];
        initialPlanSchedule(&linearOrder, numTimeSteps);
        if(task->tilActions && !checkValidInitialSchedule(&linearOrder))
        {
            return nullptr;
        }
        TState* state              = new TState(initialState);  // Make a copy of the initial state
        bool repeat                = true;
        unsigned int numRepairings = 0;
        while(repeat)
        {
            if(!isSolution)
            {
                fixScheduledTimes(numTimeSteps, state, &repeat, &invalidPlan, &linearOrder);
            }
            else
            {
                fixScheduledTimesForGoal(numTimeSteps, state, &repeat, &invalidPlan, &linearOrder);
            }
            if(invalidPlan)
            {
                delete state;
                break;
            }
            if(repeat)
            {
                delete state;
                numRepairings++;
                if(numRepairings >= numTimeSteps)
                {
                    invalidPlan = true;
                    break;
                }
                state = new TState(initialState);  // Make a copy of the initial state
            }
        }
        if(isSolution && !invalidPlan)
        {
            invalidPlan = !checkSolution(numTimeSteps);
        }
        if(!invalidPlan)
        {
            makespan = 0;
            for(unsigned int i = numTimeSteps - 1; i >= 2; i--)
            {
                if(time[i] > makespan)
                {
                    if(!getAction(i >> 1)->isTIL)
                    {
                        makespan = time[i];
                    }
                }
            }
            if(hLand != nullptr)
            {  // Calculate hLand heuristic value
                computeAchievedLandmarks(numTimeSteps, hLand);
            }
        }
        else
        {
            delete[] time;
            delete[] duration;
        }
        return invalidPlan ? nullptr : state;
    }

    void Linearizer::computeAchievedLandmarks(unsigned int numTimeSteps, LandmarkHeuristic* hLand)
    {
        initializeOpenNodes(hLand);
        TState state(initialState);  // Make a copy of the initial state
        pq.clear();
        for(unsigned int i = 2; i < numTimeSteps; i++)
        {
            Plan* p = getPlan(i >> 1);
            pq.add(new ScheduledPoint(i, time[i], p));
        }
        ScheduledPoint* p;
        unsigned int j;
        LandmarkCheck *l, *al;
        SASAction* a;
        while(pq.size() > 0)
        {
            p = (ScheduledPoint*)pq.poll();
            a = p->plan->action;
            // cout << "Executing " << p->a->name << endl;
            updateState(p->p, a, &state, duration[p->p >> 1]);
            delete p;
            j = 0;
            while(j < openNodes.size())
            {
                l = openNodes[j];
                // cout << "Landmark " << l->toString(task, false) << endl;
                if(l->goOn(&state))
                {  // The landmark holds in the state and we can progress
                    l->check();
                    openNodes.erase(openNodes.begin() + j);  // Remove node from the open nodes list
                    for(unsigned int k = 0; k < l->numNext(); k++)
                    {  // Go to the adjacent nodes
                        al = l->getNext(k);
                        if(!al->isChecked() && !findOpenNode(al))
                        {
                            openNodes.push_back(al);  // Non-visited node -> append to open nodes
                        }
                    }
                }
                else
                {
                    j++;
                }
            }
        }
    }

    bool Linearizer::checkSolution(unsigned int numTimeSteps)
    {
        // cout << "---------------- SOL -------------" << endl;
        std::vector<SASAction*> ongoingActions;
        std::vector<TStep> ongoingSteps;
        pq.clear();
        TState state(initialState);  // Make a copy of the initial state
        for(unsigned int i = 2; i < numTimeSteps; i++)
        {
            Plan* p = getPlan(i >> 1);
            pq.add(new ScheduledPoint(i, time[i], p));
        }
        bool ok = true;
        while(pq.size() > 0 && ok)
        {
            ScheduledPoint* p = (ScheduledPoint*)pq.poll();
            SASAction* a      = p->plan->action;
            TTimePoint tp     = p->p;
            // cout << time[tp] << ": " << a->name;
            TStep step = tp >> 1;
            if((tp & 1) == 0)
            {  // Start action point
                // Check action duration
                // cout << " [start]" << endl;
                // for (unsigned int i = 0; i < state.numNumVars; i++) {
                //	cout << "   " << task->numVariables[i].name << "=" << state.numState[i] << endl;
                //}
                ongoingActions.push_back(a);
                ongoingSteps.push_back(step);
                double dur = computeActionDuration(step, &state);
                if(abs(dur - (time[tp + 1] - time[tp])) > EPSILON / 2)
                {
                    ok = false;
                }
                if(!checkConditions(&a->startCond, &state) || !checkNumericConditions(&a->startNumCond, &state, dur))
                {
                    ok = false;
                }
                updateState(&a->startEff, &state);
                updateState(&a->startNumEff, &state, dur);
            }
            else
            {  // End action point
                // cout << " [end]" << endl;
                for(unsigned int i = 0; i < ongoingSteps.size(); i++)
                {
                    if(ongoingSteps[i] == step)
                    {
                        ongoingActions.erase(ongoingActions.begin() + i);
                        ongoingSteps.erase(ongoingSteps.begin() + i);
                        break;
                    }
                }
                double dur = this->duration[step];
                if(!checkConditions(&a->overCond, &state) || !checkNumericConditions(&a->overNumCond, &state, dur))
                {
                    ok = false;
                }
                if(!checkConditions(&a->endCond, &state) || !checkNumericConditions(&a->endNumCond, &state, dur))
                {
                    ok = false;
                }
                updateState(&a->endEff, &state);
                updateState(&a->endNumEff, &state, dur);
            }
            delete p;
            for(unsigned int i = 0; i < ongoingActions.size(); i++)
            {
                a          = ongoingActions[i];
                step       = ongoingSteps[i];
                double dur = this->duration[step];
                if(!checkConditions(&a->overCond, &state) || !checkNumericConditions(&a->overNumCond, &state, dur))
                {
                    ok = false;
                }
            }
        }
        // if (!ok) cout << "INVALID!!!" << endl;
        return ok;
    }

    bool Linearizer::checkConditions(std::vector<SASCondition>* c, TState* state)
    {
        for(unsigned int i = 0; i < c->size(); i++)
        {
            SASCondition* p = &(c->at(i));
            if(state->state[p->var] != p->value)
            {
                return false;
            }
        }
        return true;
    }

    bool Linearizer::checkNumericConditions(std::vector<SASNumericCondition>* c, TState* state, double dur)
    {
        for(unsigned int i = 0; i < c->size(); i++)
        {
            if(!task->holdsNumericCondition(c->at(i), state->numState, dur))
            {
                return false;
            }
        }
        return true;
    }

    void Linearizer::updateState(std::vector<SASCondition>* e, TState* state)
    {
        for(unsigned int i = 0; i < e->size(); i++)
        {
            state->setSASValue(e->at(i).var, e->at(i).value);
        }
    }

    void Linearizer::updateState(std::vector<SASNumericEffect>* e, TState* state, double dur)
    {
        for(unsigned int i = 0; i < e->size(); i++)
        {
            double value = task->evaluateNumericExpression(&(e->at(i).exp), state->numState, dur);
            state->setNumValue(e->at(i).var, e->at(i).op, value);
        }
    }

    void Linearizer::fixScheduledTimes(unsigned int numTimeSteps,
                                       TState* state,
                                       bool* repeat,
                                       bool* invalidPlan,
                                       std::vector<TTimePoint>* linearOrder)
    {
        *repeat = false;
        pq.clear();
        if(plan != nullptr)
        {
            plan->unsatisfiedNumericConditions = false;
        }
        for(unsigned int i = 2; i < numTimeSteps; i++)
        {
            Plan* p = getPlan(i >> 1);
            if(p->fixedEnd >= 0)
            {
                if(((i & 1) == 0 && time[i] >= 1.5 * EPSILON) ||
                   ((i & 1) == 1 && abs(time[i] - p->fixedEnd - EPSILON) >= EPSILON))
                {
                    for(unsigned int j = 1; j <= pq.size(); j++)
                    {
                        delete(ScheduledPoint*)pq.at(j);
                    }
                    pq.clear();
                    *invalidPlan = true;
                    return;
                }
            }
            pq.add(new ScheduledPoint(i, time[i], p));
            // if (debug) cout << i << " -> " << p->action->name << " -> " << time[i] << endl;
        }
        ScheduledPoint* p;
        std::vector<TTimePoint> unsatisfiedNumCond;
        TTimePoint tp;
        bool start;
        double dur;
        SASAction* a;
        // cout << "FIXING" << endl;
        while(pq.size() > 0)
        {
            p     = (ScheduledPoint*)pq.poll();
            a     = p->plan->action;
            tp    = p->p;
            start = (tp & 1) == 0;
            dur   = duration[tp >> 1];
            // if (debug) cout << tp << " -> " << a->name << " -> " << p->time << endl;
            if(start)
            {  // Start time point of an action
                if(time[tp + 1] - time[tp] >= dur + EPSILON)
                {
                    // if (debug)	cout << "Delay: action " << tp << ": " << time[start] << " = " << time[start + 1] <<
                    // "
                    // - " << dur << endl;
                    if(p->plan->fixedEnd < 0)
                    {
                        time[tp]     = time[tp + 1] - dur;
                        *invalidPlan = checkTopologicalOrder(linearOrder);
                    }
                    else
                    {
                        *invalidPlan = true;
                    }
                    delete p;
                    for(unsigned int j = 1; j <= pq.size(); j++)
                    {
                        delete(ScheduledPoint*)pq.at(j);
                    }
                    pq.clear();
                    *repeat = true;
                    break;
                }
            }
            if(!checkNumericConditions(tp, a, state, dur))
            {
                unsatisfiedNumCond.push_back(tp);
                unsigned int i = 1;
                while(i <= pq.size())
                {
                    ScheduledPoint* sp = (ScheduledPoint*)pq.at(i);
                    if(existOrder(tp, sp->p))
                    {
                        pq.fastRemove(i);
                        unsatisfiedNumCond.push_back(sp->p);
                        delete sp;
                    }
                    else
                    {
                        i++;
                    }
                }
                pq.fix();
                delete p;
            }
            else
            {
                updateState(tp, a, state, dur);
                // if (debug) cout << tp << " -> " << a->name << " -> " << p->time << endl;
                if(!unsatisfiedNumCond.empty())
                {
                    for(unsigned int i = 0; i < unsatisfiedNumCond.size(); i++)
                    {
                        if(existOrder(unsatisfiedNumCond[i], tp))
                        {
                            *invalidPlan = true;
                            for(unsigned int j = 1; j <= pq.size(); j++)
                            {
                                delete(ScheduledPoint*)pq.at(j);
                            }
                            pq.clear();
                            break;
                        }
                    }
                    checkUnsatisfiedConditions(p->time + EPSILON, state, &unsatisfiedNumCond);
                }
                delete p;
            }
        }
        if(!unsatisfiedNumCond.empty() && plan != nullptr)
        {
            plan->unsatisfiedNumericConditions = true;
        }
    }

    void Linearizer::fixScheduledTimesForGoal(unsigned int numTimeSteps,
                                              TState* state,
                                              bool* repeat,
                                              bool* invalidPlan,
                                              std::vector<TTimePoint>* linearOrder)
    {
        *repeat = false;
        pq.clear();
        if(plan != nullptr)
        {
            plan->unsatisfiedNumericConditions = false;
        }
        for(unsigned int i = 2; i < numTimeSteps; i++)
        {
            Plan* p = getPlan(i >> 1);
            if(p->fixedEnd >= 0)
            {
                if(((i & 1) == 0 && time[i] >= 1.5 * EPSILON) ||
                   ((i & 1) == 1 && abs(time[i] - p->fixedEnd - EPSILON) >= EPSILON))
                {
                    for(unsigned int j = 1; j <= pq.size(); j++)
                    {
                        delete(ScheduledPoint*)pq.at(j);
                    }
                    pq.clear();
                    *invalidPlan = true;
                    return;
                }
            }
            pq.add(new ScheduledPoint(i, time[i], p));
            // cout << i << " -> " << p->action->name << " -> " << time[i] << endl;
        }
        ScheduledPoint* p;
        std::vector<ScheduledPoint*> sameTime;
        std::vector<TTimePoint> unsatisfiedNumCond;
        double currentTime = 0;
        TTimePoint tp;
        bool start;
        double dur;
        SASAction* a;
        // cout << "FIXING" << endl;
        while(pq.size() > 0)
        {
            p     = (ScheduledPoint*)pq.poll();
            a     = p->plan->action;
            tp    = p->p;
            start = (tp & 1) == 0;
            dur   = computeActionDuration(tp >> 1, state);
            // cout << tp << " -> " << a->name << " -> " << p->time << endl;
            if(start)
            {  // Start time point of an action
                if(time[tp + 1] - time[tp] >= dur + EPSILON)
                {
                    // cout << "Delay: action " << tp << ": " << time[start] << " = " << time[start + 1] << " - " << dur
                    // << endl;
                    if(p->plan->fixedEnd < 0)
                    {
                        time[tp]     = time[tp + 1] - dur;
                        *invalidPlan = checkTopologicalOrder(linearOrder);
                    }
                    else
                    {
                        *invalidPlan = true;
                    }
                    delete p;
                    for(unsigned int j = 1; j <= pq.size(); j++)
                    {
                        delete(ScheduledPoint*)pq.at(j);
                    }
                    pq.clear();
                    *repeat = true;
                    break;
                }
            }
            if(abs(p->time - currentTime) < EPSILON / 2)
            {
                bool delayed = false;
                for(unsigned int i = 0; i < sameTime.size(); i++)
                {
                    if(checkNumericMutex(tp, a, sameTime[i]->p, sameTime[i]->plan->action))
                    {
                        if(start)
                        {  // Start point of the action must be delayed
                            time[tp] += EPSILON;
                            if(delayTimePoints(p))
                            {
                                delayed = true;
                            }
                            else
                            {
                                for(unsigned int j = 1; j <= pq.size(); j++)
                                {
                                    delete(ScheduledPoint*)pq.at(j);
                                }
                                pq.clear();
                                *repeat = true;
                            }
                            break;
                        }
                        else
                        {  // End point must be delayed.
                            time[tp - 1] += EPSILON;
                            for(unsigned int j = 2; j < numTimeSteps; j++)
                            {
                                if(existOrder(tp - 1, j))
                                {
                                    time[j] += EPSILON;
                                }
                            }
                            *repeat = true;  // We must repeat the checking as we updated the starting time later
                            for(unsigned int j = 1; j <= pq.size(); j++)
                            {
                                delete(ScheduledPoint*)pq.at(j);
                            }
                            pq.clear();
                            delete p;
                            break;
                        }
                    }
                }
                if(*repeat)
                {
                    break;
                }
                if(delayed)
                {
                    continue;
                }
            }
            else
            {
                currentTime = p->time;
                for(unsigned int i = 0; i < sameTime.size(); i++)
                {
                    delete sameTime[i];
                }
                sameTime.clear();
            }
            if(!checkNumericConditions(tp, a, state, dur))
            {
                unsatisfiedNumCond.push_back(tp);
                unsigned int i = 1;
                while(i <= pq.size())
                {
                    ScheduledPoint* sp = (ScheduledPoint*)pq.at(i);
                    if(existOrder(tp, sp->p))
                    {
                        pq.fastRemove(i);
                        unsatisfiedNumCond.push_back(sp->p);
                        delete sp;
                    }
                    else
                    {
                        i++;
                    }
                }
                pq.fix();
                delete p;
            }
            else
            {
                sameTime.push_back(p);
                updateState(tp, a, state, dur);
                // cout << tp << " -> " << a->name << " -> " << p->time << endl;
                if(!unsatisfiedNumCond.empty())
                {
                    checkUnsatisfiedConditions(p->time + EPSILON, state, &unsatisfiedNumCond);
                }
            }
        }
        for(unsigned int i = 0; i < sameTime.size(); i++)
        {
            delete sameTime[i];
        }
        if(!unsatisfiedNumCond.empty() && plan != nullptr)
        {
            plan->unsatisfiedNumericConditions = true;
        }
        /*
        cout << "Repeat: " << *repeat << endl;
        cout << "Invalid: " << *invalidPlan << endl;
        */
        if(!(*repeat) && !(*invalidPlan))
        {
            *invalidPlan = checkTopologicalOrder(linearOrder);
            // cout << "Invalid: " << *invalidPlan << endl;
        }
        // exit(0);
    }

    void Linearizer::checkUnsatisfiedConditions(double currentTime,
                                                TState* state,
                                                std::vector<TTimePoint>* unsatisfiedNumCond)
    {
        unsigned int i = 0;
        std::vector<TTimePoint> alreadyDelayed;
        while(i < unsatisfiedNumCond->size())
        {
            TTimePoint tp = unsatisfiedNumCond->at(i);
            TStep step    = tp >> 1;
            Plan* p       = getPlan(step);
            if(checkNumericConditions(tp, p->action, state, duration[step]))
            {
                bool validOrder = true;
                for(unsigned int j = 0; j < i; j++)
                {
                    if(existOrder(unsatisfiedNumCond->at(j), tp))
                    {
                        validOrder = false;
                        break;
                    }
                }
                if(validOrder)
                {
                    bool delayed = false;
                    for(unsigned int j = 0; j < alreadyDelayed.size(); j++)
                    {
                        if(alreadyDelayed[j] == tp)
                        {
                            delayed = true;
                            alreadyDelayed.erase(alreadyDelayed.begin() + j);
                            break;
                        }
                    }
                    unsatisfiedNumCond->erase(unsatisfiedNumCond->begin() + i);
                    if(delayed)
                    {
                        pq.add(new ScheduledPoint(tp, time[tp], p));
                    }
                    else
                    {
                        double delay = currentTime - time[tp];
                        time[tp]     = currentTime;
                        pq.add(new ScheduledPoint(tp, currentTime, p));
                        for(unsigned int j = i; j < unsatisfiedNumCond->size(); j++)
                        {
                            TTimePoint np = unsatisfiedNumCond->at(j);
                            if(existOrder(tp, np))
                            {
                                time[np] += delay;
                                alreadyDelayed.push_back(np);
                            }
                        }
                    }
                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            }
        }
    }

    bool Linearizer::delayStartTimePointKeepingEnd(ScheduledPoint* p)
    {
        TTimePoint sp = p->p;
        TTimePoint ep = sp + 1;
        double delay  = time[sp] - p->time;
        bool modified = false, valid = true;
        for(unsigned int i = 1; i <= pq.size(); i++)
        {
            ScheduledPoint* nsp = (ScheduledPoint*)pq.at(i);
            TTimePoint np       = nsp->p;
            if(np != ep && existOrder(sp, np) && !existOrder(ep, np))
            {
                nsp->time += delay;
                time[np] = nsp->time;
                modified = true;
                if(((np & 1) == 1 && (time[np] - time[np - 1] >= duration[np >> 1] + EPSILON)))
                {
                    time[np - 1] = time[np] - duration[np >> 1];
                    valid        = false;
                }
            }
        }
        if(modified && valid)
        {
            pq.fix();
        }
        pq.add(p);
        return valid;
    }

    bool Linearizer::delayTimePoints(ScheduledPoint* p)
    {
        TTimePoint tp = p->p, np;
        double delay  = time[tp] - p->time;
        p->time       = time[tp];
        ScheduledPoint* sp;
        bool modified = false, valid = true;
        for(unsigned int i = 1; i <= pq.size(); i++)
        {
            sp = (ScheduledPoint*)pq.at(i);
            np = sp->p;
            if(existOrder(tp, np))
            {
                sp->time += delay;
                time[np] = sp->time;
                modified = true;
                if(((np & 1) == 1 && (time[np] - time[np - 1] >= duration[np >> 1] + EPSILON)))
                {
                    valid = false;
                }
            }
        }
        if(modified && valid)
        {
            pq.fix();
        }
        pq.add(p);
        return valid;
    }

    void Linearizer::initialPlanSchedule(std::vector<TTimePoint>* linearOrder, unsigned int numTimeSteps)
    {
        unsigned int i, j, numNumVars = initialState->numNumVars;
        numTimeSteps--;
        TTimePoint p1, p2;
        bool startPoint;
        float* numState = new float[numNumVars];
        for(i = 0; i < numNumVars; i++)
        {
            numState[i] = initialState->numState[i];
        }
        for(i = 2; i <= numTimeSteps; i++)
        {
            p1            = (*linearOrder)[i];
            TStep step1   = p1 >> 1;
            startPoint    = (p1 & 1) == 0;
            SASAction* a1 = getAction(step1);
            if(startPoint)
            {
                duration[step1] = task->getActionDuration(a1, numState);
            }
            updateNumState(p1, a1, numState, duration[step1]);
            for(j = i + 1; j <= numTimeSteps; j++)
            {
                p2 = (*linearOrder)[j];
                if(existOrder(p1, p2))
                {
                    if(startPoint && p2 == p1 + 1)
                    {  // p1 and p2 are the start and the end of the same action, respectively
                        time[p2] = time[p1] + duration[step1];
                    }
                    else
                    {
                        if(time[p2] < time[p1] + EPSILON)
                        {
                            time[p2] = ceil(100.0 * (time[p1] + EPSILON)) / 100.0;
                        }
                    }
                }
            }
            // cout << "Time of " << p1 << " is initially " << time[p1] << "(" << a1->name << ")" << endl;
        }
        delete[] numState;
    }

    void Linearizer::initializeOpenNodes(LandmarkHeuristic* hLand)
    {
        hLand->uncheckNodes();
        hLand->copyRootNodes(&openNodes);
    }

    bool Linearizer::checkValidInitialSchedule(std::vector<TTimePoint>* linearOrder)
    {
        // if (debug) {
        TTimePoint numTimePoints = linearOrder->size();
        for(TTimePoint p1 = 2; p1 < numTimePoints; p1++)
        {
            for(TTimePoint p2 = 2; p2 < numTimePoints; p2++)
            {
                if(existOrder(p1, p2) && time[p1] > time[p2])
                {
                    return false;
                }
            }
        }
        //}
        return true;
    }

    // Check if the current topological order is valid (after the action delay) and tries to fix it otherwise
    bool Linearizer::checkTopologicalOrder(std::vector<TTimePoint>* linearOrder)
    {
        for(unsigned int i = 2; i < linearOrder->size(); i++)
        {
            TTimePoint p      = (*linearOrder)[i];
            unsigned int prev = i - 1;
            if(time[p] < time[(*linearOrder)[prev]])
            {  // Wrong order, p should be before linearOrder[prev]
                TTimePoint p2 = (*linearOrder)[prev];
                do
                {
                    // if (this->plan == nullptr || this->plan->id == PLAN_DEB)
                    //	cout << "Wrong order: " << p2 << " -> " << p << endl;
                    if(existOrder(p2, p))
                    {  // Invalid plan: doesn\92t meet temporal contraints
                        // if (this->plan == nullptr || this->plan->id == PLAN_DEB) cout << "Invalid plan: doesn\92t
                        // meet temporal contraints" << endl;
                        return true;
                    }
                    (*linearOrder)[prev + 1] = p2;
                    (*linearOrder)[prev]     = p;
                    p2                       = (*linearOrder)[--prev];
                    /*
                    if (this->plan == nullptr || this->plan->id == PLAN_DEB) {
                        cout << "* Fixed linear order:";
                        for (unsigned int k = 0; k < linearOrder->size(); k++)
                            cout << " " << (*linearOrder)[k];
                        cout << endl;
                    }*/
                } while(time[p] < time[p2]);
            }
        }
        return false;
    }

    /* Check if it is necessary to delay an action after the scheduling
    bool Linearizer::checkActionDelayNeeded() {
        TTimePoint start;
        bool delay = false;
        bool numEffects = false;
        unsigned int n = basePlanComponents.size();
        for (unsigned int i = 1; i < n; i++) {
            if (!basePlanComponents[i]->action->startNumEff.empty() ||
    !basePlanComponents[i]->action->endNumEff.empty()) numEffects = true; start = i << 1; if (time[start + 1] -
    time[start] >= duration[i] + EPSILON) {
                //	cout << "Delay: action " << i << ": " << time[start] << " = " << time[start + 1] << " - " <<
    duration[i] << endl; time[start] = time[start + 1] - duration[i]; delay = true;
            }
        }
        if (!delay && numEffects) {
            numericMutex.clear();
            for (unsigned int i = 1; i < n; i++) {
                start = i << 1;
                std::unordered_map<double,TTimePoint>::const_iterator got = numericMutex.find(time[start]);
                if (got == numericMutex.end()) {	// No collisions per moment in that time point
                    numericMutex[time[start]] = start;
                } else {
                    if (checkNumericMutexWithStartPoint(start, got->second, basePlanComponents[i]->action)) {
                        time[start] += EPSILON;
                        time[start+1] += EPSILON;
                        delay = true;
                    }
                }
            }
        }
        return delay;
    }*/

    /* Schedules the time points of the plan throught the topological order and computes the resulting frontier state
    bool Linearizer::schedulePlan(vector<TTimePoint>* linearOrder, unsigned int numTimeSteps, TState* state,
    LandmarkHeuristic* hLand) { unsigned int i, j; makespan = 0; numTimeSteps--; TTimePoint p1, p2;

        for (i = 2; i <= numTimeSteps; i++) {
            p1 = (*linearOrder)[i];
            TStep step1 = p1 >> 1;
            SASAction* a1 = step1 < basePlanComponents.size() ? basePlanComponents[step1]->action : plan->action;
            double dur = task->getActionDuration(a1, state->numState);
            bool holdNumericConditions = checkNumericConditions(p1, a1, state, dur);
            updateState(p1, a1, state, dur);
            if (hLand != nullptr) {	// Calculate hLand heuristic value
                j = 0;
                LandmarkCheck *l, *al;
                while (j < openNodes.size()) {
                    l = openNodes[j];
                    //char name[255];
                    //std::string lname = l->toString(task, false);
                    //lname.copy(name, 0, lname.length());
                    if (l->goOn(state)) {	// The landmark holds in the state and we can progress
                        //cout << "Landmark " << l->toString(task, false) << " holds" << endl;
                        l->check();
                        openNodes.erase(openNodes.begin() + j); // Remove node from the open nodes list
                        for (unsigned int k = 0; k < l->numNext(); k++) { // Go to the adjacent nodes
                            al = l->getNext(k);
                            if (!al->isChecked() && !findOpenNode(al)) {
                                openNodes.push_back(al); // Non-visited node -> append to open nodes
                            }
                        }
                    }
                    else {
                        j++;
                    }
                }
            }
            for (j = i + 1; j <= numTimeSteps; j++) {
                p2 = (*linearOrder)[j];
                if (existOrder(p1, p2)) {
                    //if (this->plan == nullptr || this->plan->id == PLAN_DEB) cout << "* " << p1 << " --> " << p2 <<
    endl; if ((p1 & 1) == 0 && p2 == p1 + 1) {	// p1 and p2 are the start and the end of the same action, respectively
                        this->duration[step1] = dur;
                        time[p2] = time[p1] + dur; //computeActionDuration(p1 >> 1, state);

                        //if (this->plan == nullptr || this->plan->id == PLAN_DEB)
                        //	cout << "Time[" << p2 << "] = " << "Time[" << p1 << +"] + " << computeActionDuration(p1 >>
    1, state) << " = " << time[p2] << endl;
                    }
                    else {
                        //if (this->plan == nullptr || this->plan->id == PLAN_DEB)
                        //	cout << " * " << time[p2] << " <= " << (time[p1] + EPSILON) << "?" << endl;
                        if (time[p2] < time[p1] + EPSILON) {
                            time[p2] = time[p1] + EPSILON;
                            //if (this->plan == nullptr || this->plan->id == PLAN_DEB)
                            //	cout << "Time[" << p2 << "] = " << "Time[" << p1 << +"] + " << EPSILON << " = " <<
    time[p2] << endl;
                        }
                    }
                }
            }
            if (time[p1] > makespan) makespan = time[p1];
            if (!holdNumericConditions) {
                if (plan != nullptr) plan->unsatisfiedNumericConditions = true;
            }
        }
        return true;
    }*/

    bool Linearizer::checkNumericMutex(std::vector<SASNumericEffect>* e1, std::vector<SASNumericEffect>* e2)
    {
        for(unsigned int i = 0; i < e1->size(); i++)
        {
            TVariable v = e1->at(i).var;
            for(unsigned int j = 0; j < e2->size(); j++)
            {
                if(e2->at(j).var == v)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool Linearizer::variableInNumericExpression(TVariable v, SASNumericExpression* e)
    {
        if(e->type == 'V')
        {
            return e->var == v;
        }
        else
        {
            for(unsigned int i = 0; i < e->terms.size(); i++)
            {
                if(variableInNumericExpression(v, &e->terms[i]))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool Linearizer::variableInCondition(TVariable v, SASNumericCondition* c)
    {
        for(unsigned int i = 0; i < c->terms.size(); i++)
        {
            if(variableInNumericExpression(v, &c->terms[i]))
            {
                return true;
            }
        }
        return false;
    }

    bool Linearizer::checkNumericMutex(std::vector<SASNumericCondition>* c, std::vector<SASNumericEffect>* e)
    {
        for(unsigned int i = 0; i < e->size(); i++)
        {
            TVariable v = e->at(i).var;
            for(unsigned int j = 0; j < c->size(); j++)
            {
                if(variableInCondition(v, &c->at(j)))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool Linearizer::checkNumericMutex(TTimePoint p1, SASAction* a1, TTimePoint p2, SASAction* a2)
    {
        if((p1 & 1) == 0)
        {  // Start of a1
            if((p2 & 1) == 0)
            {  // Start of a2
                // Start effect of a1 with start effect of a2
                if(checkNumericMutex(&a1->startNumEff, &a2->startNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a1->startNumCond, &a2->startNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a2->startNumCond, &a1->startNumEff))
                {
                    return true;
                }
            }
            else
            {  // End of a2
                // Start effect of a1 with end effect of a2
                if(checkNumericMutex(&a1->startNumEff, &a2->endNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a1->startNumCond, &a2->endNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a2->endNumCond, &a1->startNumEff))
                {
                    return true;
                }
            }
        }
        else
        {  // End of a1
            if((p2 & 1) == 0)
            {  // Start of a2
                // End effect of a1 with start effect of a2
                if(checkNumericMutex(&a1->endNumEff, &a2->startNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a1->endNumCond, &a2->startNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a2->startNumCond, &a1->endNumEff))
                {
                    return true;
                }
            }
            else
            {  // End of a2
                // End effect of a1 with end effect of a2
                if(checkNumericMutex(&a1->endNumEff, &a2->endNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a1->endNumCond, &a2->endNumEff))
                {
                    return true;
                }
                if(checkNumericMutex(&a2->endNumCond, &a1->endNumEff))
                {
                    return true;
                }
            }
        }
        return false;
    }

    // Actual time-point p is the beginning point of an action
    bool Linearizer::checkNumericMutexWithStartPoint(TTimePoint p, TTimePoint prev, SASAction* a)
    {
        if(a->startNumEff.empty() && a->endNumEff.empty())
        {
            return false;
        }
        TStep prevStep   = prev >> 1;
        SASAction* aPrev = getAction(prevStep);
        if(aPrev->startNumEff.empty() && aPrev->endNumEff.empty())
        {
            return false;
        }
        // if (a->name.compare("board p1 fast1 f6") == 0 && aPrev->name.compare("board p4 fast1 f6") == 0);
        //	cout << "AQUI" << endl;
        TVariable v;
        if((prev & 1) == 0)
        {  // Start point together with previous start point
            for(unsigned int i = 0; i < a->startNumEff.size(); i++)
            {
                v = a->startNumEff[i].var;
                for(unsigned int j = 0; j < aPrev->startNumEff.size(); j++)
                {
                    if(aPrev->startNumEff[j].var == v)
                    {
                        return true;
                    }
                }
            }
            for(unsigned int i = 0; i < a->endNumEff.size(); i++)
            {
                v = a->endNumEff[i].var;
                for(unsigned int j = 0; j < aPrev->endNumEff.size(); j++)
                {
                    if(aPrev->endNumEff[j].var == v)
                    {
                        return true;
                    }
                }
            }
        }
        else
        {  // Start point together with previous end point
            for(unsigned int i = 0; i < a->startNumEff.size(); i++)
            {
                v = a->startNumEff[i].var;
                for(unsigned int j = 0; j < aPrev->endNumEff.size(); j++)
                {
                    if(aPrev->endNumEff[j].var == v)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Linearizer::findOpenNode(LandmarkCheck* l)
    {
        for(unsigned int i = 0; i < openNodes.size(); i++)
        {
            if(openNodes[i] == l)
            {
                return true;
            }
        }
        return false;
    }

    void Linearizer::updateNumState(TTimePoint p, SASAction* a, float* numState, double dur)
    {
        std::vector<SASNumericEffect>* numEff = (p & 1) == 0 ? &(a->startNumEff) : &(a->endNumEff);
        for(unsigned int i = 0; i < numEff->size(); i++)
        {
            SASNumericEffect& e = (*numEff)[i];
            double value        = task->evaluateNumericExpression(&(e.exp), numState, dur);
            TState::setNumVarValue(e.var, e.op, value, numState);
        }
    }

    // Adds the effects in that time point to the state
    void Linearizer::updateState(TTimePoint p, SASAction* a, TState* state, double dur)
    {
        // cout << "* Time point: " << p << " of action " << a->name << endl;
        std::vector<SASCondition>* eff;
        std::vector<SASNumericEffect>* numEff;
        if((p & 1) == 0)
        {  // Start point of an action
            eff    = &(a->startEff);
            numEff = &(a->startNumEff);
        }
        else
        {  // End point of an action
            eff    = &(a->endEff);
            numEff = &(a->endNumEff);
        }
        for(unsigned int i = 0; i < eff->size(); i++)
        {
            state->setSASValue((*eff)[i].var, (*eff)[i].value);
        }
        for(unsigned int i = 0; i < numEff->size(); i++)
        {
            double value = task->evaluateNumericExpression(&((*numEff)[i].exp), state->numState, dur);
            state->setNumValue((*numEff)[i].var, (*numEff)[i].op, value);
        }
    }

    bool Linearizer::checkNumericConditions(TTimePoint p, SASAction* a, TState* state, double dur)
    {
        std::vector<SASNumericCondition>* numCond = (p & 1) == 0 ? &(a->startNumCond) : &(a->endNumCond);
        for(unsigned int i = 0; i < numCond->size(); i++)
        {
            if(!task->holdsNumericCondition((*numCond)[i], state->numState, dur))
            {
                return false;
            }
        }
        for(unsigned int i = 0; i < a->overNumCond.size(); i++)
        {
            if(!task->holdsNumericCondition(a->overNumCond[i], state->numState, dur))
            {
                return false;
            }
        }
        return true;
    }

    // Returns a copy of the initial state
    TState* Linearizer::copyInitialState(SASTask* task)
    {
        unsigned int i;
        TState* state = new TState(task->variables.size(), task->numVariables.size());
        for(i = 0; i < state->numSASVars; i++)
        {
            state->state[i] = task->initialState[i];
            // cout << "Var. " << task->variables[i].name << " Value. " << task->values[state->state[i]].name << endl;
        }
        for(i = 0; i < state->numNumVars; i++)
        {
            state->numState[i] = task->numInitialState[i];
        }
        return state;
    }

    // Store in an array time[t] the time for each time point t in the plan
    //	* Time[0] = -epsilon	(start of the initial fictitious action)
    //	* Time[1] = 0			(end of the initial fictitious action)
    //	* Time[t] = epsilon, forall t > 1
    void Linearizer::initializeTimeArray(unsigned int numTimeSteps)
    {
        time    = new double[numTimeSteps];
        time[0] = -EPSILON;
        time[1] = 0;
        for(unsigned int i = 2; i < numTimeSteps; i++)
        {
            time[i] = EPSILON;
        }
    }

    // Returns the action duration at the given time
    double Linearizer::computeActionDuration(TStep step, TState* state)
    {
        SASAction* action    = getAction(step);
        this->duration[step] = task->getActionDuration(action, state->numState);
        return this->duration[step];
    }

}  // namespace grstaps
