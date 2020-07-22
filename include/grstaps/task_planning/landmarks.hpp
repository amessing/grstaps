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

#ifndef GRSTAPS_LANDMARKS_HPP
#define GRSTAPS_LANDMARKS_HPP

// global
#include <algorithm>
#include <vector>

// local
#include "grstaps/task_planning/temporal_rpg.hpp"

namespace grstaps
{
    class LTNode;

    class USet {				// Disjunctive landmark
    public:
        int id;
        std::vector<LMFluent*> fluentSet;
        LTNode* node;
        int value;

        USet() { }
        USet(USet* s) {
            id = s->id;
            for (unsigned int i = 0; i < s->fluentSet.size(); i++)
                fluentSet.push_back(s->fluentSet[i]);
            node = s->node;
            value = s->value;
        }
        void initialize(LMFluent *l, int fncIndex) {
            id = fncIndex;
            fluentSet.push_back(l);
            node = nullptr;
            value = 0;
        }
        void addElement(LMFluent *l) {
            if (!contains(l)) {
                fluentSet.push_back(l);
            }
        }
        bool contains(LMFluent *l) {
            for (unsigned int i = 0; i < fluentSet.size(); i++) {
                if (fluentSet[i]->index == l->index) {
                    return true;
                }
            }
            return false;
        }
        void calculateValue() {
            value = 0;
            for (unsigned int i = 0; i < fluentSet.size(); i++) {
                value += fluentSet[i]->index;
            }
        }
        bool matches(SASCondition &c) {
            for (unsigned int i = 0; i < fluentSet.size(); i++)
            {
                if (fluentSet[i]->variable == c.var && fluentSet[i]->value == c.value)
                    return true;
            }
            return false;
        }
        bool isEqual(USet* u) {
            if (id != u->id || fluentSet.size() != u->fluentSet.size()) return false;
            for (unsigned int i = 0; i < fluentSet.size(); i++) {
                if (fluentSet[i]->variable != u->fluentSet[i]->variable ||
                    fluentSet[i]->value != u->fluentSet[i]->value)
                    return false;
            }
            return true;
        }
        std::string toString(SASTask* task) {
            std::string res = "[";
            if (!fluentSet.empty()) res += fluentSet[0]->toString(task);
            for (unsigned int i = 1; i < fluentSet.size(); i++) {
                res += "," + fluentSet[i]->toString(task);
            }
            return res + "]";
        }
    };

    class LTNode {				// Landmark node
    private:
        LMFluent* fluent;
        USet* disjunction;
        bool singleLiteral;
        unsigned int index;

    public:
        LTNode() { index = MAX_INT32; }
        LTNode(LMFluent* f, unsigned int i) {
            fluent = new LMFluent(*f);
            disjunction = nullptr;
            singleLiteral = true;
            index = i;
        }
        LTNode(USet* u, unsigned int i) {
            fluent = nullptr;
            disjunction = u;
            singleLiteral = false;
            index = i;
        }
        inline unsigned int getIndex() { return index; }
        inline LMFluent* getFluent() { return fluent; }
        inline USet* getSet() { return disjunction; }
        inline bool single() { return singleLiteral; }
        std::string toString(SASTask* task) {
            if (singleLiteral) return "Node " + std::to_string(index) + ": " + fluent->toString(task);
            else return "Node " + std::to_string(index) + ": " + disjunction->toString(task);
        }
    };

    class LMOrdering {			// Landmark ordering
    public:
        LTNode* node1;
        LTNode* node2;
        int ordType;
        void initialize(LTNode* l1, LTNode* l2, int t) {
            node1 = l1;
            node2 = l2;
            ordType = t;
        }
        std::string toString(SASTask* task) {
            return node1->toString(task) + " -> " + node2->toString(task);
        }
    };

    class LandmarkRPG {
    private:
        SASTask* task;
        std::unordered_map<TVarValue, bool> achievedFluent;
        bool* achievedAction;
        std::vector<TVarValue> *lastLevel;
        std::vector<TVarValue> *newLevel;
        std::vector<TVarValue> remainingGoals;

        void initialize(TState* s);
        void addGoal(SASCondition* c);
        inline bool fluentAchieved(TVariable v, TValue value) {
            return fluentAchieved(SASTask::getVariableValueCode(v, value));
        }
        inline bool fluentAchieved(TVarValue vv) {
            return achievedFluent.find(vv) != achievedFluent.end();
        }
        bool isExecutable(SASAction* a, TVariable v, TValue value);
        bool isExecutable(SASAction* a, std::vector<TVariable>* v, std::vector<TValue>* value);
        bool isExecutable(SASAction* a);
        void addActionEffects(SASAction* a);
        void swapLevels();
        void clearMemory();
        bool allowedAction(SASAction* a, std::vector<SASAction*>* actions);

