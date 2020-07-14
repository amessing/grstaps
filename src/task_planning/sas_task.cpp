#include "grstaps/task_planning/sas_task.hpp"

#include <cassert>
#include <iostream>

namespace grstaps
{
    /********************************************************/
/* CLASS: SASValue                                   */
/********************************************************/

    std::string SASValue::toString()
    {
        return std::to_string(index) + ":" + name;
    }

/********************************************************/
/* CLASS: SASVariable                                   */
/********************************************************/

    void SASVariable::addPossibleValue(unsigned int value)
    {
        possibleValues.push_back(value);
    }

    void SASVariable::addInitialValue(unsigned int sasValue, bool isTrue, float timePoint)
    {
        unsigned int index = std::numeric_limits<unsigned int>::max();
        for(unsigned int i = 0; i < possibleValues.size(); i++)
        {
            if(possibleValues[i] == sasValue)
            {
                index = i;
                break;
            }
        }
        if(index == std::numeric_limits<unsigned int>::max())
        {
            std::cout << "Invalid intial value " << sasValue << " for variable " << name << std::endl;
            assert(false);
        }
        if(isTrue)
        {
            value.push_back(sasValue);
            time.push_back(timePoint);
        }
        else if(possibleValues.size() == 2)
        {
            index = index == 0 ? 1 : 0;
            value.push_back(possibleValues[index]);
            time.push_back(timePoint);
        }
        else
        {
            std::cout << "Unable to translate negated initial value for variable " << name << std::endl;
            assert(false);
        }
    }

    std::string SASVariable::toString()
    {
        return std::to_string(index) + ":" + name;
    }

    std::string SASVariable::toString(std::vector <SASValue>& values)
    {
        std::string s = toString();
        for(unsigned int i = 0; i < possibleValues.size(); i++)
        {
            s += "\n* " + values[possibleValues[i]].name;
        }
        return s;
    }

    std::string SASVariable::toStringInitialState(std::vector <SASValue>& values)
    {
        std::string s = "";
        for(unsigned int i = 0; i < value.size(); i++)
        {
            if(i > 0)
            { s += " "; }
            s += "(at " + std::to_string(time[i]) + " (= " + name + " " + values[value[i]].name + "))";
        }
        if(value.size() == 0)
        { s = "Uninitialized: " + name; }
        return s;
    }

    unsigned int SASVariable::getOppositeValue(unsigned int v)
    {
        if(possibleValues.size() != 2)
        {
            std::cout << "Unable to translate negated initial value for variable " << name << std::endl;
            assert(false);
        }
        if(possibleValues[0] == v)
        { return possibleValues[1]; }
        if(possibleValues[1] == v)
        { return possibleValues[0]; }
        std::cout << "Invalid value " << v << " for variable " << name << std::endl;
        assert(false);
        return 0;
    }

    TVarValue SASVariable::getInitialStateValue()
    {
        for(unsigned int i = 0; i < time.size(); i++)
        {
            if(time[i] == 0)
            { return value[i]; }
        }
        return MAX_INT32;
    }

    int SASVariable::getPossibleValueIndex(unsigned int value)
    {
        for(unsigned int i = 0; i < possibleValues.size(); i++)
        {
            if(possibleValues[i] == value)
            {
                return (int)i;
            }
        }
        return -1;
    }

/********************************************************/
/* CLASS: NumericVariable                               */
/********************************************************/

    void NumericVariable::addInitialValue(float value, float time)
    {
        for(unsigned int i = 0; i < this->value.size(); i++)
        {
            if(time == this->time[i])
            {
                if(this->value[i] == value)
                { break; }
                std::cout << "Contradictory value " << value << " in time " << time << " for variable " << name << std::endl;
                assert(false);
            }
        }
        this->value.push_back(value);
        this->time.push_back(time);
    }

    std::string NumericVariable::toString()
    {
        return std::to_string(index) + ":" + name;
    }

    std::string NumericVariable::toStringInitialState()
    {
        std::string s = "";
        for(unsigned int i = 0; i < value.size(); i++)
        {
            if(i > 0)
            { s += " "; }
            s += "(at " + std::to_string(time[i]) + " (= " + name + " " + std::to_string(value[i]) + "))";
        }
        if(value.size() == 0)
        { s = "Uninitialized: " + name; }
        return s;
    }

    float NumericVariable::getInitialStateValue()
    {
        for(unsigned int i = 0; i < time.size(); i++)
        {
            if(time[i] == 0)
            { return value[i]; }
        }
        return 0.0f;
    }

/********************************************************/
/* CLASS: SASNumericExpression                          */
/********************************************************/

    std::string SASNumericExpression::toString(std::vector <NumericVariable>* numVariables)
    {
        std::string s;
        switch(type)
        {
            case 'N':
                s = std::to_string(value);
                break;
            case 'V':
                s = (*numVariables)[var].name;
                break;
            case '+':
            case '-':
            case '/':
            case '*':
                s = "(" + type;
                for(unsigned int i = 0; i < terms.size(); i++)
                {
                    s += " " + terms[i].toString(numVariables);
                }
                break;
            case '#':
                if(terms.size() == 0)
                { s = "#t"; }
                else
                { s = "(* #t " + terms[0].toString(numVariables) + ")"; }
                break;
            default:
                s = "?duration";
        }
        return s;
    }

/********************************************************/
/* CLASS: SASDuration                                   */
/********************************************************/

