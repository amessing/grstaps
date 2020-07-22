/*
 * Copyright (C) 2020 Andrew Messing
 *
 * grstaps is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * grstaps is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grstaps; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "grstaps/task_planning/landmarks.hpp"

// local
#include "grstaps/task_planning/state.hpp"

namespace grstaps
{
#define NECESSARY    1
#define REASONABLE    2


/*******************************************/
/* LandmarkRPG                             */
/*******************************************/

    bool LandmarkRPG::verifyFluent(TVariable v, TValue value, TState* s, SASTask* task)
    {
        this->task = task;
        initialize(s);
        achievedFluent.erase(SASTask::getVariableValueCode(v, value));
        while(remainingGoals.size() > 0 && lastLevel->size() > 0)
        {
            newLevel->clear();
            for(unsigned int i = 0; i < lastLevel->size(); i++)
            {
                TVariable gv = SASTask::getVariableIndex(lastLevel->at(i));
                TValue gvalue = SASTask::getValueIndex(lastLevel->at(i));
                if(gv == v && gvalue == value)
                { continue; }
                std::vector<SASAction*>& aList = task->requirers[gv][gvalue];
                for(unsigned int j = 0; j < aList.size(); j++)
                {
                    SASAction* a = aList[j];
                    if(!achievedAction[a->index] && isExecutable(a, v, value))
                    {
                        achievedAction[a->index] = true;
                        addActionEffects(a);
                    }
                }
            }
            swapLevels();
        }
        bool res = remainingGoals.size() > 0;
        clearMemory();
        return res;
    }

    bool LandmarkRPG::verifyFluents(std::vector<TVariable>* v, std::vector<TValue>* value, TState* s, SASTask* task)
    {
        this->task = task;
        initialize(s);
        unsigned int n = v->size();
        for(unsigned int i = 0; i < n; i++)
        {
            achievedFluent.erase(SASTask::getVariableValueCode(v->at(i), value->at(i)));
        }
        while(remainingGoals.size() > 0 && lastLevel->size() > 0)
        {
            newLevel->clear();
            for(unsigned int i = 0; i < lastLevel->size(); i++)
            {
                TVariable gv = SASTask::getVariableIndex(lastLevel->at(i));
                TValue gvalue = SASTask::getValueIndex(lastLevel->at(i));
                bool inSet = false;
                for(unsigned int i = 0; i < n; i++)
                {
                    if(gv == v->at(i) && gvalue == value->at(i))
                    {
                        inSet = true;
                        break;
                    }
                }
                if(!inSet)
                {
                    std::vector<SASAction*>& aList = task->requirers[gv][gvalue];
                    for(unsigned int j = 0; j < aList.size(); j++)
                    {
                        SASAction* a = aList[j];
                        if(!achievedAction[a->index] && isExecutable(a, v, value))
                        {
                            achievedAction[a->index] = true;
                            addActionEffects(a);
                        }
                    }
                }
            }
            swapLevels();
        }
        bool res = remainingGoals.size() > 0;
        clearMemory();
        return res;
    }

    bool LandmarkRPG::verifyActions(std::vector<SASAction*>* actions, TState* s, SASTask* task)
    {
        this->task = task;
        initialize(s);
        while(remainingGoals.size() > 0 && lastLevel->size() > 0)
        {
            newLevel->clear();
            for(unsigned int i = 0; i < lastLevel->size(); i++)
            {
                TVariable gv = SASTask::getVariableIndex(lastLevel->at(i));
                TValue gvalue = SASTask::getValueIndex(lastLevel->at(i));
                std::vector<SASAction*>& aList = task->requirers[gv][gvalue];
                for(unsigned int j = 0; j < aList.size(); j++)
                {
                    SASAction* a = aList[j];
                    if(!achievedAction[a->index] && isExecutable(a) && allowedAction(a, actions))
                    {
                        achievedAction[a->index] = true;
                        addActionEffects(a);
                    }
                }
            }
            swapLevels();
        }
        bool res = remainingGoals.size() > 0;
        clearMemory();
        return res;
    }

    void LandmarkRPG::swapLevels()
    {
        TVarValue code;
        unsigned int i;
        lastLevel->clear();
        for(i = 0; i < newLevel->size(); i++)
        {
            code = newLevel->at(i);
            if(!fluentAchieved(code))
            {
                achievedFluent[code] = true;
                lastLevel->push_back(code);
            }
        }
        newLevel->clear();
        i = 0;
        while(i < remainingGoals.size())
        {
            code = remainingGoals[i];
            if(fluentAchieved(code))
            {
                remainingGoals.erase(remainingGoals.begin() + i);
            }
            else
            {
                i++;
            }
        }
    }

    bool LandmarkRPG::isExecutable(SASAction* a, TVariable v, TValue value)
    {
        if(!isExecutable(a))
        { return false; }
        for(unsigned int j = 0; j < a->startEff.size(); j++)
        {
            if(a->startEff[j].var == v && a->startEff[j].value == value)
            { return false; }
        }
        for(unsigned int j = 0; j < a->endEff.size(); j++)
        {
            if(a->endEff[j].var == v && a->endEff[j].value == value)
            { return false; }
        }
        return true;
    }

    bool LandmarkRPG::isExecutable(SASAction* a, std::vector<TVariable>* v, std::vector<TValue>* value)
    {
        if(!isExecutable(a))
        { return false; }
        unsigned int n = v->size();
        for(unsigned int j = 0; j < a->startEff.size(); j++)
        {
            for(unsigned int i = 0; i < n; i++)
            {
                if(a->startEff[j].var == v->at(i) && a->startEff[j].value == value->at(i))
                { return false; }
            }
        }
        for(unsigned int j = 0; j < a->endEff.size(); j++)
        {
            for(unsigned int i = 0; i < n; i++)
            {
                if(a->endEff[j].var == v->at(i) && a->endEff[j].value == value->at(i))
                { return false; }
            }
        }
        return true;
    }

    bool LandmarkRPG::isExecutable(SASAction* a)
    {
        for(unsigned int j = 0; j < a->startCond.size(); j++)
        {
            if(!fluentAchieved(a->startCond[j].var, a->startCond[j].value))
            { return false; }
        }
        for(unsigned int j = 0; j < a->overCond.size(); j++)
        {
            if(!fluentAchieved(a->overCond[j].var, a->overCond[j].value))
            { return false; }
        }
        for(unsigned int j = 0; j < a->endCond.size(); j++)
        {
            if(!fluentAchieved(a->endCond[j].var, a->endCond[j].value))
            { return false; }
        }
        return true;
    }

    bool LandmarkRPG::allowedAction(SASAction* a, std::vector<SASAction*>* actions)
    {
        for(unsigned int i = 0; i < actions->size(); i++)
        {
            if(a->index == actions->at(i)->index)
            { return false; }
        }
        return true;
    }

    void LandmarkRPG::addActionEffects(SASAction* a)
    {
        TVarValue code;
        for(unsigned int j = 0; j < a->startEff.size(); j++)
        {
            code = SASTask::getVariableValueCode(a->startEff[j].var, a->startEff[j].value);
            if(!fluentAchieved(code))
            {
                newLevel->push_back(code);
            }
        }
        for(unsigned int j = 0; j < a->endEff.size(); j++)
        {
            code = SASTask::getVariableValueCode(a->endEff[j].var, a->endEff[j].value);
            if(!fluentAchieved(code))
            {
                newLevel->push_back(code);
            }
        }
    }

    void LandmarkRPG::initialize(TState* s)
    {
        unsigned int numActions = task->actions.size();
        achievedAction = new bool[numActions];
        for(unsigned int i = 0; i < numActions; i++)
        { achievedAction[i] = false; }
        lastLevel = new std::vector<TVarValue>();
        lastLevel->reserve((s->numSASVars) << 1);
        newLevel = new std::vector<TVarValue>();
        newLevel->reserve((s->numSASVars) << 1);
        remainingGoals.reserve(s->numSASVars);
        for(unsigned int i = 0; i < s->numSASVars; i++)
        {
            TVarValue code = SASTask::getVariableValueCode(i, s->state[i]);
            lastLevel->push_back(code);
            achievedFluent[code] = true;
        }
        for(unsigned int i = 0; i < task->goals.size(); i++)
        {
            SASAction& g = task->goals[i];
            for(unsigned int j = 0; j < g.startCond.size(); j++)
            { addGoal(&(g.startCond[j])); }
            for(unsigned int j = 0; j < g.overCond.size(); j++)
            { addGoal(&(g.overCond[j])); }
            for(unsigned int j = 0; j < g.endCond.size(); j++)
            { addGoal(&(g.endCond[j])); }
        }
    }

    void LandmarkRPG::addGoal(SASCondition* c)
    {
        if(!fluentAchieved(c->var, c->value))
        {
            remainingGoals.push_back(SASTask::getVariableValueCode(c->var, c->value));
        }
    }

    void LandmarkRPG::clearMemory()
    {
        delete newLevel;
        delete lastLevel;
        delete[] achievedAction;
        achievedFluent.clear();
        remainingGoals.clear();
    }


