#include "grstaps/task_planning/successors.hpp"
#include <iostream>

#include "grstaps/task_planning/state.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: PlanEffect                                    */
    /********************************************************/

    void PlanEffect::add(TTimePoint time, unsigned int iteration)
    {
        if(this->iteration != iteration)
        {  // Delete data from previous iterations
            timePoints.clear();
            this->iteration = iteration;
        }
        timePoints.push_back(time);
    }

    /********************************************************/
    /* CLASS: PlanEffect                                    */
    /********************************************************/

    void VarChange::add(TValue v, TTimePoint time, unsigned int iteration)
    {
        if(this->iteration != iteration)
        {  // Delete data from previous iterations
            values.clear();
            timePoints.clear();
            this->iteration = iteration;
        }
        values.push_back(v);
        timePoints.push_back(time);
    }

    /********************************************************/
    /* CLASS: Threat                                        */
    /********************************************************/

    Threat::Threat(TTimePoint c1, TTimePoint c2, TTimePoint p, TVariable v)
    {
        p1  = c1;
        p2  = c2;
        tp  = p;
        var = v;
    }

    /********************************************************/
    /* CLASS: PlanBuilder                                   */
    /********************************************************/

    PlanBuilder::PlanBuilder(SASAction* a, Linearizer* linearizer, TStep lastStep)
    {
        action              = a;
        currentPrecondition = currentEffect = 0;
        setPrecondition                     = MAX_UNSIGNED_INT;
        this->linearizer                    = linearizer;
        lastTimePoint                       = stepToEndPoint(lastStep);
    }

    bool PlanBuilder::addLink(SASCondition* c, TTimePoint p1, TTimePoint p2)
    {
        if(addOrdering(p1, p2))
        {
            causalLinks.emplace_back(c->var, c->value, p1, p2);
            return true;
        }
        return false;
    }

    bool PlanBuilder::addLink(TVarValue vv, TTimePoint p1, TTimePoint p2)
    {
        if(addOrdering(p1, p2))
        {
            causalLinks.emplace_back(vv, p1, p2);
            return true;
        }
        return false;
    }

    bool PlanBuilder::addOrdering(TTimePoint p1, TTimePoint p2)
    {
        if(p1 == p2 || linearizer->existOrder(p2, p1))
        {
            return false;
        }
        if(linearizer->existOrder(p1, p2))
        {
            numOrderingsAdded.push_back(0);  // Ordering already exists
        }
        else
        {
            unsigned int newOrderings = 0;
            TTimePoint prevP1, nextP2;
            prevPoints.clear();
            nextPoints.clear();
            prevPoints.push_back(p1);
            nextPoints.push_back(p2);
            for(TTimePoint t = 1; t <= lastTimePoint; t++)
            {
                if(linearizer->existOrder(t, p1))
                {
                    prevPoints.push_back(t);
                }
                if(linearizer->existOrder(p2, t))
                {
                    nextPoints.push_back(t);
                }
            }
            for(unsigned int i = 0; i < prevPoints.size(); i++)
            {
                prevP1 = prevPoints[i];
                // cout << "* PrevPoint = " << prevP1 << " (from " << lastTimePoint << ")" << endl;
                for(unsigned int j = 0; j < nextPoints.size(); j++)
                {
                    nextP2 = nextPoints[j];
                    // cout << "* NextPoint = " << nextP2 << endl;
                    if(prevP1 != nextP2 && !linearizer->existOrder(prevP1, nextP2))
                    {
                        newOrderings++;
                        linearizer->setOrder(prevP1, nextP2);
                        orderings.push_back(getOrdering(prevP1, nextP2));
                        // cout << "+ Ord: " << prevP1 << " ---> " << nextP2 << endl;
                    }  // else cout << "   * Ord " << prevP1 << " -> " << nextP2 << " already exists" << endl;
                }
            }
            numOrderingsAdded.push_back(newOrderings);
        }
        return true;
    }

    void PlanBuilder::removeLastLink()
    {
        causalLinks.pop_back();
        removeLastOrdering();
    }

    void PlanBuilder::removeLastOrdering()
    {
        unsigned int newOrderings = numOrderingsAdded.back();
        numOrderingsAdded.pop_back();
        for(; newOrderings > 0; newOrderings--)
        {
            TOrdering o = orderings.back();
            orderings.pop_back();
            linearizer->clearOrder(firstPoint(o), secondPoint(o));
            // cout << "  - Removing ord: " << Successors::firstPoint(o) << " -> " << Successors::secondPoint(o) <<
            // endl;
        }
    }

    Plan* PlanBuilder::generatePlan(Plan* basePlan, uint32_t idPlan)
    {
        Plan* p = new Plan(this->action, basePlan, idPlan);
        for(unsigned int i = 0; i < this->causalLinks.size(); i++)
        {
            p->causalLinks.push_back(this->causalLinks[i]);
        }
        TTimePoint p1, p2;
        for(unsigned int i = 0; i < this->orderings.size(); i++)
        {
            p1 = firstPoint(this->orderings[i]);
            p2 = secondPoint(this->orderings[i]);
            if(p1 > 1 && ((p1 & 1) == 1 || (p1 + 1 != p2)))
            {  // Don't add ordering from the beginning to the end of a step
                p->orderings.push_back(this->orderings[i]);  // and from the initial step
            }
        }
        for(unsigned int i = 0; i < this->openCond.size(); i++)
        {
            p->addOpenCondition(this->openCond[i], timePointToStep(this->lastTimePoint));
        }
        // cout << "LAST TIME POINT: " << this->lastTimePoint << endl;
        this->removeLastOrdering();
        return p;
    }

    /********************************************************/
    /* CLASS: Successors                                    */
    /********************************************************/

    // Constructor
    void Successors::initialize(TState* state,
                                SASTask* task,
                                bool forceAtEndConditions,
                                bool filterRepeatedStates,
                                std::vector<SASAction*>* tilActions)
    {
        this->task                 = task;
        this->helpfulActions       = true;
        this->forceAtEndConditions = forceAtEndConditions;
        this->filterRepeatedStates = filterRepeatedStates;
        linearizer.setInitialState(state, task);
        numVariables = task->variables.size();
        numActions   = task->actions.size();
        planEffects  = new PlanEffect*[numVariables];
        for(unsigned int i = 0; i < numVariables; i++)
        {
            planEffects[i] = new PlanEffect[task->values.size()];
        }
        varChanges = new VarChange[numVariables];
        idPlan     = 0;
        solution   = nullptr;
        evaluator.initialize(state, task, tilActions, forceAtEndConditions);
        memoization.initialize(task);
        successors = nullptr;
        basePlan   = nullptr;
        // basePlanState = nullptr;
        newStep = 0;
        // cout << landmarks.toString(task) << endl;
        for(unsigned int i = 0; i < numActions; i++)
        {
            checkedAction.push_back(0);
        }
        currentIteration = 0;
    }

    // Destructor
    Successors::~Successors()
    {
        for(unsigned int i = 0; i < numVariables; i++)
        {
            delete[] planEffects[i];
        }
        delete[] planEffects;
        delete[] varChanges;
    }

    // Fills std::vector suc with the possible successor plans of the given base plan
    void Successors::computeSuccessors(Plan* base, std::vector<Plan*>* suc)
    {
        // Calculate the frontier state for the base plan
        // cout << "SUC OF " << base->action->name << endl;
        linearizer.setCurrentBasePlan(base);
        linearizer.setCurrentPlan(nullptr);
        newStep = linearizer.numComponents();  // Steps start by 0
        // basePlanState = linearizer.linearize(newStep, newStep << 1, task, nullptr);
        // Initialize the successors generator
        successors = suc;
        basePlan   = base;
        computeBasePlanEffects();
        suc->clear();
        if(!meetDeadlines())
        {
            return;
        }
        if(!base->repeatedState)
        {
            computeSolutionSuccessors();
        }
        if(solution != nullptr)
        {
            return;
        }
        currentIteration++;
        if(base->isRoot())
        {  // Full calculation of successors
            for(unsigned int i = 0; i < task->goals.size(); i++)
            {
                fullActionCheck(&(task->goals[i]));
            }
            for(unsigned int i = 0; i < task->actions.size(); i++)
            {
                // cout << "Action " << i << endl;
                fullActionCheck(&(task->actions[i]));
            }
        }
        else
        {  // Calculation of successores based on the parent plan
            computeSuccessorsSupportedByLastActions();
            computeSuccessorsThroughBrotherPlans();
        }
        // delete basePlanState;
    }

    bool Successors::meetDeadlines()
    {
        if(!task->areGoalDeadlines())
        {
            return true;
        }
        float makespan                       = basePlan->gc;
        std::vector<GoalDeadline>* deadlines = task->getGoalDeadlines();
        for(unsigned int i = 0; i < deadlines->size(); i++)
        {
            GoalDeadline& deadline = deadlines->at(i);
            if(makespan > deadline.time)
            {
                for(unsigned int j = 0; j < deadline.goals.size(); j++)
                {
                    TVarValue goal = deadline.goals[j];
                    TVariable v    = SASTask::getVariableIndex(goal);
                    TValue value   = SASTask::getValueIndex(goal);
                    if(planEffects[v][value].iteration != linearizer.getIteration())
                    {
                        std::cout << "-";
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Fills std::vector suc with the possible successor plans of the given base plan in concurrent domains
    void Successors::computeSuccessorsConcurrent(Plan* base, std::vector<Plan*>* suc)
    {
        if(!base->repeatedState)
        {
            computeSolutionSuccessors();
        }
        if(solution != nullptr)
        {
            return;
        }
        if(base->isRoot())
        {  // Full calculation of successors
            computeSuccessors(base, suc);
            return;
        }
        currentIteration++;
        linearizer.setCurrentBasePlan(base);
        linearizer.setCurrentPlan(nullptr);
        newStep    = linearizer.numComponents();  // Steps start by 0
        successors = suc;
        basePlan   = base;
        computeBasePlanEffects();
        suc->clear();
        computeSuccessorsSupportedByLastActions();
        computeSuccessorsThroughBrotherPlans();
        TState* s = linearizer.getFrontierState(task, nullptr);
        for(unsigned int i = 0; i < s->numSASVars; i++)
        {
            std::vector<SASAction*>& req = task->requirers[i][s->state[i]];
            for(unsigned int j = 0; j < req.size(); j++)
            {
                if(!visitedAction(req[j]) && s->isExecutable(req[j]))
                {
                    fullActionCheck(req[j]);
                }
            }
        }
        delete s;
        computeSolutionSuccessors();
    }

    // Fill the planEffects matrix with the effects produced by the base plan
    void Successors::computeBasePlanEffects()
    {
        unsigned int var, value;
        TTimePoint time;
        for(unsigned int i = 0; i < linearizer.numComponents(); i++)
        {
            SASAction* a = linearizer.getComponent(i)->action;
            for(unsigned int j = 0; j < a->startEff.size(); j++)
            {
                var   = a->startEff[j].var;
                value = a->startEff[j].value;
                time  = stepToStartPoint(i);
                planEffects[var][value].add(time, linearizer.getIteration());
                varChanges[var].add(value, time, linearizer.getIteration());
            }
            for(unsigned int j = 0; j < a->endEff.size(); j++)
            {
                var   = a->endEff[j].var;
                value = a->endEff[j].value;
                time  = stepToEndPoint(i);
                planEffects[var][value].add(time, linearizer.getIteration());
                varChanges[var].add(value, time, linearizer.getIteration());
            }
        }
    }

    // Checks if the given action can generate a successor plan
    void Successors::fullActionCheck(SASAction* a)
    {
        if(supportedAction(a))
        {  // Check if the (non-numeric) action precondtions can be supported by the steps in the current base plan
#ifdef DEBUG_SUCC_ON
            cout << "Action " << a->name << " supported" << endl;
#endif
            PlanBuilder pb(a, &linearizer, newStep);
            fullActionSupportCheck(&pb);
        }
    }

    // Checks if it is possible to support the next precondition of the action
    void Successors::fullActionSupportCheck(PlanBuilder* pb)
    {
        if(pb->currentPrecondition == pb->setPrecondition)
        {
            pb->currentPrecondition++;
            fullActionSupportCheck(pb);
            pb->currentPrecondition--;
        }
        else if(pb->currentPrecondition < pb->action->startCond.size())
        {  // At-start condition
            fullCondtionSupportCheck(
                pb, &(pb->action->startCond[pb->currentPrecondition]), stepToStartPoint(newStep), false, false);
        }
        else if(pb->currentPrecondition < pb->action->startCond.size() + pb->action->overCond.size())
        {  // Over-all condition
            fullCondtionSupportCheck(pb,
                                     &(pb->action->overCond[pb->currentPrecondition - pb->action->startCond.size()]),
                                     stepToStartPoint(newStep),
                                     true,
                                     false);
        }
        else if(pb->currentPrecondition <
                pb->action->startCond.size() + pb->action->overCond.size() + pb->action->endCond.size())
        {  // At-end condition
            fullCondtionSupportCheck(
                pb,
                &(pb->action
                      ->endCond[pb->currentPrecondition - pb->action->startCond.size() - pb->action->overCond.size()]),
                stepToEndPoint(newStep),
                false,
                !forceAtEndConditions);
        }
        else
        {  // Al condition supported -> check threats
            checkTheatsBetweenCausalLinkInBasePlanWithNewAction(pb);
        }
    }

    // Supports a non-numeric action condition, solving the threats that appear (if any)
    void Successors::fullCondtionSupportCheck(PlanBuilder* pb,
                                              SASCondition* c,
                                              TTimePoint condPoint,
                                              bool overAll,
                                              bool canLeaveOpen)
    {
        // cout << "Checking condition " << task->variables[c->var].name << "," << task->values[c->value].name << " for
        // action " << pb->action->name << endl;
        bool supportFound = false;
        if(linearizer.checkIteration(planEffects[c->var][c->value].iteration))
        {
            std::vector<TTimePoint>* supports = &(planEffects[c->var][c->value].timePoints);
            for(unsigned int i = 0; i < supports->size(); i++)
            {
                TTimePoint p = (*supports)[i];
                // cout << "+ CL: " << p << " ---> " << condPoint << " (" << task->variables[c->var].name << "," <<
                // task->values[c->value].name << ")" << endl;
                if(pb->addLink(c, p, condPoint))
                {  // Causal link added: p --- (c->var = c->value) ----> condPoint
                    if(overAll)
                    {
                        pb->addLink(c, p, condPoint + 1);
                    }
                    pb->currentPrecondition++;
                    fullActionSupportCheck(pb);
                    pb->currentPrecondition--;
                    pb->removeLastLink();
                    if(overAll)
                    {
                        pb->removeLastLink();
                    }
                    supportFound = true;
                }
            }
        }
        if(!supportFound && canLeaveOpen)
        {
            int precNumber = pb->currentPrecondition - pb->action->startCond.size() - pb->action->overCond.size();
            pb->openCond.push_back(precNumber);
            pb->currentPrecondition++;
            std::cout << "Leaving precondition open: " << precNumber << std::endl;
            fullActionSupportCheck(pb);
            pb->currentPrecondition--;
        }
    }

    // Generates a successor plan from the plan builder data
    void Successors::generateSuccessor(PlanBuilder* pb)
    {
        pb->addOrdering(pb->lastTimePoint - 1,
                        pb->lastTimePoint);  // Ordering from the begining to the end of the new step
        if(basePlan->hasOpenConditions())
        {  // If there are open conditions in the base plan, check if they can be solved through the effects of the new
           // action
            std::cout << "Plan with open conditions" << std::endl;
            solveBasePlanOpenConditionIfPossible(0, pb);
            return;
        }
        Plan* p = pb->generatePlan(basePlan, ++idPlan);
        if(postprocessPlan(p))
        {
            addSuccessor(p);
        }
    }

    // Tries to support the open condition (condNumber) in the base plan throw the effects of the new action added to
    // the successor's plan
    void Successors::solveBasePlanOpenConditionIfPossible(unsigned int condNumber, PlanBuilder* pb)
    {
        TOpenCond& c       = basePlan->openCond->at(condNumber);
        Plan* component    = linearizer.getComponent(c.step);
        SASCondition* cond = &(component->action->endCond[c.condNumber]);
        // cout << "Open condition number = " << c.condNumber << ".  " << task->variables[cond->var].name << "," <<
        // task->values[cond->value].name << endl;
        SASAction* a            = pb->action;
        SASCondition* eff       = nullptr;
        TTimePoint effTimePoint = pb->lastTimePoint - 1;
        for(unsigned int i = 0; i < a->startEff.size(); i++)
        {
            if(a->startEff[i].var == cond->var && a->startEff[i].value == cond->value)
            {
                eff = &(a->startEff[i]);
                break;
            }
        }
        if(eff == nullptr)
        {
            effTimePoint++;
            for(unsigned int i = 0; i < a->endEff.size(); i++)
            {
                if(a->endEff[i].var == cond->var && a->endEff[i].value == cond->value)
                {
                    eff = &(a->endEff[i]);
                    break;
                }
            }
        }
        if(eff != nullptr)
        {
            // cout << "Support found = " << task->variables[eff->var].name << "," << task->values[eff->value].name <<
            // endl;
            if(!pb->addLink(cond, effTimePoint, stepToEndPoint(c.step)))
            {
                eff = nullptr;
            }
        }
        if(++condNumber < basePlan->openCond->size())
        {
            solveBasePlanOpenConditionIfPossible(condNumber, pb);
        }
        else
        {
            Plan* p = pb->generatePlan(basePlan, ++idPlan);
            if(postprocessPlan(p))
            {
                addSuccessor(p);
            }
        }
        if(eff != nullptr)
        {
            pb->removeLastLink();
        }
    }

    // Adds a new plan to successor's list
    void Successors::addSuccessor(Plan* p)
    {
        successors->push_back(p);
#ifdef DEBUG_SUCC_ON
        cout << "Plan " << p->id << " generated" << endl;
#endif
        if(p->isSolution())
        {
#ifdef DEBUG_SUCC_ON
            cout << "SOLUTION PLAN" << endl;
#endif
            // cout << "SOL.: " << p->gc << "," << p->g << endl;
            solution = p;
        }
    }

    // Reuses the action, causal links and orderings of the given plan to generate a new successor. Threats
    // with the new action and causal links added in the base plan should be checked and solved
    void Successors::reuseAction(Plan* plan)
    {
#ifdef DEBUG_SUCC_ON
        cout << "Action " << plan->action->name << " supported (brother plan)" << endl;
#endif
        PlanBuilder pb(plan->action, &linearizer, newStep);
        TTimePoint startNewStep = stepToStartPoint(newStep), p2;
        // cout << startNewStep << endl;
        int numCl = 0, numOrd = 0;
        for(unsigned int i = 0; i < plan->causalLinks.size(); i++)
        {
            CausalLink& cl = plan->causalLinks[i];
            p2             = startNewStep;
            if((cl.secondPoint() & 1) == 1)
            {
                p2++;
            }  // at-end
#ifdef DEBUG_SUCC_ON
            cout << "CL: " << cl.firstPoint() << " ---> " << p2 << " (" << task->variables[cl.getVar()].name << ","
                 << task->values[cl.getValue()].name << ")" << endl;
#endif
            if(!pb.addLink(cl.varValue, cl.firstPoint(), p2))
                return;
            numCl++;
        }
        for(unsigned int i = 0; i < plan->orderings.size(); i++)
        {
            TOrdering ord = plan->orderings[i];
            TTimePoint p1 = firstPoint(ord);
            p2            = secondPoint(ord);
            if(p1 >= startNewStep - 2)
            {
                p1 += 2;
            }
            else
            {
                p2 += 2;
            }
            if(!pb.addOrdering(p1, p2))
            {
                return;
            }
            numOrd++;
            // matrix[p1][p2] = iteration;
            // pb.orderings.push_back(Successors::getOrdering(p1, p2));
#ifdef DEBUG_SUCC_ON
            cout << "Ord: " << p1 << " ---> " << p2 << endl;
#endif
        }  // Action, causal links and orderings added. Now check if there are threats
        checkTheatsBetweenCausalLinkInBasePlanWithNewAction(&pb);
        for(int i = 0; i < numCl; i++)
        {
            pb.removeLastLink();
        }
        for(int i = 0; i < numOrd; i++)
        {
            pb.removeLastOrdering();
        }
    }

    void Successors::checkContradictoryEffects(PlanBuilder* pb)
    {
        if(pb->currentEffect < pb->action->startEff.size())
        {  // At-start effect
            checkContradictoryEffects(pb, &(pb->action->startEff[pb->currentEffect]), stepToStartPoint(newStep));
        }
        else if(pb->currentEffect < pb->action->endEff.size() + pb->action->startEff.size())
        {  // End effect
            checkContradictoryEffects(
                pb, &(pb->action->endEff[pb->currentEffect - pb->action->startEff.size()]), stepToEndPoint(newStep));
        }
        else
        {
            generateSuccessor(pb);
        }
    }

    void Successors::checkContradictoryEffects(PlanBuilder* pb, SASCondition* c, TTimePoint effPoint)
    {
        VarChange& vc = varChanges[c->var];
        if(linearizer.checkIteration(vc.iteration))
        {
            for(unsigned int j = 0; j < vc.timePoints.size(); j++)
            {
                if(vc.values[j] != c->value)
                {
                    TTimePoint p = vc.timePoints[j];
                    if(p > 1 && !linearizer.existOrder(p, effPoint) && !linearizer.existOrder(effPoint, p))
                    {
                        if(pb->addOrdering(p, effPoint))
                        {
                            checkContradictoryEffects(pb, c, effPoint);
                            pb->removeLastOrdering();
                        }
                        if(pb->addOrdering(effPoint, p))
                        {
                            checkContradictoryEffects(pb, c, effPoint);
                            pb->removeLastOrdering();
                        }
                        return;
                        // cout << task->variables[c->var].name << ": " << task->values[vc.values[j]].name << " <---> "
                        // << task->values[c->value].name << endl;
                    }
                }
            }
        }
        pb->currentEffect++;
        checkContradictoryEffects(pb);
        pb->currentEffect--;
    }

    // Check the threats between the causal links of the base plan and the new action
    void Successors::checkTheatsBetweenCausalLinkInBasePlanWithNewAction(PlanBuilder* pb)
    {
        std::vector<Threat> threats;
        /*if (pb->action->isGoal)
            cout << "AQUI: " << this->basePlan->id << endl;
        */
        TTimePoint pc                       = pb->lastTimePoint - 1;
        std::vector<SASCondition>& startEff = pb->action->startEff;
        std::vector<SASCondition>& endEff   = pb->action->endEff;
        unsigned int numCausalLinks;
        TTimePoint p1, p2;
        TVariable var;
        TValue v;
        for(unsigned p = 1; p < linearizer.numComponents(); p++)
        {  // Threats between the causal links in the base plan and the effects of the new action
            numCausalLinks = linearizer.getComponent(p)->causalLinks.size();
            for(unsigned int i = 0; i < numCausalLinks; i++)
            {
                CausalLink& cl = linearizer.getComponent(p)->causalLinks[i];
                p1             = cl.firstPoint();
                p2             = cl.secondPoint();
                if(!linearizer.existOrder(pc, p1) && !linearizer.existOrder(p2, pc))
                {
                    var = cl.getVar();
                    v   = cl.getValue();
#ifdef DEBUG_SUCC_ON
                    cout << " - Threat : " << p1 << " -- " << task->variables[var].name << "," << task->values[v].name
                         << " --> " << p2 << endl;
#endif
                    for(unsigned int j = 0; j < startEff.size(); j++)
                    {
                        if(startEff[j].var == var && startEff[j].value != v)
                        {
                            threats.emplace_back(p1, p2, pc, var);
#ifdef DEBUG_SUCC_ON
                            cout << "   Threat found" << endl;
#endif
                            break;
                        }
                    }
                    pc++;
                    for(unsigned int j = 0; j < endEff.size(); j++)
                    {
                        if(endEff[j].var == var && endEff[j].value != v)
                        {
                            threats.emplace_back(p1, p2, pc, var);
#ifdef DEBUG_SUCC_ON
                            cout << "   Threat found" << endl;
#endif
                            break;
                        }
                    }
                    pc--;
                }
            }
        }
        for(unsigned int i = 0; i < pb->causalLinks.size(); i++)
        {  // Threats between the new causal links and the actions in the base plan
            CausalLink& cl = pb->causalLinks[i];
            TTimePoint p1 = cl.firstPoint(), p2 = cl.secondPoint();
            TVariable var = cl.getVar();
            TValue v      = cl.getValue();
            /*
            if (pb->action->isGoal) {
                cout << "New cl: " << p1 << "--" << task->variables[var].name << "=" <<
                        task->values[v].name << "--> " << p2 << endl;
            }*/

            VarChange& vc = varChanges[var];
#ifdef DEBUG_SUCC_ON
            cout << "New cl: " << p1 << " -> " << p2 << endl;
#endif
            if(linearizer.checkIteration(vc.iteration))
            {
                for(unsigned int j = 0; j < vc.timePoints.size(); j++)
                {
                    if(vc.values[j] != v)
                    {
                        pc = vc.timePoints[j];
#ifdef DEBUG_SUCC_ON
                        cout << "Dif. value in time point " << pc << endl;
#endif
                        if(!linearizer.existOrder(pc, p1) && !linearizer.existOrder(p2, pc))
                        {
#ifdef DEBUG_SUCC_ON
                            cout << "Threat by " << pc << endl;
#endif
                            threats.emplace_back(p1, p2, pc, var);
                        } /*
                     else {
                         if (linearizer.existOrder(pc, p1)) cout << "ORDER " << pc << " -> " << p1 << " exists" << endl;
                         if (linearizer.existOrder(p2, pc)) cout << "ORDER " << p2 << " -> " << pc << " exists" << endl;
                     }*/
                    }
                }
            }
        }
        solveThreats(pb, &threats);
    }

    // Computes the succesors obtained by adding new actions which are supported by the last action added in the base
    // plan
    void Successors::computeSuccessorsSupportedByLastActions()
    {
        if(!basePlan->repeatedState /*&& !basePlan->unsatisfiedNumericConditions*/)
        {
            SASAction* a                   = basePlan->action;
            TTimePoint startTimeNewAction  = stepToStartPoint(newStep);
            TTimePoint startTimeLastAction = startTimeNewAction - 2;
            TVariable var;
            TValue v;
            for(unsigned int i = 0; i < a->startEff.size(); i++)
            {
                var                          = a->startEff[i].var;
                v                            = a->startEff[i].value;
                std::vector<SASAction*>& req = task->requirers[var][v];
                for(unsigned int j = 0; j < req.size(); j++)
                {
                    if(!visitedAction(req[j]))
                    {
                        setVisitedAction(req[j]);
#ifdef DEBUG_SUCC_ON
                        cout << "Action " << req[j]->name << " supported by at-start" << endl;
#endif
                        PlanBuilder pb(req[j], &linearizer, newStep);
                        unsigned int n = addActionSupport(&pb, var, v, startTimeLastAction, startTimeNewAction);
                        fullActionSupportCheck(&pb);
                        for(unsigned int k = 0; k < n; k++)
                        {
                            pb.removeLastLink();
                        }
                    }
                }
            }
            for(unsigned int i = 0; i < a->endEff.size(); i++)
            {
                var                          = a->endEff[i].var;
                v                            = a->endEff[i].value;
                std::vector<SASAction*>& req = task->requirers[var][v];
                for(unsigned int j = 0; j < req.size(); j++)
                {
                    if(!visitedAction(req[j]))
                    {
                        setVisitedAction(req[j]);
#ifdef DEBUG_SUCC_ON
                        cout << "Action " << req[j]->name << " supported by at-end" << endl;
#endif
                        PlanBuilder pb(req[j], &linearizer, newStep);
                        unsigned int n = addActionSupport(&pb, var, v, startTimeLastAction + 1, startTimeNewAction);
                        fullActionSupportCheck(&pb);
                        for(unsigned int k = 0; k < n; k++)
                        {
                            pb.removeLastLink();
                        }
                    }
                }
            }
        }
    }

    // Build successors by adding the las actions of the brother plans
    void Successors::computeSuccessorsThroughBrotherPlans()
    {
        Plan* parentPlan                 = basePlan->parentPlan;
        std::vector<Plan*>* brotherPlans = parentPlan->childPlans;
        for(unsigned int i = 0; i < brotherPlans->size(); i++)
        {
            Plan* brotherPlan = (*brotherPlans)[i];
            if(brotherPlan != basePlan && !brotherPlan->expanded() && !visitedAction(brotherPlan->action))
            {
                // reuseAction(brotherPlan);
                setVisitedAction(brotherPlan->action);
                PlanBuilder pb(brotherPlan->action, &linearizer, newStep);
                fullActionSupportCheck(&pb);
            }
        }
    }

    void Successors::computeSolutionSuccessors()
    {
        for(unsigned int i = 0; i < task->goals.size(); i++)
        {
            fullActionCheck(&(task->goals[i]));
        }
    }

    // Adds a causal link to support one precondition. Return the number of links added (two in the case of over-all
    // conditions)
    unsigned int Successors::addActionSupport(PlanBuilder* pb,
                                              TVariable var,
                                              TValue value,
                                              TTimePoint effectTime,
                                              TTimePoint startTimeNewAction)
    {
        SASAction* a = pb->action;
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            if(a->startCond[i].var == var && a->startCond[i].value == value)
            {
                pb->setPrecondition = i;
                // cout << "+ CLS: " << effectTime << " ---> " << startTimeNewAction << " (" <<
                // task->variables[var].name << "," << task->values[value].name << ")" << endl;
                if(pb->addLink(&(a->startCond[i]), effectTime, startTimeNewAction))
                {
                    return 1;
                }
                return 0;
            }
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            if(a->overCond[i].var == var && a->overCond[i].value == value)
            {
                pb->setPrecondition = i + a->startCond.size();
                // cout << "+ CLO: " << effectTime << " ---> " << startTimeNewAction << " (" <<
                // task->variables[var].name << "," << task->values[value].name << ")" << endl; cout << "+ CLO: " <<
                // effectTime << " ---> " << (startTimeNewAction+1) << " (" << task->variables[var].name << "," <<
                // task->values[value].name << ")" << endl;
                if(pb->addLink(&(a->overCond[i]), effectTime, startTimeNewAction))
                {
                    if(pb->addLink(&(a->overCond[i]), effectTime, startTimeNewAction + 1))
                    {
                        return 2;
                    }
                    pb->removeLastLink();
                    return 0;
                }
                return 0;
            }
        }
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            if(a->endCond[i].var == var && a->endCond[i].value == value)
            {
                pb->setPrecondition = i + a->startCond.size() + a->overCond.size();
                // cout << "+ CLE: " << effectTime << " ---> " << (startTimeNewAction+1) << " (" <<
                // task->variables[var].name << "," << task->values[value].name << ")" << endl;
                if(pb->addLink(&(a->endCond[i]), effectTime, startTimeNewAction + 1))
                {
                    return 1;
                }
                return 0;
            }
        }
        return 0;
    }

    // Solves the threats in the plan
    void Successors::solveThreats(PlanBuilder* pb, std::vector<Threat>* threats)
    {
#ifdef DEBUG_SUCC_ON
        cout << threats->size() << " threats remaining" << endl;
#endif
        if(threats->size() == 0)
        {
#ifdef DEBUG_SUCC_ON
            cout << "Generating successor" << endl;
#endif
            checkContradictoryEffects(pb);
        }
        else
        {
            Threat t = threats->back();
            threats->pop_back();
#ifdef DEBUG_SUCC_ON
            cout << t.p1 << " --> " << t.p2 << " (threatened by " << t.tp << ")" << endl;
#endif
            if(!linearizer.existOrder(t.tp, t.p1) && !linearizer.existOrder(t.p2, t.tp))
            {  // Threat already exists
                bool promotion, demotion;
                if(mutexPoints(t.tp, t.p2, t.var, pb))
                {
#ifdef DEBUG_SUCC_ON
                    cout << "Unsolvable threat" << endl;
#endif
                    promotion = demotion = false;
                }
                else
                {
                    promotion = t.p1 > 1 && !linearizer.existOrder(t.p1, t.tp);
                    demotion  = !linearizer.existOrder(t.tp, t.p2);
                }
                if(promotion && demotion)
                {  // Both choices are possible
#ifdef DEBUG_SUCC_ON
                    cout << "Promotion and demotion valid" << endl;
#endif
                    if(pb->addOrdering(t.p2, t.tp))
                    {
                        solveThreats(pb, threats);
                        pb->removeLastOrdering();
                    }
                    if(pb->addOrdering(t.tp, t.p1))
                    {
                        solveThreats(pb, threats);
                        pb->removeLastOrdering();
                    }
                }
                else if(demotion)
                {  // Only demotion is possible: p2 -> tp
#ifdef DEBUG_SUCC_ON
                    cout << "Demotion valid" << endl;
                    cout << "Order " << t.p2 << " -> " << t.tp << " added" << endl;
#endif
                    if(pb->addOrdering(t.p2, t.tp))
                    {
                        solveThreats(pb, threats);
                        pb->removeLastOrdering();
                    }
                }
                else if(promotion)
                {  // Only promotion is possible: tp -> p1
#ifdef DEBUG_SUCC_ON
                    cout << "Promotion valid" << endl;
                    cout << "Order " << t.tp << " -> " << t.p1 << " added" << endl;
#endif
                    if(pb->addOrdering(t.tp, t.p1))
                    {
                        solveThreats(pb, threats);
                        pb->removeLastOrdering();
                    }
                }
#ifdef DEBUG_SUCC_ON
                else
                {  // Unsolvable threat
                    cout << "Unsolvable threat" << endl;
                }
#endif
            }
            else
            {
#ifdef DEBUG_SUCC_ON
                cout << "Not a threat now" << endl;
#endif
                solveThreats(pb, threats);
            }
        }
    }

    // Checks if in both time-steps the same fluent (var=value) is required, and in both time-steps that variable is
    // modified.
    bool Successors::mutexPoints(TTimePoint p1, TTimePoint p2, TVariable var, PlanBuilder* pb)
    {
        TStep s1 = p1 >> 1, s2 = p2 >> 1;
        SASAction* a1    = s1 == linearizer.numComponents() ? pb->action : linearizer.getComponent(s1)->action;
        SASCondition* c1 = getRequiredValue(p1, a1, var);
        if(c1 == nullptr || !c1->isModified)
        {
            return false;
        }
        SASAction* a2    = s2 == linearizer.numComponents() ? pb->action : linearizer.getComponent(s2)->action;
        SASCondition* c2 = getRequiredValue(p2, a2, var);
        return c2 != nullptr && c2->isModified && c2->value == c1->value;
    }

    TState* Successors::getFrontierState(Plan* p)
    {
        linearizer.setCurrentBasePlan(p);
        linearizer.setCurrentPlan(nullptr);
        return linearizer.getFrontierState(task, nullptr);
    }

    void Successors::printState(Plan* p)
    {
        linearizer.setCurrentPlan(p);
        TState* state = linearizer.getFrontierState(task, evaluator.getLandmarkHeuristic());
        for(unsigned int i = 0; i < state->numSASVars; i++)
        {
            std::cout << task->variables[i].name << "=" << task->values[state->state[i]].name << std::endl;
        }
        for(unsigned int i = 0; i < state->numNumVars; i++)
        {
            std::cout << task->numVariables[i].name << "=" << state->numState[i] << std::endl;
        }
        delete state;
    }

    // Linearizes the plan, check numeric/duration constraints and evaluates the plan
    bool Successors::postprocessPlan(Plan* p)
    {
        linearizer.setCurrentPlan(p);
        TState* state = linearizer.getFrontierState(task,
                                                    evaluator.getLandmarkHeuristic());  //, &(p->timeLastAddedStep));
        if(state != nullptr)
        {
            if(p->isSolution())
            {
                if(!goalsSupported(state))
                {
                    delete state;
                    return false;
                }
            }
            p->gc = task->evaluateMetric(state->numState, linearizer.makespan);
            evaluator.evaluate(p, state, linearizer.makespan, helpfulActions);
            p->repeatedState = filterRepeatedStates ? memoization.isRepeatedState(p, state) : false;
            // p->checkUsefulPlan();
            delete state;
            return true;
        }
        else
        {
#ifdef DEBUG_SUCC_ON
            cout << "Error: invalid state" << endl;
            cout << "INVALID PLAN: " << p->id << endl << p->toString() << endl;
#endif
            return false;  // Invalid plan
        }
    }

    void Successors::evaluate(Plan* p)
    {
        linearizer.setCurrentBasePlan(p);
        linearizer.setCurrentPlan(nullptr);
        TState* state = linearizer.getFrontierState(task, evaluator.getLandmarkHeuristic());
        p->gc         = task->evaluateMetric(state->numState, linearizer.makespan);
        evaluator.evaluate(p, state, linearizer.makespan, helpfulActions);
        delete state;
    }

    bool Successors::goalsSupported(TState* s)
    {
        for(unsigned int i = 0; i < task->goals.size(); i++)
        {
            if(actionSupported(&(task->goals[i]), s))
            {
                return true;
            }
        }
        return false;
    }

    bool Successors::actionSupported(SASAction* a, TState* s)
    {
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            if(s->state[a->startCond[i].var] != a->startCond[i].value)
            {
                return false;
            }
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            if(s->state[a->overCond[i].var] != a->overCond[i].value)
            {
                return false;
            }
        }
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            if(s->state[a->endCond[i].var] != a->endCond[i].value)
            {
                return false;
            }
        }
        for(unsigned int i = 0; i < a->startNumCond.size(); i++)
        {
            if(!task->holdsNumericCondition(a->startNumCond[i], s->numState, EPSILON))
            {
                return false;
            }
        }
        for(unsigned int i = 0; i < a->endNumCond.size(); i++)
        {
            if(!task->holdsNumericCondition(a->endNumCond[i], s->numState, EPSILON))
            {
                return false;
            }
        }
        return true;
    }

    bool Successors::informativeLandmarks()
    {
        return evaluator.informativeLandmarks();
    }

    void Successors::clearMemoization()
    {
        memoization.clear();
    }

    void Successors::clear()
    {
        solution = nullptr;
        idPlan   = 0;  // Plan counter
        clearMemoization();
    }

}  // namespace grstaps