    std::string SASDuration::toString(std::vector <NumericVariable>* numVariables)
    {
        return SASTask::toStringTime(time) + "(" + SASTask::toStringComparator(comp) + " ?duration " +
               exp.toString(numVariables) + ")";
    }

/********************************************************/
/* CLASS: SASCondition                                  */
/********************************************************/

    SASCondition::SASCondition(unsigned int var, unsigned int value)
    {
        this->var = var;
        this->value = value;
        this->isModified = false;
    }

/********************************************************/
/* CLASS: SASNumericCondition                           */
/********************************************************/

    std::string SASNumericCondition::toString(std::vector <NumericVariable>* numVariables)
    {
        std::string s = "(" + SASTask::toStringComparator(comp);
        for(unsigned int i = 0; i < terms.size(); i++)
        {
            s += " " + terms[i].toString(numVariables);
        }
        return s + ")";
    }

/********************************************************/
/* CLASS: SASNumericEffect                              */
/********************************************************/

    std::string SASNumericEffect::toString(std::vector <NumericVariable>* numVariables)
    {
        return "(" + SASTask::toStringAssignment(op) + " " + (*numVariables)[var].toString() + " " +
               exp.toString(numVariables) + ")";
    }

/********************************************************/
/* CLASS: SASTask                                       */
/********************************************************/

// Constructor
    SASTask::SASTask()
    {
        createNewValue("<true>", FICTITIOUS_FUNCTION);
        createNewValue("<false>", FICTITIOUS_FUNCTION);
        createNewValue("<undefined>", FICTITIOUS_FUNCTION);
        requirers = nullptr;
        producers = nullptr;
        numGoalsInPlateau = 1;
    }

    SASTask::~SASTask()
    {
        if(requirers != nullptr)
        {
            for(unsigned int i = 0; i < variables.size(); i++)
            {
                delete[] requirers[i];
            }
            delete[] requirers;
        }
        if(producers != nullptr)
        {
            for(unsigned int i = 0; i < variables.size(); i++)
            {
                delete[] producers[i];
            }
            delete[] producers;
        }
        delete[] initialState;
        delete[] numInitialState;
        if(staticNumFunctions != nullptr)
        { delete[] staticNumFunctions; }
    }

// Adds a mutex relationship between (var1, value1) and (var2, value2)
    void SASTask::addMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2)
    {
        mutex[getMutexCode(var1, value1, var2, value2)] = true;
        mutex[getMutexCode(var2, value2, var1, value1)] = true;
        //cout << "Mutex added: " << variables[var1].name << "=" << values[value1].name << " and " <<
        //	variables[var2].name << "=" << values[value2].name << endl;
    }

// Checks if (var1, value1) and (var2, value2) are mutex
    bool SASTask::isMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2)
    {
        return mutex.find(getMutexCode(var1, value1, var2, value2)) != mutex.end();
    }

    bool SASTask::isPermanentMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2)
    {
        return permanentMutex.find(getMutexCode(var1, value1, var2, value2)) != mutex.end();
    }

    bool SASTask::isPermanentMutex(SASAction* a1, SASAction* a2)
    {
        uint64_t n = a1->index;
        n = (n << 32) + a2->index;
        return permanentMutexActions.find(n) != permanentMutexActions.end();
    }

// Adds a new variable
    SASVariable* SASTask::createNewVariable()
    {
        variables.emplace_back();
        SASVariable* v = &(variables.back());
        v->index = variables.size() - 1;
        v->name = "var" + std::to_string(v->index);
        return v;
    }

// Adds a new variable
    SASVariable* SASTask::createNewVariable(std::string name)
    {
        variables.emplace_back();
        SASVariable* v = &(variables.back());
        v->index = variables.size() - 1;
        v->name = name;
        return v;
    }

// Adds a new value
    unsigned int SASTask::createNewValue(std::string name, unsigned int fncIndex)
    {
        //if (fncIndex == FICTITIOUS_FUNCTION) cout << "NEW VALUE: " << name << endl;
        std::unordered_map<std::string, unsigned int>::const_iterator it = valuesByName.find(name);
        if(it != valuesByName.end())
        { return it->second; }
        values.emplace_back();
        SASValue* v = &(values.back());
        v->index = values.size() - 1;
        v->fncIndex = fncIndex;
        v->name = name;
        valuesByName[v->name] = v->index;
        return v->index;
    }

// Adds a new value if it does not exist yet
    unsigned int SASTask::findOrCreateNewValue(std::string name, unsigned int fncIndex)
    {
        std::unordered_map<std::string, unsigned int>::const_iterator value = valuesByName.find(name);
        if(value == valuesByName.end())
        { return createNewValue(name, fncIndex); }
        else
        { return value->second; }
    }

// Return the index of a value through its name
    unsigned int SASTask::getValueByName(const std::string& name)
    {
        return valuesByName[name];
    }

// Adds a new numeric variable
    NumericVariable* SASTask::createNewNumericVariable(std::string name)
    {
        numVariables.emplace_back();
        NumericVariable* v = &(numVariables.back());
        v->index = numVariables.size() - 1;
        v->name = name;
        return v;
    }