    public:
        bool verifyFluent(TVariable v, TValue value, TState* s, SASTask* task);
        bool verifyFluents(std::vector<TVariable>* v, std::vector<TValue>* value, TState* s, SASTask* task);
        bool verifyActions(std::vector<SASAction*>* actions, TState* s, SASTask* task);
    };

    class LandmarkTree {
    private:
        TState* state;
        TemporalRPG rpg;
        SASTask* task;
        std::vector<int> fluentNode;
        std::vector< std::vector< LMFluent* > > objs;
        std::vector< std::vector< USet* > > disjObjs;
        bool** reasonableOrderings;
        bool** matrix;
        bool** mutexMatrix;
        std::vector<LMOrdering> reasonableOrderingsGoalsList;

        void addGoalNode(SASCondition* c, TState* state);
        void exploreRPG();
        void actionProcessing(std::vector<SASAction*> *a, LTNode* g, int level);
        void checkPreconditions(SASAction* a, int* common);
        bool verify(LMFluent* p);
        bool verify(std::vector<LMFluent*>* v);
        bool verify(std::vector<SASAction*>* a);
        void groupUSet(std::vector<USet*>* res, std::vector<LMFluent*>* u, std::vector<SASAction*>* a);
        void analyzeSet(USet* s, std::vector<SASAction*>* a, std::vector<USet*>* u1);
        int equalParameters(LMFluent* l, std::vector<LMFluent*>* actionFluents);
        USet* findDisjObject(USet* u, int level);
        void postProcessing();
        void getActions(std::vector<SASAction*>* aList, LMFluent* l1, LMFluent* l2);

    public:
        std::vector<LTNode*> nodes;
        std::vector<LMOrdering> edges;

        LandmarkTree(TState* state, SASTask* task, std::vector<SASAction*>* tilActions);
        ~LandmarkTree();
    };

    class LandmarkNode {
    private:
        int index;
        std::vector<TVariable> variables;
        std::vector<TValue> values;
        std::vector<LandmarkNode*> nextNodes;

    public:
        LandmarkNode(int index, LTNode* n) {
            this->index = index;
            if (n->single()) {
                variables.push_back(n->getFluent()->variable);
                values.push_back(n->getFluent()->value);
            } else {
                std::vector<LMFluent*>* fs = &(n->getSet()->fluentSet);
                for (unsigned int i = 0; i < fs->size(); i++) {
                    variables.push_back(fs->at(i)->variable);
                    values.push_back(fs->at(i)->value);
                }
            }
        }
        void addAdjacent(LandmarkNode* nextNode) {
            std::vector<LandmarkNode*>::iterator it = find(nextNodes.begin(), nextNodes.end(), nextNode);
            if (it == nextNodes.end()) {
                nextNodes.push_back(nextNode);
            }
        }
        std::string toString(SASTask* task) {
            std::string res = "Node " + std::to_string(index) + ": (" + task->variables[variables[0]].name
                              + "," + task->values[values[0]].name + ")";
            for (unsigned int i = 1; i < variables.size(); i++) {
                res += ",(" + task->variables[variables[i]].name + "," + task->values[values[i]].name + ")";
            }
            return res;
        }
        inline unsigned int numAdjacents() {
            return nextNodes.size();
        }
        inline LandmarkNode* getAdjacent(unsigned int i) {
            return nextNodes[i];
        }
        inline int getIndex() {
            return index;
        }
        void deleteAdjacent(unsigned int i) {
            nextNodes.erase(nextNodes.begin() + i);
        }
        unsigned int getNumFluents() {
            return variables.size();
        }
        TVariable getVariable(unsigned int i) {
            return variables[i];
        }
        TValue getValue(unsigned int i) {
            return values[i];
        }
    };

    class Landmarks {
    private:
        std::vector<LandmarkNode> nodes;

        bool checkIndirectReachability(int orig, int dst);
        bool checkIndirectReachability(int orig, int current, int dst, std::vector<bool> *visited);

    public:
        Landmarks(TState* state, SASTask* task, std::vector<SASAction*>* tilActions);
        void filterTransitiveOrders(SASTask* task);
        unsigned int numNodes() { return nodes.size(); }
        LandmarkNode* getNode(unsigned int index) { return &(nodes[index]); }
        std::string toString(SASTask* task);
    };
}

#endif //GRSTAPS_LANDMARKS_HPP
