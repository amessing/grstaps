#ifndef GRSTAPS_STATE_HPP
#define GRSTAPS_STATE_HPP

#include "grstaps/task_planning/sas_task.hpp"

namespace grstaps
{
    class TState
    {
    public:
        unsigned int numSASVars;    // Number of SAS variables
        unsigned int numNumVars;    // Number of numeric variables
        TValue* state;                // Values of the SAS variables in the state
        float* numState;            // Values of the numeric variables in the state

        TState(unsigned int numSASVars, unsigned int numNumVars);

        TState(SASTask* task);

        TState(TState* s);

        ~TState();

        inline void setSASValue(TVariable var, TValue value)
        {
            state[var] = value;
        }

        inline void setNumValue(TVariable var, char op, float value)
        {
            switch(op)
            {
                case '=':
                    numState[var] = value;
                    break;
                case '+':
                    numState[var] += value;
                    break;
                case '-':
                    numState[var] -= value;
                    break;
                case '*':
                    numState[var] *= value;
                    break;
                default:
                    if(value != 0)
                    { numState[var] /= value; }
                    else
                    { numState[var] = FLOAT_INFINITY; }
                    break;
            }
        }

        inline uint64_t getCode()
        {
            uint64_t code = 0;
            for(unsigned int i = 0; i < numSASVars; i++)
            {
                code = 31 * code + state[i];
            }
            for(unsigned int i = 0; i < numNumVars; i++)
            {
                code = 31 * code + ((uint64_t)(100 * numState[i]));
            }
            return code;
        }

        std::string toString(SASTask* task);

        inline bool compareTo(TState* s)
        {
            for(unsigned int i = 0; i < numNumVars; i++)
            {
                if(numState[i] != s->numState[i])
                { return false; }
            }
            for(unsigned int i = 0; i < numSASVars; i++)
            {
                if(state[i] != s->state[i])
                { return false; }
            }
            return true;
        }

        inline static void setNumVarValue(TVariable var, char op, float value, float* numState)
        {
            switch(op)
            {
                case '=':
                    numState[var] = value;
                    break;
                case '+':
                    numState[var] += value;
                    break;
                case '-':
                    numState[var] -= value;
                    break;
                case '*':
                    numState[var] *= value;
                    break;
                default:
                    if(value != 0)
                    { numState[var] /= value; }
                    else
                    { numState[var] = FLOAT_INFINITY; }
                    break;
            }
        }

        inline bool isExecutable(SASAction* a)
        {
            for(unsigned int i = 0; i < a->startCond.size(); i++)
            {
                if(!holdsCondition(&(a->startCond[i])))
                { return false; }
            }
            for(unsigned int i = 0; i < a->overCond.size(); i++)
            {
                if(!holdsCondition(&(a->overCond[i])))
                { return false; }
            }
            for(unsigned int i = 0; i < a->endCond.size(); i++)
            {
                if(!holdsCondition(&(a->endCond[i])))
                { return false; }
            }
            return true;
        }

        inline bool holdsCondition(SASCondition* c)
        {
            return state[c->var] == c->value;
        }

    };
}

#endif //GRSTAPS_STATE_HPP