// Adds a new action
    SASAction* SASTask::createNewAction(std::string name)
    {
        actions.emplace_back();
        SASAction* a = &(actions.back());
        a->index = actions.size() - 1;
        a->name = name;
        a->isGoal = false;
        return a;
    }

// Adds a new goal
    SASAction* SASTask::createNewGoal()
    {
        goals.emplace_back();
        SASAction* a = &(goals.back());
        a->index = goals.size() - 1;
        a->name = "<goal>";
        a->isGoal = true;
        return a;
    }

// Computes the initial state
    void SASTask::computeInitialState()
    {
        initialState = new TValue[variables.size()];
        for(unsigned int i = 0; i < variables.size(); i++)
        {
            initialState[i] = variables[i].getInitialStateValue();
        }
        numInitialState = new float[numVariables.size()];
        for(unsigned int i = 0; i < numVariables.size(); i++)
        {
            numInitialState[i] = numVariables[i].getInitialStateValue();
        }
    }

// Computes the list of actions that requires a (= var value)
    void SASTask::computeRequirers()
    {
        requirers = new std::vector < SASAction * > *[variables.size()];
        for(unsigned int i = 0; i < variables.size(); i++)
        {
            requirers[i] = new std::vector<SASAction*>[values.size()];
        }
        unsigned int numActions = actions.size();
        for(unsigned int i = 0; i < numActions; i++)
        {
            SASAction* a = &(actions[i]);
            for(unsigned int j = 0; j < a->startCond.size(); j++)
            {
                addToRequirers(a->startCond[j].var, a->startCond[j].value, a);
            }
            for(unsigned int j = 0; j < a->overCond.size(); j++)
            {
                addToRequirers(a->overCond[j].var, a->overCond[j].value, a);
            }
            for(unsigned int j = 0; j < a->endCond.size(); j++)
            {
                addToRequirers(a->endCond[j].var, a->endCond[j].value, a);
            }
            if(a->startCond.empty() && a->overCond.empty() && a->endCond.empty())
            {
                actionsWithoutConditions.push_back(a);
            }
        }
    }

// Adds a requirer, checking for no duplicates
    void SASTask::addToRequirers(TVariable v, TValue val, SASAction* a)
    {
        std::vector<SASAction*>& req = requirers[v][val];
        for(unsigned int i = 0; i < req.size(); i++)
        {
            if(req[i] == a)
            { return; }
        }
        req.push_back(a);
    }