/*******************************************/
/* LandmarkTree                            */
/*******************************************/

    LandmarkTree::LandmarkTree(TState* state, SASTask* task, std::vector<SASAction*>* tilActions)
    {
        this->state = state;
        this->task = task;
        rpg.initialize(false, task, tilActions);
        rpg.build(state);
        rpg.computeLiteralLevels();
        rpg.computeActionLevels(state);
        fluentNode.reserve(rpg.getFluentListSize());
        for(unsigned int i = 0; i < rpg.getFluentListSize(); i++)
        {
            fluentNode[i] = -1;
        }
        unsigned int numLevels = rpg.getNumFluentLevels();
        objs.reserve(numLevels);
        disjObjs.reserve(numLevels);
        for(unsigned int i = 0; i < numLevels; i++)
        {
            objs.emplace_back();
            disjObjs.emplace_back();
        }
        for(unsigned int i = 0; i < task->goals.size(); i++)
        {
            SASAction& g = task->goals[i];
            for(unsigned int j = 0; j < g.startCond.size(); j++)
            { addGoalNode(&(g.startCond[j]), state); }
            for(unsigned int j = 0; j < g.overCond.size(); j++)
            { addGoalNode(&(g.overCond[j]), state); }
            for(unsigned int j = 0; j < g.endCond.size(); j++)
            { addGoalNode(&(g.endCond[j]), state); }
        }
        exploreRPG();
        // Creating the adjacency matrix
        unsigned int n = nodes.size();
        matrix = new bool* [n];
        for(unsigned int j = 0; j < n; j++)
        {
            matrix[j] = new bool[n];
            for(unsigned int i = 0; i < n; i++)
            {
                matrix[j][i] = false;
            }
        }
        for(unsigned int i = 0; i < edges.size(); i++)
        {
            matrix[edges[i].node1->getIndex()][edges[i].node2->getIndex()] = true;
        }
        // Verifying necessary orderings
        postProcessing();
    }

    LandmarkTree::~LandmarkTree()
    {
        for(unsigned int i = 0; i < nodes.size(); i++)
        {
            delete[] matrix[i];
        }
        delete[] matrix;
        for(unsigned int i = 0; i < nodes.size(); i++)
        { delete nodes[i]; }
    }

    void LandmarkTree::addGoalNode(SASCondition* c, TState* state)
    {
        //if (state->state[c->var] == c->value) return;
        int index = rpg.getFluentIndex(c->var, c->value);
        if(index >= 0)
        {
            LMFluent* goal = rpg.getFluentByIndex(index);
            goal->isGoal = true;
            LTNode* n = new LTNode(goal, nodes.size());
            nodes.push_back(n);
            fluentNode[index] = n->getIndex();
            int levelIndex = rpg.getLevelIndex(goal->level);
            objs[levelIndex].push_back(new LMFluent(*goal));
        }
    }

