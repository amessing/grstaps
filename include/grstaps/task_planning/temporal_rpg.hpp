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

#ifndef GRSTAPS_TEMPORAL_RPG_HPP
#define GRSTAPS_TEMPORAL_RPG_HPP

// local
#include "grstaps/task_planning/priority_queue.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class TState;

    class FluentLevel : public PriorityQueueItem
    {        // Level of a(sub)goal
    public:
        TVariable variable;
        TValue value;
        float level;

        FluentLevel(TVariable var, TValue val, float lev)
        {
            variable = var;
            value = val;
            level = lev;
        }

        virtual inline int compare(PriorityQueueItem* other)
        {
            float otherLevel = ((FluentLevel*)other)->level;
            if(level < otherLevel)
            { return -1; }
            else if(level > otherLevel)
            { return 1; }
            else
            { return 0; }
        }

        std::string toString(SASTask* task)
        {
            return "(" + task->variables[variable].name + "," + task->values[value].name + ") -> " +
                   std::to_string(level);
        }
    };

    class LMFluent
    {    // Landmark literal
    public:
        TVariable variable;
        TValue value;
        float level;
        int index;
        bool isGoal;
        std::vector<SASAction*> producers;

        LMFluent()
        {}

        LMFluent(LMFluent* cp)
        {
            variable = cp->variable;
            value = cp->value;
            level = cp->level;
            index = cp->index;
            isGoal = cp->isGoal;
        }

        void initialize(TVarValue vv, float lev, int i)
        {
            variable = SASTask::getVariableIndex(vv);
            value = SASTask::getValueIndex(vv);
            level = lev;
            index = i;
            isGoal = false;
        }

        std::string toString(SASTask* task)
        {
            return "[" + std::to_string(index) + "]: (" + task->variables[variable].name + "," +
                   task->values[value].name + ") -> " + std::to_string(level);
        }
    };

    class TemporalRPG
    {
    private:
        SASTask* task;
        int numActions;
        std::unordered_map<TVarValue, float> firstGenerationTime;
        PriorityQueue qPNormal;
        bool untilGoals;
        std::vector <TVarValue> goalsToAchieve;
        bool verifyFluent;
        LMFluent fluentToVerify;
        char* visitedAction;
        std::vector <LMFluent> fluentList;                        // List of fluents (ComputeLiterals)
        std::unordered_map<TVarValue, int> fluentIndex;            // [variable,value] -> fluent index
        std::vector <std::vector<TVarValue>> fluentLevels;        // List of fluents at each level
        std::unordered_map<float, int> fluentLevelIndex;        // Time -> level
        float* actionLevels;                                    // Starting time of each action
        std::vector<SASAction*>* tilActions;

        void addGoalToAchieve(SASCondition& c);

        void init(TState* state);

        inline float getFirstGenerationTime(TVariable v, TValue value)
        {
            return getFirstGenerationTime(SASTask::getVariableValueCode(v, value));
        }

        inline float getFirstGenerationTime(TVarValue vv)
        {
            std::unordered_map<TVarValue, float>::const_iterator got = firstGenerationTime.find(vv);
            if(got == firstGenerationTime.end())
            { return -1; }
            else
            { return got->second; }
        }

        bool checkAcheivedGoals();

        bool actionProducesFluent(SASAction* a);

        void clearPriorityQueue();

        float getActionLevel(SASAction* a, TState* state);

        void programAction(SASAction* a, TState* state);

    public:
        void initialize(bool untilGoals, SASTask* task, std::vector<SASAction*>* tilActions);

        ~TemporalRPG();

        void build(TState* state);

        void computeLiteralLevels();

        void computeActionLevels(TState* state);

        inline unsigned int getFluentListSize()
        { return fluentList.size(); }

        inline unsigned int getNumFluentLevels()
        { return fluentLevels.size(); }

        inline int getFluentIndex(TVariable v, TValue value)
        {
            std::unordered_map<TVarValue, int>::const_iterator got = fluentIndex.find(SASTask::getVariableValueCode(v,
                                                                                                                    value));
            if(got == fluentIndex.end())
            { return -1; }
            else
            { return got->second; }
        }

        inline LMFluent* getFluentByIndex(int index)
        { return &fluentList[index]; }

        inline int getLevelIndex(float level)
        {
            std::unordered_map<float, int>::const_iterator got = fluentLevelIndex.find(level);
            if(got == fluentLevelIndex.end())
            { return -1; }
            else
            { return got->second; }
        }
    };
}

#endif //GRSTAPS_TEMPORAL_RPG_HPP