// Computes the list of actions that produces (= var value)
    void SASTask::computeProducers()
    {
        producers = new std::vector < SASAction * > *[variables.size()];
        for(unsigned int i = 0; i < variables.size(); i++)
        {
            producers[i] = new std::vector<SASAction*>[values.size()];
        }
        unsigned int numActions = actions.size();
        for(unsigned int i = 0; i < numActions; i++)
        {
            SASAction* a = &(actions[i]);
            for(unsigned int j = 0; j < a->startEff.size(); j++)
            {
                addToProducers(a->startEff[j].var, a->startEff[j].value, a);
            }
            for(unsigned int j = 0; j < a->endEff.size(); j++)
            {
                addToProducers(a->endEff[j].var, a->endEff[j].value, a);
            }
        }
    }

    void SASTask::computeMutexWithVarValues()
    {
        uint32_t vv1, vv2;
        std::unordered_map<uint64_t, bool>::const_iterator got = mutex.begin();
        std::unordered_map<uint32_t, std::vector<uint32_t>*>::const_iterator it;
        for(; got != mutex.end(); ++got)
        {
            uint64_t n = got->first;
            vv2 = n & 0xFFFFFFFF;
            vv1 = (uint32_t)(n >> 32);
            it = mutexWithVarValue.find(vv1);
            if(it == mutexWithVarValue.end())
            {
                std::vector<uint32_t>* item = new std::vector<uint32_t>();
                item->push_back(vv2);
                mutexWithVarValue[vv1] = item;
            }
            else
            {
                it->second->push_back(vv2);
            }
        }
    }

    void SASTask::checkEffectReached(SASCondition* c,
                                     std::unordered_map<TVarValue, bool>* goals,
                                     std::unordered_map<TVarValue, bool>* visitedVarValue,
                                     std::vector<TVarValue>* state)
    {
        TVarValue code = getVariableValueCode(c->var, c->value);
        goals->erase(code);
        if(visitedVarValue->find(code) == visitedVarValue->end())
        {
            (*visitedVarValue)[code] = true;
            state->push_back(code);
        }
    }

    void SASTask::checkReachability(TVarValue vv, std::unordered_map<TVarValue, bool>* goals)
    {
        /*
        cout << "Checking if " << variables[getVariableIndex(vv)].name << "=" <<
                values[getValueIndex(vv)].name << " is permanent mutex with:" << endl;
        for (std::unordered_map<TVarValue,bool>::const_iterator itg = goals->begin(); itg != goals->end(); ++itg)
            cout << " " << variables[getVariableIndex(itg->first)].name << "=" << values[getValueIndex(itg->first)].name << endl;
        */
        unsigned int numActions = actions.size();
        bool visited[numActions];
        for(unsigned int i = 0; i < numActions; i++)
        { visited[i] = false; }
        std::vector<TVarValue> state;
        std::unordered_map<TVarValue, bool> visitedVarValue;
        state.push_back(vv);
        visitedVarValue[vv] = true;
        unsigned int start = 0;
        while(start < state.size() && !goals->empty())
        {
            TVariable v = getVariableIndex(state[start]);
            TValue value = getValueIndex(state[start]);
            start++;
            std::vector<SASAction*>& req = requirers[v][value];
            //cout << variables[v].name << "=" << values[value].name << endl;
            for(unsigned int i = 0; i < req.size(); i++)
            {
                SASAction* a = req[i];
                if(!visited[a->index])
                {
                    visited[a->index] = true;
                    //cout << " - " << a->name << endl;
                    for(unsigned int j = 0; j < a->startEff.size(); j++)
                    {
                        checkEffectReached(&(a->startEff[j]), goals, &visitedVarValue, &state);
                    }
                    for(unsigned int j = 0; j < a->endEff.size(); j++)
                    {
                        checkEffectReached(&(a->endEff[j]), goals, &visitedVarValue, &state);
                    }
                }
            }
        }
    }

    void SASTask::computePermanentMutex()
    {
        //clock_t tini = clock();
        computeMutexWithVarValues();
        std::unordered_map<uint32_t, std::vector<uint32_t>*>::const_iterator it;
        std::unordered_map<uint32_t, bool>::const_iterator ug;
        for(it = mutexWithVarValue.begin(); it != mutexWithVarValue.end(); ++it)
        {
            //cout << it->second->size() << endl;
            std::unordered_map<uint32_t, bool> goals;
            for(unsigned int i = 0; i < it->second->size(); i++)
            {
                goals[it->second->at(i)] = true;
            }
            checkReachability(it->first, &goals);
            for(ug = goals.begin(); ug != goals.end(); ++ug)
            {
                TMutex code = (((TMutex)it->first) << 32) + ug->first;
                permanentMutex[code] = true;
            }
        }
        if(permanentMutex.size() > 0)
        {
            unsigned int numActions = actions.size();
            for(unsigned int i = 0; i < numActions - 1; i++)
            {
                SASAction* a1 = &(actions[i]);
                for(unsigned int j = i + 1; j < numActions; j++)
                {
                    if(checkActionMutex(a1, &(actions[j])))
                    {
                        //cout << a1->name << " <- mutex -> " << actions[j].name << endl;
                        uint64_t n = a1->index;
                        n = (n << 32) + actions[j].index;
                        permanentMutexActions[n] = true;
                        n = actions[j].index;
                        n = (n << 32) + a1->index;
                        permanentMutexActions[n] = true;
                    }
                }
            }
        }
        //cout << (float) (((int) (1000 * (clock() - tini)/(float) CLOCKS_PER_SEC))/1000.0) << " sec." << endl;
    }

    bool SASTask::checkActionMutex(SASAction* a1, SASAction* a2)
    {
        return checkActionOrdering(a1, a2) && checkActionOrdering(a2, a1);
    }

    bool SASTask::checkActionOrdering(SASAction* a1, SASAction* a2)
    {
        for(unsigned int i = 0; i < a1->startEff.size(); i++)
        {
            TVariable v1 = a1->startEff[i].var;
            TValue value1 = a1->startEff[i].value;
            for(unsigned int j = 0; j < a2->startCond.size(); j++)
            {
                TVariable v2 = a2->startCond[j].var;
                TValue value2 = a2->startCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
            for(unsigned int j = 0; j < a2->overCond.size(); j++)
            {
                TVariable v2 = a2->overCond[j].var;
                TValue value2 = a2->overCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
            for(unsigned int j = 0; j < a2->endCond.size(); j++)
            {
                TVariable v2 = a2->endCond[j].var;
                TValue value2 = a2->endCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
        }
        for(unsigned int i = 0; i < a1->endEff.size(); i++)
        {
            TVariable v1 = a1->endEff[i].var;
            TValue value1 = a1->endEff[i].value;
            for(unsigned int j = 0; j < a2->startCond.size(); j++)
            {
                TVariable v2 = a2->startCond[j].var;
                TValue value2 = a2->startCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
            for(unsigned int j = 0; j < a2->overCond.size(); j++)
            {
                TVariable v2 = a2->overCond[j].var;
                TValue value2 = a2->overCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
            for(unsigned int j = 0; j < a2->endCond.size(); j++)
            {
                TVariable v2 = a2->endCond[j].var;
                TValue value2 = a2->endCond[j].value;
                if(isPermanentMutex(v1, value1, v2, value2))
                { return true; }
            }
        }
        return false;
    }

// Adds a producer, checking for no duplicates
    void SASTask::addToProducers(TVariable v, TValue val, SASAction* a)
    {
        std::vector<SASAction*>& prod = producers[v][val];
        for(unsigned int i = 0; i < prod.size(); i++)
        {
            if(prod[i] == a)
            { return; }
        }
        prod.push_back(a);
    }

// Computes the cost of the actions according to the metric (if possible)
// The cost of an action cannot be computed in the following cases:
// * The duration of the action depends on the state (numeric value of one or more variables) and the metric depends on the plan duration.
// * A numeric variable (e.g. fuel-used) is modified through the action, and this change of value depends on the value of another numeric
//   variable in the state (e.g. temperature), and the numeric variable (fuel-used) is used in the metric.
    void SASTask::computeInitialActionsCost(bool keepStaticData)
    {
        if(keepStaticData)
        {    // Static functions have not been deleted, so it is necessary to check which ones are static
            staticNumFunctions = new bool[numVariables.size()];
            for(unsigned int i = 0; i < numVariables.size(); i++)
            {
                staticNumFunctions[i] = true;
            }
            for(unsigned int i = 0; i < actions.size(); i++)
            {
                SASAction& a = actions[i];
                for(unsigned int j = 0; i < a.startNumEff.size(); i++)
                {
                    staticNumFunctions[a.startNumEff[j].var] = false;
                }
                for(unsigned int j = 0; i < a.endNumEff.size(); i++)
                {
                    staticNumFunctions[a.endNumEff[j].var] = false;
                }
            }
        }
        else
        {
            staticNumFunctions = nullptr;
        }
        bool* variablesOnMetric = new bool[numVariables.size()];
        for(unsigned int i = 0; i < numVariables.size(); i++)
        {
            variablesOnMetric[i] = false;
        }
        variableCosts = false;
        metricDependsOnDuration = checkVariablesUsedInMetric(&metric, variablesOnMetric);
        for(unsigned int i = 0; i < actions.size(); i++)
        {
            computeActionCost(&actions[i], variablesOnMetric);
            if(!actions[i].fixedCost)
            { variableCosts = true; }
        }
        for(unsigned int i = 0; i < goals.size(); i++)
        {
            goals[i].setGoalCost();
        }
        delete[] variablesOnMetric;
    }

// Check the numeric variables used in the metric function
// Return true if the metric depends on the plan duration
    bool SASTask::checkVariablesUsedInMetric(SASMetric* m, bool* variablesOnMetric)
    {
        bool metricDependsOnDuration = false;
        switch(m->type)
        {
            case '+':
            case '-':
            case '*':
            case '/':
                for(unsigned int i = 0; i < m->terms.size(); i++)
                {
                    if(checkVariablesUsedInMetric(&(m->terms[i]), variablesOnMetric))
                    {
                        metricDependsOnDuration = true;
                    }
                }
                break;
            case 'T':
                metricDependsOnDuration = true;
                break;
            case 'F':
                variablesOnMetric[m->index] = true;
                break;
            default:
                break;
        }
        return metricDependsOnDuration;
    }

// Computes the cost of the action according to the metric (if possible)
// The cost of an action cannot be computed in the following cases:
// * The duration of the action depends on the state (numeric value of one or more variables) and the metric depends on the plan duration.
// * A numeric variable (e.g. fuel-used) is modified through the action, and this change of value depends on the value of another numeric
//   variable in the state (e.g. temperature), and the numeric variable (fuel-used) is used in the metric.
    void SASTask::computeActionCost(SASAction* a, bool* variablesOnMetric)
    {
        a->fixedDuration = true;
        for(unsigned int i = 0; i < a->duration.size(); i++)
        {
            if(checkVariableExpression(&(a->duration[i].exp), nullptr))
            {
                a->fixedDuration = false;
                break;
            }
        }
        if(a->fixedDuration)
        {
            for(unsigned int i = 0; i < a->duration.size(); i++)
            {
                a->fixedDurationValue.push_back(computeFixedExpression(&(a->duration[i].exp)));
                //cout << "Duration of " << a->name << " is " << a->fixedDurationValue[i] << endl;
            }
        }
        a->fixedCost = a->fixedDuration || !metricDependsOnDuration;
        if(a->fixedCost)
        {
            for(unsigned int i = 0; i < a->startNumEff.size(); i++)
            {
                if(checkVariableExpression(&(a->startNumEff[i].exp), variablesOnMetric))
                {
                    a->fixedCost = false;
                    break;
                }
            }
            if(a->fixedCost)
            {
                for(unsigned int i = 0; i < a->endNumEff.size(); i++)
                {
                    if(checkVariableExpression(&(a->endNumEff[i].exp), variablesOnMetric))
                    {
                        a->fixedCost = false;
                        break;
                    }
                }
                if(a->fixedCost)
                {
                    a->fixedCostValue = computeActionCost(a, numInitialState, 0);
                    //cout << " * Fixed cost is " << a->fixedCostValue << endl;
                }
            }
        }
    }

// Computes the cost of aplying an action in a given state
    float SASTask::computeActionCost(SASAction* a, float* numState, float makespan)
    {
        float startMetricValue = evaluateMetric(&metric,
                                                numState,
                                                makespan), endMetricValue, dur = a->fixedDurationValue[0];
        if(a->startNumEff.empty() && a->endNumEff.empty())
        {
            endMetricValue = evaluateMetric(&metric, numState, makespan + dur);
        }
        else
        {
            unsigned int numV = numVariables.size();
            float* newState = new float[numV];
            for(unsigned int i = 0; i < numV; i++)
            {
                newState[i] = numState[i];
            }
            for(unsigned int i = 0; i < a->startNumEff.size(); i++)
            {
                updateNumericState(newState, &(a->startNumEff[i]), dur);
            }
            for(unsigned int i = 0; i < a->endNumEff.size(); i++)
            {
                updateNumericState(newState, &(a->endNumEff[i]), dur);
            }
            endMetricValue = evaluateMetric(&metric, newState, makespan + dur);
            delete[] newState;
        }
        return endMetricValue - startMetricValue;
    }

// Updates the numeric state through the given action effect
    void SASTask::updateNumericState(float* s, SASNumericEffect* e, float duration)
    {
        float v = evaluateNumericExpression(&(e->exp), s, duration);
        switch(e->op)
        {
            case '=':
                s[e->var] = v;
                break;
            case '+':
                s[e->var] += v;
                break;
            case '-':
                s[e->var] -= v;
                break;
            case '*':
                s[e->var] *= v;
                break;
            case '/':
                s[e->var] /= v;
                break;
            default:;
        }
    }

// Computes the duration of an action
    float SASTask::getActionDuration(SASAction* a, float* s)
    {
        if(a->duration.size() > 1)
        {
            std::cout << "Duration intervals not supported yet" << std::endl;
            assert(false);
            return EPSILON;
        }
        else
        {
            SASDuration* duration = &(a->duration[0]);
            if(duration->time == 'E' || duration->time == 'A')
            {
                std::cout << "At-end or over-all durations not supported yet" << std::endl;
                assert(false);
                return EPSILON;
            }
            else if(duration->comp != '=')
            {
                std::cout << "Inequalities in duration not supported yet" << std::endl;
                assert(false);
                return EPSILON;
            }
            else if(a->fixedDuration)
            {
                return a->fixedDurationValue[0];
            }
            else
            {
                return evaluateNumericExpression(&(duration->exp), s, 0);
            }
        }
    }

// Checks if a numeric condition holds in the given numeric state
    bool SASTask::holdsNumericCondition(SASNumericCondition& cond, float* s, float duration)
    {
        float v1 = evaluateNumericExpression(&(cond.terms[0]), s, duration);
        float v2 = evaluateNumericExpression(&(cond.terms[1]), s, duration);
        //cout << "Condition: " << v1 << " " << cond.comp << " " << v2 << endl;
        switch(cond.comp)
        {
            case '=':
                return v1 == v2;
            case '<':
                return v1 < v2;
            case 'L':
                return v1 <= v2;
            case '>':
                return v1 > v2;
            case 'G':
                return v1 >= v2;
            case 'N':
                return v1 != v2;
        }
        return false;
    }

// Evaluates a numeric expression in a given state and with the given action duration
    float SASTask::evaluateNumericExpression(SASNumericExpression* e, float* s, float duration)
    {
        if(e->type == 'N')
        { return e->value; }            // NUMBER
        if(e->type == 'V') return s[e->var];            // VAR
        if(e->type == 'D') return duration;
        if(e->type == '#')
        {
            std::cout << "#t in duration not supported yet" << std::endl;
            assert(false);
        }
        float res = evaluateNumericExpression(&(e->terms[0]), s, duration);
        for(unsigned int i = 1; i < e->terms.size(); i++)
        {
            switch(e->type)
            {
                case '+':
                    res += evaluateNumericExpression(&(e->terms[i]), s, duration);
                    break;    // SUM
                case '-':
                    res -= evaluateNumericExpression(&(e->terms[i]), s, duration);
                    break;    // SUB
                case '*':
                    res *= evaluateNumericExpression(&(e->terms[i]), s, duration);
                    break;    // MUL
                case '/':
                    res /= evaluateNumericExpression(&(e->terms[i]), s, duration);
                    break;    // DIV
            }
        }
        return res;
    }

// Calculates the metric cost in the given state and with the given plan duration
    float SASTask::evaluateMetric(SASMetric* m, float* numState, float makespan)
    {
        switch(m->type)
        {
            case 'N':
                return m->value;
            case 'T':
                return makespan;
            case 'F':
                return numState[m->index];
            case 'V':
                return 0;
            case '+':
            case '-':
            case '*':
            case '/':
            {
                float v = evaluateMetric(&(m->terms[0]), numState, makespan);
                if(m->terms.size() == 1)
                {
                    return m->type == '-' ? -v : v;
                }
                else
                {
                    for(unsigned int i = 1; i < m->terms.size(); i++)
                    {
                        float t = evaluateMetric(&(m->terms[i]), numState, makespan);
                        switch(m->type)
                        {
                            case '+':
                                v += t;
                                break;
                            case '-':
                                v -= t;
                                break;
                            case '*':
                                v *= t;
                                break;
                            case '/':
                                v /= t;
                                break;
                            default:;
                        }
                    }
                    return v;
                }
            }
                break;
            default:
                std::cout << "Metric type not supported yet. Method SASTask::evaluateMetric." << std::endl;
                exit(1);
        }
    }

// Computes the value of a non-variable expression (that doesn't depend on the value of numeric variables)
    float SASTask::computeFixedExpression(SASNumericExpression* e)
    {
        float res = 0;
        switch(e->type)
        {
            case 'N':
                res = e->value;
                break;
            case '+':
            case '-':
            case '/':
            case '*':
                res = computeFixedExpression(&(e->terms[0]));
                if(e->terms.size() == 1)
                {
                    if(e->type == '-')
                    { res = -res; }
                }
                else
                {
                    for(unsigned int i = 1; i < e->terms.size(); i++)
                    {
                        float t = computeFixedExpression(&(e->terms[i]));
                        switch(e->type)
                        {
                            case '+':
                                res += t;
                                break;
                            case '-':
                                res -= t;
                                break;
                            case '/':
                                res /= t;
                                break;
                            case '*':
                                res *= t;
                                break;
                            default:;
                        }
                    }
                }
                break;
            case 'V':
                res = numVariables[e->var].getInitialStateValue();
                break;
            default:;
        }
        return res;
    }

// Checks if the given expression depends on the state
    bool SASTask::checkVariableExpression(SASNumericExpression* e, bool* variablesOnMetric)
    {
        switch(e->type)
        {
            case 'V':
                if(staticNumFunctions != nullptr && staticNumFunctions[e->var])
                {
                    return false;                        // Variable is static -> does not depend on the state
                }
                if(variablesOnMetric == nullptr)
                {        // Not necessary to check if the variable is used in the metric
                    return true;
                }
                else
                {
                    return variablesOnMetric[e->var];    // Only if the variable is used in the metric
                }
                break;
            case '+':
            case '-':
            case '/':
            case '*':
                for(unsigned int i = 0; i < e->terms.size(); i++)
                {
                    if(checkVariableExpression(&(e->terms[i]), variablesOnMetric))
                    {
                        return true;
                    }
                }
                return false;
            default:
                return false;
        }
    }

// Returns a std::string representation of this task
    std::string SASTask::toString()
    {
        std::string s = "OBJECTS:\n";
        for(unsigned int i = 0; i < values.size(); i++)
        {
            s += values[i].toString() + "\n";
        }
        s += "VARIABLES:\n";
        for(unsigned int i = 0; i < variables.size(); i++)
        {
            s += variables[i].toString(values) + "\n";
        }
        for(unsigned int i = 0; i < numVariables.size(); i++)
        {
            s += numVariables[i].toString() + "\n";
        }
        s += "INITIAL STATE:\n";
        for(unsigned int i = 0; i < variables.size(); i++)
        {
            s += variables[i].toStringInitialState(values) + "\n";
        }
        for(unsigned int i = 0; i < numVariables.size(); i++)
        {
            s += numVariables[i].toStringInitialState() + "\n";
        }
        for(unsigned int i = 0; i < actions.size(); i++)
        {
            s += toStringAction(actions[i]);
        }
        for(unsigned int i = 0; i < goals.size(); i++)
        {
            s += toStringAction(goals[i]);
        }
        s += "CONSTRAINTS:\n";
        for(unsigned int i = 0; i < constraints.size(); i++)
        {
            s += toStringConstraint(&constraints[i]) + "\n";
        }
        if(metricType != 'X')
        {
            s += "METRIC:\n";
            if(metricType == '<')
            { s += "MINIMIZE "; }
            else
            { s += "MAXIMIZE "; }
            s += toStringMetric(&metric);
        }
        return s;
    }

// Returns a std::string representation of this action
    std::string SASTask::toStringAction(SASAction& a)
    {
        std::string s = "ACTION " + a.name + "\n";
        for(unsigned int i = 0; i < a.duration.size(); i++)
        {
            s += " :duration " + a.duration[i].toString(&numVariables) + "\n";
        }
        for(unsigned int i = 0; i < a.startNumCond.size(); i++)
        {
            s += " :con (at start " + a.startNumCond[i].toString(&numVariables) + ")\n";
        }
        for(unsigned int i = 0; i < a.startCond.size(); i++)
        {
            s += " :con (at start " + toStringCondition(a.startCond[i]) + ")\n";
        }
        for(unsigned int i = 0; i < a.overCond.size(); i++)
        {
            s += " :con (over all " + toStringCondition(a.overCond[i]) + ")\n";
        }
        for(unsigned int i = 0; i < a.overNumCond.size(); i++)
        {
            s += " :con (over all " + a.overNumCond[i].toString(&numVariables) + ")\n";
        }
        for(unsigned int i = 0; i < a.endCond.size(); i++)
        {
            s += " :con (at end " + toStringCondition(a.endCond[i]) + ")\n";
        }
        for(unsigned int i = 0; i < a.endNumCond.size(); i++)
        {
            s += " :con (at end " + a.endNumCond[i].toString(&numVariables) + ")\n";
        }
        for(unsigned int i = 0; i < a.preferences.size(); i++)
        {
            s += " :con (" + toStringPreference(&(a.preferences[i])) + ")\n";
        }
        for(unsigned int i = 0; i < a.startEff.size(); i++)
        {
            s += " :eff (at start " + toStringCondition(a.startEff[i]) + ")\n";
        }
        for(unsigned int i = 0; i < a.startNumEff.size(); i++)
        {
            s += " :eff (at start " + a.startNumEff[i].toString(&numVariables) + ")\n";
        }
        for(unsigned int i = 0; i < a.endEff.size(); i++)
        {
            s += " :eff (at end " + toStringCondition(a.endEff[i]) + ")\n";
        }
        for(unsigned int i = 0; i < a.endNumEff.size(); i++)
        {
            s += " :eff (at end " + a.endNumEff[i].toString(&numVariables) + ")\n";
        }
        return s;
    }

// Returns a std::string representation of this condition
    std::string SASTask::toStringCondition(SASCondition& c)
    {
        return "(= " + variables[c.var].name + " " + values[c.value].name + ")";
    }

// Returns a std::string representation of this preference
    std::string SASTask::toStringPreference(SASPreference* pref)
    {
        return "preference " + preferenceNames[pref->index] + " " + toStringGoalDescription(&(pref->preference));
    }

// Returns a std::string representation of this goal description
    std::string SASTask::toStringGoalDescription(SASGoalDescription* g)
    {
        std::string s = "(" + toStringTime(g->time);
        if(g->time != 'N')
        { s += " ("; }
        switch(g->type)
        {
            case 'V':
                s += "= " + variables[g->var].name + " " + values[g->value].name;
                break;
            case '&':
            case '|':
            case '!':
                if(g->type == '&')
                { s += "and"; }
                else if(g->type == '|')
                { s += "or"; }
                else
                { s += "not"; }
                for(unsigned int i = 0; i < g->terms.size(); i++)
                {
                    s += " " + toStringGoalDescription(&(g->terms[i]));
                }
                break;
            default:
                s += toStringComparator(g->type);
                for(unsigned int i = 0; i < g->exp.size(); i++)
                {
                    s += " " + g->exp[i].toString(&numVariables);
                }
        }
        if(g->time != 'N')
        { s += ")"; }
        return s + ")";
    }

// Returns a std::string representation of this constraint
    std::string SASTask::toStringConstraint(SASConstraint* c)
    {
        std::string s = "(";
        switch(c->type)
        {
            case '&':
                s += "and";
                for(unsigned int i = 0; i < c->terms.size(); i++)
                {
                    s += " " + toStringConstraint(&(c->terms[i]));
                }
                break;
            case 'P':
                s += "preference " + preferenceNames[c->preferenceIndex] + " " + toStringConstraint(&(c->terms[0]));
                break;
            case 'G':
                s += "preference " + preferenceNames[c->preferenceIndex] + " " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'E':
                s += "at end " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'A':
                s += "always " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'S':
                s += "sometime " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'W':
                s += "within " + std::to_string(c->time[0]) + " " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'O':
                s += "at-most-once " + toStringGoalDescription(&(c->goal[0]));
                break;
            case 'F':
                s += "sometime-after " + toStringGoalDescription(&(c->goal[0])) + " " +
                     toStringGoalDescription(&(c->goal[1]));
                break;
            case 'B':
                s += "sometime-before " + toStringGoalDescription(&(c->goal[0])) + " " +
                     toStringGoalDescription(&(c->goal[1]));
                break;
            case 'T':
                s += "always-within " + std::to_string(c->time[0]) + " " + toStringGoalDescription(&(c->goal[0])) + " " +
                     toStringGoalDescription(&(c->goal[1]));
                break;
            case 'D':
                s += "hold-during " + std::to_string(c->time[0]) + " " + std::to_string(c->time[1]) + " " +
                     toStringGoalDescription(&(c->goal[0]));
                break;
            case 'H':
                s += "hold-after " + std::to_string(c->time[0]) + " " + toStringGoalDescription(&(c->goal[0]));
                break;
        }
        return s + ")";
    }

// Returns a std::string representation of the metric
    std::string SASTask::toStringMetric(SASMetric* m)
    {
        std::string s = "";
        switch(m->type)
        {
            case '+':
            case '-':
            case '*':
            case '/':
                s += "(";
                s += m->type;
                for(unsigned int i = 0; i < m->terms.size(); i++)
                {
                    s += " " + toStringMetric(&(m->terms[i]));
                }
                s += ")";
                break;
            case 'N':
                s += std::to_string(m->value);
                break;
            case 'T':
                s += "total-time";
                break;
            case 'V':
                s += "(is-violated " + preferenceNames[m->index] + ")";
                break;
            case 'F':
                s += numVariables[m->index].name;
                break;
        }
        return s;
    }

    std::vector<TVarValue>* SASTask::getListOfGoals()
    {
        if(goalList.empty())
        {
            for(unsigned int i = 0; i < goals.size(); i++)
            {
                SASAction& g = goals[i];
                for(unsigned int j = 0; j < g.startCond.size(); j++)
                {
                    addGoalToList(&(g.startCond[j]));
                }
                for(unsigned int j = 0; j < g.overCond.size(); j++)
                {
                    addGoalToList(&(g.overCond[j]));
                }
                for(unsigned int j = 0; j < g.endCond.size(); j++)
                {
                    addGoalToList(&(g.endCond[j]));
                }
            }
        }
        return &goalList;
    }

    void SASTask::addGoalToList(SASCondition* c)
    {
        TVarValue vv = getVariableValueCode(c->var, c->value);
        for(unsigned int i = 0; i < goalList.size(); i++)
        {
            if(goalList[i] == vv)
            { return; }
        }
        goalList.push_back(vv);
    }

    void SASTask::addGoalDeadline(float time, TVarValue goal)
    {
        for(unsigned int i = 0; i < goalDeadlines.size(); i++)
        {
            if(goalDeadlines[i].time == time)
            {
                goalDeadlines[i].goals.push_back(goal);
            }
        }
        goalDeadlines.emplace_back();
        goalDeadlines.back().time = time;
        goalDeadlines.back().goals.push_back(goal);
    }

}