// The RPG is explored backwards, beginning from the last literal level
    void LandmarkTree::exploreRPG()
    {
        int level = (int)rpg.getNumFluentLevels() - 1;
        while(level > 0)
        {
#ifdef DEBUG_LANDMARKS_ON
            cout << "EXPLORING LEVEL " << level << ", whith " << objs[level].size() << " items" << endl;
#endif
            for(unsigned int i = 0; i < objs[level].size(); i++)
            {
                LMFluent* obj = objs[level][i];
#ifdef DEBUG_LANDMARKS_ON
                cout << "* Obj: " << obj->toString(task) << endl;
#endif
                // calculate the set A of actions that produce the literal
                // Once 'a' is calculated, the action processing method is invoked
                // actionProcessing is only launched if there are producers, that is, if A is not an empty set
#ifdef DEBUG_LANDMARKS_ON
                for (unsigned int x = 0; x < obj->producers.size(); x++)
                cout << "* Producer: " << obj->producers[x]->name << endl;
#endif
                actionProcessing(&(obj->producers), nodes[fluentNode[obj->index]], level);
            }
            for(unsigned int i = 0; i < disjObjs[level].size(); i++)
            {
                USet* disjObj = disjObjs[level][i];
#ifdef DEBUG_LANDMARKS_ON
                cout << "* Dobj: " << disjObj->toString(task) << endl;
#endif
                // For each disjunction of literals in disjObjs[level], we calculate A
                // as the union of the producer actions of each literal in the disjunction
                std::vector<SASAction*> a;
                bool initialState = false;
                for(unsigned int j = 0; j < disjObj->fluentSet.size(); j++)
                {
                    if(disjObj->fluentSet[j]->level == 0)
                    {
                        initialState = true;
                        break;
                    }
                    for(unsigned int k = 0; k < disjObj->fluentSet[j]->producers.size(); k++)
                    {
                        a.push_back(disjObj->fluentSet[j]->producers[k]);
                    }
                }
                // Once A is calculated, the action processing method is invoked
                if(!initialState)
                {
#ifdef DEBUG_LANDMARKS_ON
                    for (unsigned int x = 0; x < a.size(); x++)
                    cout << "* Producer: " << a[x]->name << endl;
#endif
                    actionProcessing(&a, disjObj->node, level);
                }
            }
            level--;
        }
    }

    void LandmarkTree::actionProcessing(std::vector<SASAction*>* a, LTNode* g, int level)
    {
        if(a->size() == 0)
        { return; }
        std::vector<USet*> d;        // Calculating I set: preconditions that
        std::vector<LMFluent*> i;    // are common to all the actions in A
        std::vector<LMFluent*> u;
        unsigned int numFluents = rpg.getFluentListSize();
        int* common = new int[numFluents];
        for(unsigned int n = 0; n < numFluents; n++)
        { common[n] = 0; }
        for(unsigned int n = 0; n < a->size(); n++)
        {
#ifdef DEBUG_LANDMARKS_ON
            std::string name = a->at(n)->name;
        cout << "* Checking " << name.c_str() << endl;
#endif
            checkPreconditions(a->at(n), common);
        }
        for(unsigned int n = 0; n < numFluents; n++)
        {
            if(common[n] == (int)a->size())
            { i.push_back(rpg.getFluentByIndex(n)); }
            else if(common[n] > 0)
            { u.push_back(rpg.getFluentByIndex(n)); }
        }
        for(unsigned int n = 0; n < i.size(); n++)
        {    // Exploring candidate landmarks in I
            LMFluent* p = i[n];
#ifdef DEBUG_LANDMARKS_ON
            cout << " - Candidate: " << p->toString(task) << endl;
#endif
            if(verify(p))
            {
                // Adding landmark p to N, and transition p->g in E
                // The literal is stored only if it hasn't appeared before (it is ensured by checking literalNode)
                LTNode* node;
                if(fluentNode[p->index] == -1)
                {
                    node = new LTNode(p, nodes.size());
                    nodes.push_back(node);
                    fluentNode[p->index] = node->getIndex();
#ifdef DEBUG_LANDMARKS_ON
                    cout << "    * Added as landmark: " << node->toString(task) << endl;
#endif
                }
                else
                {
                    node = nodes[fluentNode[p->index]];
                }
                // Adding a new transition to E
                LMOrdering ordering;
                ordering.initialize(node, g, NECESSARY);
#ifdef DEBUG_LANDMARKS_ON
                cout << "    * Edges size = " << edges.size() << ": " << ordering.toString(task) << endl;
#endif
                edges.push_back(ordering);
                bool found = false;
                int candidateLevel = rpg.getLevelIndex(p->level);
#ifdef DEBUG_LANDMARKS_ON
                cout << "    * Candidate level: " << candidateLevel << endl;
#endif
                for(unsigned int m = 0; m < objs[candidateLevel].size(); m++)
                {
                    if(objs[candidateLevel][m]->index == p->index)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
#ifdef DEBUG_LANDMARKS_ON
                    cout << "    * Adding: " << p->toString(task) << endl;
#endif
                    objs[candidateLevel].push_back(node->getFluent());
                }
            }
        }
        // Exploring candidate disjunctive landmarks in D
        groupUSet(&d, &u, a);
        for(unsigned int n = 0; n < d.size(); n++)
        {
            USet* d2 = d[n];
            USet* d1 = findDisjObject(d2, level);
            if(d1 == nullptr)
            {
                if(verify(&(d2->fluentSet)))
                {
                    d2->node = new LTNode(d2, nodes.size());
                    nodes.push_back(d2->node);
                    edges.emplace_back();
                    edges.back().initialize(d2->node, g, NECESSARY);
                    int candidateLevel = rpg.getLevelIndex(d2->fluentSet[0]->level);
                    for(unsigned int m = 1; m < d2->fluentSet.size(); m++)
                    {
                        if(rpg.getLevelIndex(d2->fluentSet[m]->level) > candidateLevel)
                        {
                            candidateLevel = rpg.getLevelIndex(d2->fluentSet[m]->level);
                        }
                    }
                    disjObjs[candidateLevel].push_back(d2);
#ifdef DEBUG_LANDMARKS_ON
                    cout << "D2: " << d2->toString(task) << ", at level " << candidateLevel << endl;
#endif
                }
            }
            else
            {
                edges.emplace_back();
                edges.back().initialize(d1->node, g, NECESSARY);
            }
        }
        delete[] common;
    }

    USet* LandmarkTree::findDisjObject(USet* u, int level)
    {
        //cout << "Searching for " << u->toString(task) << endl;
        for(int i = level; i >= 0; i--)
        {
            for(int j = 0; j < (int)disjObjs[i].size(); j++)
            {
                //cout << "Comparing to " << disjObjs[i][j]->toString(task) << endl;
                if(u->isEqual(disjObjs[i][j]))
                {
                    return disjObjs[i][j];
                }
            }
        }
        return nullptr;
    }

    void LandmarkTree::groupUSet(std::vector<USet*>* res, std::vector<LMFluent*>* u, std::vector<SASAction*>* a)
    {
        res->clear();
        std::vector<USet*> hashU;
        std::vector<USet*> aux;
#ifdef DEBUG_LANDMARKS_ON
        cout << "U size: " << u->size() << endl;
#endif
        for(unsigned int i = 0; i < u->size(); i++)
        {
            LMFluent* l = u->at(i);
            unsigned int f = task->values[l->value].fncIndex;
#ifdef DEBUG_LANDMARKS_ON
            cout << "U: " << l->toString(task) << ", fnc = " << f << endl;
#endif
            if(f != FICTITIOUS_FUNCTION)
            {
                while(hashU.size() <= f)
                { hashU.push_back(nullptr); }
                if(hashU[f] == nullptr)
                {
                    hashU[f] = new USet();
                    hashU[f]->initialize(l, (int)f);
                    aux.push_back(hashU[f]);
                }
                else
                {
                    hashU[f]->addElement(l);
                }
            }
        }
        for(unsigned int i = 0; i < aux.size(); i++)
        {
            aux[i]->calculateValue();
        }
        // Verify if the uSets are correct
        // All the actions must have provided the uSet with at least a precondition of each type
        for(unsigned int i = 0; i < aux.size(); i++)
        {
            USet* s = aux[i];
            int instances = 0;
            int actions = 0;
            if(s->fluentSet.size() != 1)
            {
                for(unsigned int j = 0; j < a->size(); j++)
                {
                    SASAction* action = a->at(j);
                    bool visited = false;
                    for(unsigned int k = 0; k < action->startCond.size(); k++)
                    {
                        if(s->matches(action->startCond[k]))
                        {
                            instances++;
                            if(!visited)
                            {
                                actions++;
                                visited = true;
                            }
                        }
                    }
                    for(unsigned int k = 0; k < action->overCond.size(); k++)
                    {
                        if(s->matches(action->overCond[k]))
                        {
                            instances++;
                            if(!visited)
                            {
                                actions++;
                                visited = true;
                            }
                        }
                    }
                    for(unsigned int k = 0; k < action->endCond.size(); k++)
                    {
                        if(s->matches(action->endCond[k]))
                        {
                            instances++;
                            if(!visited)
                            {
                                actions++;
                                visited = true;
                            }
                        }
                    }
                }
                // If there is one instance per action, the uSet is added to res
                if(actions == (int)a->size() && instances == actions)
                {
#ifdef DEBUG_LANDMARKS_ON
                    cout << "Set added: " << s->toString(task) << endl;
#endif
                    res->push_back(new USet(s));
                }
                else if(actions == (int)a->size() && instances > (int)a->size())
                {
                    analyzeSet(s, a, res);
                }
            }
            delete s;
        }
    }

    void LandmarkTree::analyzeSet(USet* s, std::vector<SASAction*>* a, std::vector<USet*>* u1)
    {
        std::vector<std::vector<LMFluent*>> fluentProducers(a->size());
        // Grouping the literals in the set according to the actions that generated them
        for(unsigned int i = 0; i < a->size(); i++)
        {
            SASAction* action = a->at(i);
            for(unsigned int j = 0; j < action->startCond.size(); j++)
            {
                if((int)task->values[action->startCond[j].value].fncIndex == s->id)
                {
                    int n = rpg.getFluentIndex(action->startCond[j].var, action->startCond[j].value);
                    fluentProducers[i].push_back(rpg.getFluentByIndex(n));
                }
            }
            for(unsigned int j = 0; j < action->overCond.size(); j++)
            {
                if((int)task->values[action->overCond[j].value].fncIndex == s->id)
                {
                    int n = rpg.getFluentIndex(action->overCond[j].var, action->overCond[j].value);
                    fluentProducers[i].push_back(rpg.getFluentByIndex(n));
                }
            }
            for(unsigned int j = 0; j < action->endCond.size(); j++)
            {
                if((int)task->values[action->endCond[j].value].fncIndex == s->id)
                {
                    int n = rpg.getFluentIndex(action->endCond[j].var, action->endCond[j].value);
                    fluentProducers[i].push_back(rpg.getFluentByIndex(n));
                }
            }
        }
        // An uSet has only one element per action in A
        bool finish = false;
        for(unsigned int i = 0; i < fluentProducers[0].size() && !finish; i++)
        {
            LMFluent* l = fluentProducers[0][i];
            USet u;
            u.initialize(l, s->id);
            for(unsigned int j = 1; j < fluentProducers.size(); j++)
            {
                std::vector<LMFluent*>* actionFluents = &(fluentProducers[j]);
                if(actionFluents->size() == 0)
                {
                    finish = true;
                    break;
                }
                int k = equalParameters(l, actionFluents);
                LMFluent* similar = actionFluents->at(k);
                actionFluents->erase(actionFluents->begin() + k);
                u.addElement(similar);
            }
            u1->push_back(new USet(&u));
        }
    }

    bool LandmarkTree::verify(LMFluent* p)
    {
        if(p->isGoal)
        { return true; }
        LandmarkRPG r;
        return r.verifyFluent(p->variable, p->value, state, task);
    }

    bool LandmarkTree::verify(std::vector<LMFluent*>* v)
    {
        LandmarkRPG r;
        std::vector<TVariable> var;
        std::vector<TValue> val;
        for(unsigned int i = 0; i < v->size(); i++)
        {
            if(!v->at(i)->isGoal)
            {
                var.push_back(v->at(i)->variable);
                val.push_back(v->at(i)->value);
            }
        }
        if(var.empty())
        { return true; }
        return r.verifyFluents(&var, &val, state, task);
    }

    bool LandmarkTree::verify(std::vector<SASAction*>* a)
    {
        LandmarkRPG r;
        return r.verifyActions(a, state, task);
    }

    void LandmarkTree::checkPreconditions(SASAction* a, int* common)
    {
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            common[rpg.getFluentIndex(a->startCond[i].var, a->startCond[i].value)]++;
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            common[rpg.getFluentIndex(a->overCond[i].var, a->overCond[i].value)]++;
        }
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            common[rpg.getFluentIndex(a->endCond[i].var, a->endCond[i].value)]++;
        }
    }

    int LandmarkTree::equalParameters(LMFluent* l, std::vector<LMFluent*>* actionFluents)
    {
        std::vector<int> candidates = std::vector<int>();
        std::vector<int> auxCandidates = std::vector<int>();
        SASValue& p1 = task->values[l->value];
        for(unsigned int i = 0; i < actionFluents->size(); i++)
        {
            candidates.push_back((int)i);
        }
        // Check if the candidate and the target literal are equal
        for(unsigned int i = 0; i < candidates.size(); i++)
        {
            int c = candidates[i];
            SASValue& p2 = task->values[actionFluents->at(c)->value];
            if(p1.name.compare(p2.name) == 0)
            {
                auxCandidates.push_back(c);
            }
        }
        // If there is only one candidate left, return it
        if(auxCandidates.size() == 1)
        { return auxCandidates[0]; }
        if(auxCandidates.size() > 1)
        {
            candidates = auxCandidates;
        }
        auxCandidates.clear();
        int candidate = candidates[0];
        //cout << "Incomplete code at equalParameters method!" << endl;
        // Check which candidate has most parameters of the same type than the target literal
        /*
        int32_t min = MAX_INT32;
        for i := 0 to candidates.size - 1 do begin
            c := candidates.items[i];
            equalParameters := 0;
            p2 := gbGTask.variables.items[actionLiterals.items[c].variable];
            for j := 0 to p2.numParams - 1 do begin
                o1 := gbTask.objects.items[p1.params[j]];
                o2 := gbTask.objects.items[p2.params[j]];
                if o1.isCompatible(o2, gbTask) then
                Inc(equalParameters);
            end;
            if min > equalParameters then begin
                min := equalParameters;
                candidate := c;
            end;
        end;
        */
        return candidate;
    }

    void LandmarkTree::postProcessing()
    {
        // P: Candidate landmarks that precede necessarily a given candidate landmark g
        std::vector<LMFluent*> p;
        // A: Actions that produce a landmark g
        std::vector<SASAction*> a;
        // We analyze all the literal nodes g of the Landmark Tree
        for(unsigned int i = 0; i < nodes.size(); i++)
        {    // Only single literals are processed
            if(nodes[i]->single())
            {
                for(unsigned int j = 0; j < nodes.size(); j++)
                { // Check g column of the matrix to find literals l such that l <= n g
                    if(matrix[j][i] && nodes[j]->single())
                    {
#ifdef DEBUG_LANDMARKS_ON
                        cout << "PP: " << nodes[j]->toString(task) << " -> " << nodes[i]->toString(task) << endl;
#endif
                        getActions(&a, nodes[j]->getFluent(), nodes[i]->getFluent());
                        // We check if the actions in A are necessary to reach the goals
                        if(!verify(&a))
                        {
#ifdef DEBUG_LANDMARKS_ON
                            cout << "Removed" << endl;
#endif
                            matrix[j][i] = false;
                            // We also remove the ordering from the orderings list
                            unsigned int ord = 0;
                            while(ord < edges.size())
                            {
                                LMOrdering* e = &(edges[ord]);
                                if(e->node1->single() && e->node2->single() && e->node1->getIndex() == j &&
                                   e->node2->getIndex() == i)
                                {
                                    edges.erase(edges.begin() + ord);
                                }
                                else
                                {
                                    ord++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void LandmarkTree::getActions(std::vector<SASAction*>* aList, LMFluent* l1, LMFluent* l2)
    {
        aList->clear();
        if(rpg.getFluentByIndex(l1->index) == nullptr || rpg.getFluentByIndex(l2->index) == nullptr)
        { return; }
        std::vector<SASAction*>& producers = task->producers[l2->variable][l2->value];
        for(unsigned int i = 0; i < producers.size(); i++)
        {
            SASAction* a = producers[i];
            bool added = false;
            for(unsigned int j = 0; j < a->startCond.size() && !added; j++)
            {
                int n = rpg.getFluentIndex(a->startCond[j].var, a->startCond[j].value);
                if(l1->index == n)
                {
                    aList->push_back(a);
                    added = true;
                }
            }
            for(unsigned int j = 0; j < a->overCond.size() && !added; j++)
            {
                int n = rpg.getFluentIndex(a->overCond[j].var, a->overCond[j].value);
                if(l1->index == n)
                {
                    aList->push_back(a);
                    added = true;
                }
            }
            for(unsigned int j = 0; j < a->endCond.size() && !added; j++)
            {
                int n = rpg.getFluentIndex(a->endCond[j].var, a->endCond[j].value);
                if(l1->index == n)
                {
                    aList->push_back(a);
                    added = true;
                }
            }
#ifdef DEBUG_LANDMARKS_ON
            if (added) cout << " * Action added: " << a->name << endl;
#endif
        }
    }


/*******************************************/
/* Landmarks                               */
/*******************************************/

    Landmarks::Landmarks(TState* state, SASTask* task, std::vector<SASAction*>* tilActions)
    {
        LandmarkTree lt(state, task, tilActions);
        std::unordered_map<int, int> mapping;
        for(unsigned int i = 0; i < lt.nodes.size(); i++)
        {
            LandmarkNode node((int)i, lt.nodes[i]);
            mapping[lt.nodes[i]->getIndex()] = (int)i;
            nodes.push_back(node);
#ifdef DEBUG_LANDMARKS_ON
            cout << "Copying node " << i << endl;
        cout << nodes[i].toString(task) << endl;
#endif
        }
        for(unsigned int i = 0; i < lt.edges.size(); i++)
        {
            LMOrdering& o = lt.edges[i];
            int index1 = mapping[o.node1->getIndex()];
            int index2 = mapping[o.node2->getIndex()];
            nodes[index1].addAdjacent(&(nodes[index2]));
#ifdef DEBUG_LANDMARKS_ON
            cout << index1 << " ------> " << index2 << endl;
#endif
        }
    }

    void Landmarks::filterTransitiveOrders(SASTask* task)
    {
        for(unsigned int i = 0; i < nodes.size(); i++)
        {
            unsigned int j = 0;
            int n1 = nodes[i].getIndex();
            while(j < nodes[i].numAdjacents())
            {
                int n2 = nodes[i].getAdjacent(j)->getIndex();
                bool indirectReachable = checkIndirectReachability(n1, n2);
                if(indirectReachable)
                {
                    nodes[i].deleteAdjacent(j);
                    //cout << "Delete " << n1 << " to " << n2 << endl;
                }
                else
                {
                    j++;
                }
            }
        }
    }

    bool Landmarks::checkIndirectReachability(int orig, int dst)
    {
        std::vector<bool> visited(nodes.size(), false);
        return checkIndirectReachability(orig, orig, dst, &visited);
    }

    bool Landmarks::checkIndirectReachability(int orig, int current, int dst, std::vector<bool>* visited)
    {
        (*visited)[current] = true;
        unsigned int numAdj = nodes[current].numAdjacents();
        for(unsigned int i = 0; i < numAdj; i++)
        {
            int adj = nodes[current].getAdjacent(i)->getIndex();
            if(!visited->at(adj))
            {
                if(adj == dst && current != orig)
                { return true; }
                if(adj != dst && checkIndirectReachability(orig, adj, dst, visited))
                { return true; }
            }
        }
        return false;
    }

    std::string Landmarks::toString(SASTask* task)
    {
        std::string res = "LANDMARKS:\n";
        unsigned int n = nodes.size();
        for(unsigned int i = 0; i < n; i++)
        {
            //cout << i << endl;
            //cout << nodes[i].toString(task) << endl;
            res += nodes[i].toString(task) + "\n";
            unsigned int na = nodes[i].numAdjacents();
            for(unsigned int j = 0; j < na; j++)
            {
                res += "    -> " + nodes[i].getAdjacent(j)->toString(task) + "\n";
            }
        }
        return res;
    }
}