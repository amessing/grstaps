#include "grstaps/task_planning/state.hpp"

namespace grstaps
{
    TState::TState(unsigned int numSASVars, unsigned int numNumVars)
    {
        this->numSASVars = numSASVars;
        this->numNumVars = numNumVars;
        state            = new TValue[numSASVars];
        numState         = new float[numNumVars];
    }

    TState::TState(SASTask* task)
        : TState(task->variables.size(), task->numVariables.size())
    {  // Create the initial state
        for(unsigned int i = 0; i < numSASVars; i++)
        {
            state[i] = task->initialState[i];
        }
        for(unsigned int i = 0; i < numNumVars; i++)
        {
            numState[i] = task->numInitialState[i];
        }
    }

    TState::TState(TState* s)
    {
        this->numSASVars = s->numSASVars;
        this->numNumVars = s->numNumVars;
        state            = new TValue[numSASVars];
        numState         = new float[numNumVars];
        for(unsigned int i = 0; i < numSASVars; i++)
        {
            state[i] = s->state[i];
        }
        for(unsigned int i = 0; i < numNumVars; i++)
        {
            numState[i] = s->numState[i];
        }
    }

    TState::~TState()
    {
        delete[] state;
        delete[] numState;
    }

    std::string TState::toString(SASTask* task)
    {
        std::string res = "STATE\n";
        for(unsigned int i = 0; i < numSASVars; i++)
        {
            res += "* " + task->variables[i].name + " = " + task->values[state[i]].name + "\n";
        }
        for(unsigned int i = 0; i < numNumVars; i++)
        {
            res += "* " + task->numVariables[i].name + " = " + std::to_string(numState[i]) + "\n";
        }
        return res;
    }

}  // namespace grstaps
