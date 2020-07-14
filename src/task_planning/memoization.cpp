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
#include "grstaps/task_planning/memoization.hpp"

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/state.hpp"

namespace grstaps
{
#define INITIAL_MEMO_SIZE    15121

/********************************************************/
/* CLASS: Memoization                                   */
/********************************************************/

    Memoization::Memoization()
    {
        task = nullptr;
        memo.reserve(INITIAL_MEMO_SIZE);
    }

    void Memoization::initialize(SASTask* task)
    {
        this->task = task;
        initialState = new TState(task);
        linearizer.setInitialState(initialState, task);
        isRepeatedState(nullptr, initialState);
    }

    bool Memoization::isRepeatedState(Plan* p, TState* state)
    {
        uint64_t code = state->getCode();
        std::unordered_map<uint64_t, std::vector<Plan*>*>::const_iterator got = memo.find(code);
        if(got == memo.end())
        {    // New state
            memo[code] = new std::vector<Plan*>(1, p);
            return false;
        }
        else
        {
            std::vector<Plan*>* collisions = got->second;
            for(unsigned int i = 0; i < collisions->size(); i++)
            {
                Plan* pc = collisions->at(i);
                if(pc == nullptr)
                {
                    if(state->compareTo(initialState))
                    {
                        return true;
                    }
                }
                else if(sameState(state, pc))
                {
                    if(p->gc >= pc->gc)
                    {
                        return true;    // Same state and worse g
                    }
                    else
                    {                                // Same state but better g
                        (*collisions)[i] = p;
                        return false;
                    }
                }
            }
            collisions->push_back(p);
            return false;
        }
    }

    bool Memoization::sameState(TState* state, Plan* pc)
    {
        linearizer.setCurrentBasePlan(pc);
        linearizer.setCurrentPlan(nullptr);
        TState* sc = linearizer.getFrontierState(task, nullptr);
        if(sc == nullptr)
        {
            return true;
        }
        bool equal = state->compareTo(sc);
        delete sc;
        return equal;
    }

    void Memoization::clear()
    {
        memo.clear();
    }
}