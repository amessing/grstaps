/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* October 2015                                         */
/********************************************************/
/* Grounds all possible actions from the                */
/* PreprocessedTask.                                    */
/********************************************************/

#include "grstaps/task_planning/grounder.hpp"
#include <assert.h>
#include <iostream>

#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    //#define _GROUNDER_TRACE_ON_

    /********************************************************/
    /* CLASS: GrounderAssignment                            */
    /********************************************************/

    GrounderAssignment::GrounderAssignment(OpFluent &f)
    {
        fncIndex = f.variable.fncIndex;
        params   = &(f.variable.params);
        grounded = false;
        value    = &(f.value);
    }

    /********************************************************/
    /* CLASS: GrounderOperator                              */
    /********************************************************/

    void GrounderOperator::initialize(Operator &o)
    {
        op                         = &o;
        numParams                  = o.parameters.size();
        paramValues                = new std::vector<unsigned int>[numParams];
        compatibleObjectsWithParam = new std::vector<unsigned int>[numParams];
        for(unsigned int i = 0; i < o.atStart.prec.size(); i++)
            preconditions.emplace_back(o.atStart.prec[i]);
        for(unsigned int i = 0; i < o.overAllPrec.size(); i++)
            preconditions.emplace_back(o.overAllPrec[i]);
    }

    GrounderOperator::~GrounderOperator()
    {
        delete[] paramValues;
        delete[] compatibleObjectsWithParam;
    }

    /********************************************************/
    /* CLASS: ProgrammedValue                              */
    /********************************************************/

    ProgrammedValue::ProgrammedValue(unsigned int index, unsigned int varIndex, unsigned int valueIndex)
    {
        this->index      = index;
        this->varIndex   = varIndex;
        this->valueIndex = valueIndex;
    }

    /********************************************************/
    /* CLASS: Grounder                                      */
    /********************************************************/

    // Grounding process
    GroundedTask *Grounder::groundTask(PreprocessedTask *prepTask, bool keepStaticData)
    {
        currentLevel   = 0;
        this->prepTask = prepTask;
        gTask          = new GroundedTask(prepTask->task);
        initTypesMatrix();
        initOperators();
        initInitialState();
        for(unsigned int i = 0; i < numOps; i++)
        {
            Operator *op = ops[i].op;
            if(op->atStart.prec.size() == 0 && op->overAllPrec.size() == 0)
            {
                groundRemainingParameters(ops[i]);
            }
        }
        // Program the facts in the initial state
        for(unsigned int i = 0; i < auxValues->size(); i++)
        {
            ProgrammedValue &pv = auxValues->at(i);
            newValues->push_back(pv);
            valuesByFunction[gTask->variables[pv.varIndex].fncIndex].push_back(pv);
        }
        auxValues->clear();
        while(newValues->size() > 0)
        {
            for(unsigned int i = 0; i < newValues->size(); i++)
            {
                match(newValues->at(i));
            }
            startNewValues += newValues->size();
            swapLevels();
            currentLevel++;
        }
        removeADLFeaturesInPreferences();
        removeADLFeaturesInConstraints();
        if(gTask->task->metricType == MT_NONE)
        {
            gTask->metricType = 'X';
        }
        else
        {
            gTask->metricType = gTask->task->metricType == MT_MAXIMIZE ? '>' : '<';
            gTask->metric     = groundMetric(&(gTask->task->metric));
        }
        if(!keepStaticData)
        {
            removeStaticVariables();
        }
        checkNumericConditions();
        computeInitialVariableValues();
        clearMemory();
        return gTask;
    }

    // Creates the matrix of types for a fast checking of types compatibility
    void Grounder::initTypesMatrix()
    {
        unsigned int numTypes = prepTask->task->types.size();
        typesMatrix           = new bool *[numTypes];
        for(unsigned int i = 0; i < numTypes; i++)
            typesMatrix[i] = new bool[numTypes]();
        for(unsigned int i = 0; i < numTypes; i++)
            addTypeToMatrix(i, i);
    }

    // Deletes the allocated memory
    void Grounder::clearMemory()
    {
        for(unsigned int i = 0; i < prepTask->task->types.size(); i++)
            delete[] typesMatrix[i];
        delete[] typesMatrix;
        delete[] opRequireFunction;
        delete[] ops;
        delete[] valuesByFunction;
        delete newValues;
        delete auxValues;
    }

    // Recursively initializes the matrix of types
    void Grounder::addTypeToMatrix(unsigned int typeIndex, unsigned int subtypeIndex)
    {
        typesMatrix[typeIndex][subtypeIndex] = true;
        Type &t                              = prepTask->task->types[subtypeIndex];
        for(unsigned int i = 0; i < t.parentTypes.size(); i++)
            addTypeToMatrix(typeIndex, t.parentTypes[i]);
    }

    // Initializes the operators for grounding
    void Grounder::initOperators()
    {
        numOps                  = prepTask->operators.size();
        ops                     = new GrounderOperator[numOps];
        unsigned int numObjects = prepTask->task->objects.size();
        for(unsigned int i = 0; i < numOps; i++)
        {
            GrounderOperator &g = ops[i];
            g.initialize(prepTask->operators[i]);
            for(unsigned int j = 0; j < g.numParams; j++)
            {
                for(unsigned int k = 0; k < numObjects; k++)
                {
                    if(objectIsCompatible(k, g.op->parameters[j].types))
                    {
                        g.compatibleObjectsWithParam[j].push_back(k);
                    }
                }
            }
        }
        unsigned int numFunctions = prepTask->task->functions.size();
        opRequireFunction         = new std::vector<GrounderOperator *>[numFunctions];
        for(unsigned int i = 0; i < numOps; i++)
        {
            std::vector<OpFluent> &atStart = ops[i].op->atStart.prec;
            for(unsigned int j = 0; j < atStart.size(); j++)
            {
                addOpToRequireFunction(&ops[i], atStart[j].variable.fncIndex);
            }
            std::vector<OpFluent> &overAll = ops[i].op->overAllPrec;
            for(unsigned int j = 0; j < overAll.size(); j++)
            {
                addOpToRequireFunction(&ops[i], overAll[j].variable.fncIndex);
            }
        }
    }

    // Annotates that an operator requires a given function as a precondition
    void Grounder::addOpToRequireFunction(GrounderOperator *op, unsigned int f)
    {
        std::vector<GrounderOperator *> &v = opRequireFunction[f];
        bool included                      = false;
        for(unsigned int i = 0; i < v.size(); ++i)
        {
            if(v[i] == op)
            {
                included = true;
                break;
            }
        }
        if(!included)
        {
            v.push_back(op);
        }
    }

    // Sets the initial state in the first level of the relaxed planning graph
    void Grounder::initInitialState()
    {
        unsigned int numFunctions  = prepTask->task->functions.size();
        newValues                  = new std::vector<ProgrammedValue>();
        auxValues                  = new std::vector<ProgrammedValue>();
        valuesByFunction           = new std::vector<ProgrammedValue>[numFunctions];
        unsigned int initStateSize = prepTask->task->init.size();
        for(unsigned int i = 0; i < initStateSize; i++)
            createVariable(prepTask->task->init[i]);
        numValues = 0;
        for(unsigned int i = 0; i < initStateSize; i++)
        {
            Fact &f = prepTask->task->init[i];
            if(!f.valueIsNumeric)
            {  // Program only non-numeric variables
                ProgrammedValue pv(numValues++, getVariableIndex(f), f.value);
                newValues->push_back(pv);
                valuesByFunction[f.function].push_back(pv);
                gTask->reachedValues[pv.varIndex][pv.valueIndex] = 0;
            }
        }
        startNewValues = 0;
    }

    // Creates a new variable
    void Grounder::createVariable(const Fact &f)
    {
        std::string factName = getVariableName(f.function, f.parameters);
        if(variableIndex.find(factName) == variableIndex.end())
        {
            // New variable
            GroundedVar v;
            v.index     = gTask->variables.size();
            v.fncIndex  = f.function;
            v.isNumeric = f.valueIsNumeric;
            v.params    = f.parameters;
            gTask->variables.push_back(v);
            variableIndex[factName] = v.index;
            unsigned int notReached = MAX_UNSIGNED_INT;
            if(v.isNumeric)
            {
                gTask->reachedValues.emplace_back(0, notReached);
            }
            else
            {
                gTask->reachedValues.emplace_back(prepTask->task->objects.size(), notReached);
                std::vector<unsigned int> values = gTask->reachedValues.back();
                values[f.value]                  = 0;
            }
        }
    }

    // Returns the description of a variable
    std::string Grounder::getVariableName(unsigned int function, const std::vector<unsigned int> &parameters)
    {
        std::string name = std::to_string(function);
        for(unsigned int i = 0; i < parameters.size(); i++)
        {
            name += " " + std::to_string(parameters[i]);
        }
        return name;
    }

    // Returns the description of a literal
    std::string Grounder::getVariableName(const Literal &l, const std::vector<unsigned int> &opParameters)
    {
        std::string name = std::to_string(l.fncIndex);
        for(unsigned int i = 0; i < l.params.size(); i++)
        {
            if(l.params[i].isVariable)
            {
                name += " " + std::to_string(opParameters[l.params[i].index]);
            }
            else
            {
                name += " " + std::to_string(l.params[i].index);
            }
        }
        return name;
    }

    // Returns the index of a variable
    unsigned int Grounder::getVariableIndex(const Fact &f)
    {
        return variableIndex[getVariableName(f.function, f.parameters)];
    }

    // Returns the index of a variable
    unsigned int Grounder::getVariableIndex(const Literal &l, const std::vector<unsigned int> &opParameters)
    {
        const std::string &name                                          = getVariableName(l, opParameters);
        std::unordered_map<std::string, unsigned int>::const_iterator it = variableIndex.find(name);
        if(it == variableIndex.end())
            return MAX_UNSIGNED_INT;
        else
            return it->second;
    }

    // Grounding by combining all possible values for the parameters
    void Grounder::groundRemainingParameters(GrounderOperator &op)
    {
        unsigned int pIndex = MAX_UNSIGNED_INT;
        for(unsigned int i = 0; i < op.numParams; i++)
        {
            if(op.paramValues[i].size() == 0)
            {
                pIndex = i;
                break;
            }
        }
        if(pIndex == MAX_UNSIGNED_INT)
        {
            groundAction(op);
        }
        else
        {
            std::vector<unsigned int> &v = op.compatibleObjectsWithParam[pIndex];
            for(unsigned int i = 0; i < v.size(); i++)
            {
                op.paramValues[pIndex].push_back(v[i]);
                groundRemainingParameters(op);
                op.paramValues[pIndex].pop_back();
            }
        }
    }

    // Grounds a new action
    void Grounder::groundAction(GrounderOperator &op)
    {
        GroundedAction a;
        a.index = gTask->actions.size();
        a.name  = op.op->name;
        for(unsigned int i = 0; i < op.numParams; i++)
        {
            // Action parameters grounding
            a.parameters.push_back(op.paramValues[i].back());
        }
        if(!op.op->isGoal)
        {
            std::string name = a.getName(gTask->task);
            if(groundedActions.find(name) != groundedActions.end())
            {
                return;  // Repeated action
            }
            groundedActions[name] = a.index;
        }
        if(!checkEqualityConditions(op, a))
            return;
        if(!groundPreconditions(op, a))
            return;
        if(!groundEffects(op, a))
            return;
        if(!groundPreferences(op, a))
            return;
        if(!groundDuration(op, a))
            return;
        for(unsigned int i = 0; i < a.startEff.size(); i++)
            programNewValue(a.startEff[i]);
        for(unsigned int i = 0; i < a.endEff.size(); i++)
            programNewValue(a.endEff[i]);
        if(op.op->isGoal)
        {
            gTask->goals.push_back(a);
        }
        else
        {
            gTask->actions.push_back(a);
        }
    }

    // Programs a new reached value
    void Grounder::programNewValue(GroundedCondition &eff)
    {
        std::vector<unsigned int> &v = gTask->reachedValues[eff.varIndex];
        if(v[eff.valueIndex] == MAX_UNSIGNED_INT)
        {  // New value
            v[eff.valueIndex] = currentLevel + 1;
            auxValues->emplace_back(numValues, eff.varIndex, eff.valueIndex);
            numValues++;
        }
    }

    // Checks if the given object is compatible with the given types
    bool Grounder::objectIsCompatible(unsigned int objIndex, std::vector<unsigned int> &types)
    {
        std::vector<unsigned int> &v = prepTask->task->objects[objIndex].types;
        for(unsigned int i = 0; i < v.size(); i++)
        {
            unsigned int t = v[i];
            for(unsigned int j = 0; j < types.size(); j++)
            {
                if(typesMatrix[t][types[j]])
                    return true;
            }
        }
        return false;
    }

    // Checks whether a programmed value matches one of the preconditions of the operators
    void Grounder::match(ProgrammedValue &pv)
    {
        std::vector<GrounderOperator *> &rf = opRequireFunction[gTask->variables[pv.varIndex].fncIndex];
        for(unsigned int i = 0; i < rf.size(); i++)
        {
            GrounderOperator *op = rf[i];
            int precIndex        = -1;
#ifdef _GROUNDER_TRACE_ON_
            if(op->op->name.compare("inspect") == 0)
                std::cout << "Grounding " << op->op->name << " with "
                          << gTask->variables[pv.varIndex].toString(gTask->task) << "="
                          << gTask->task->objects[pv.valueIndex].name << std::endl;
#endif
            do
            {
                precIndex = matches(op, pv.varIndex, pv.valueIndex, precIndex + 1);
                if(precIndex != -1)
                {  // Match found
#ifdef _GROUNDER_TRACE_ON_
                    if(op->op->name.compare("inspect") == 0)
                        std::cout << "* Match with precondition " << precIndex << " found" << std::endl;
#endif
                    op->newValueIndex = pv.index;
                    stackParameters(op, precIndex, pv.varIndex, pv.valueIndex);
                    completeMatch(op, 0);
                    unstackParameters(op, precIndex);
                }
            } while(precIndex != -1);
        }
    }

    // Exchanges the levels of programmed values (newValues <-> auxValues)
    void Grounder::swapLevels()
    {
        for(unsigned int i = 0; i < auxValues->size(); i++)
        {
            ProgrammedValue &pv = auxValues->at(i);
            valuesByFunction[gTask->variables[pv.varIndex].fncIndex].push_back(pv);
        }
        std::vector<ProgrammedValue> *aux = newValues;
        newValues                         = auxValues;
        auxValues                         = aux;
        auxValues->clear();
    }

    // Checks whether a fluent matches an operator precondition
    int Grounder::matches(GrounderOperator *op, unsigned int varIndex, unsigned int valueIndex, int startPrec)
    {
        unsigned int fncIndex = gTask->variables[varIndex].fncIndex;
        for(unsigned int i = (unsigned int)startPrec; i < op->preconditions.size(); i++)
        {
            GrounderAssignment &p = op->preconditions[i];
            if(!p.grounded && p.fncIndex == fncIndex && precMatches(op, p, varIndex, valueIndex))
            {
                return (int)i;
            }
        }
        return -1;
    }

    // Stacks the parameter values when a precondition matches with a fluent
    void Grounder::stackParameters(GrounderOperator *op, int precIndex, unsigned int varIndex, unsigned int valueIndex)
    {
        unsigned int i;
        GrounderAssignment &prec = op->preconditions[(unsigned int)precIndex];
        GroundedVar &v           = gTask->variables[varIndex];
        for(i = 0; i < v.params.size(); i++)
            if(prec.params->at(i).isVariable)
                op->paramValues[prec.params->at(i).index].push_back(v.params[i]);
        if(prec.value->isVariable)
            op->paramValues[prec.value->index].push_back(valueIndex);
        prec.grounded = true;
    }

    // Unstacks the last parameter values
    void Grounder::unstackParameters(GrounderOperator *op, int precIndex)
    {
        unsigned int index;
        GrounderAssignment &prec = op->preconditions[(unsigned int)precIndex];
        for(unsigned int i = 0; i < prec.params->size(); i++)
            if(prec.params->at(i).isVariable)
            {
                index = prec.params->at(i).index;
                op->paramValues[index].pop_back();
            }
        if(prec.value->isVariable)
        {
            index = prec.value->index;
            op->paramValues[index].pop_back();
        }
        prec.grounded = false;
    }

    // Checks whether a precondition p matches with a fluent (variable v = valueIndex)
    bool Grounder::precMatches(GrounderOperator *op,
                               GrounderAssignment &p,
                               unsigned int varIndex,
                               unsigned int valueIndex)
    {
        unsigned int paramIndex;
        GroundedVar &v = gTask->variables[varIndex];
        for(unsigned int i = 0; i < v.params.size(); i++)
        {
            // Check the parameters
            paramIndex = p.params->at(i).index;
            if(p.params->at(i).isVariable)
            {
                // Parameter
                std::vector<unsigned int> &paramValues = op->paramValues[paramIndex];
                if(paramValues.size() == 0)
                {  // Ungrounded parameter, types should match
                    if(!objectIsCompatible(v.params[i], op->op->parameters[paramIndex].types))
                    {
                        return false;
                    }
                }
                else
                {  // Grounded parameter, objects must coincide
                    if(paramValues.back() != v.params[i])
                    {
                        return false;
                    }
                }
            }
            else
            {  // Constant object
                if(paramIndex != v.params[i])
                {
                    return false;
                }
            }
        }
        paramIndex = p.value->index;  // Check the value
        if(p.value->isVariable)
        {  // Parameter
            std::vector<unsigned int> &paramValues = op->paramValues[paramIndex];
            if(paramValues.size() == 0)
            {  // Ungrounded parameter, types should match
                return objectIsCompatible(valueIndex, op->op->parameters[paramIndex].types);
            }
            else
            {  // Grounded parameter, objects must coincide
                return paramValues.back() == valueIndex;
            }
        }
        else
        {  // Constant object
            return valueIndex == paramIndex;
        }
    }

    // Completes the operator matching process
    void Grounder::completeMatch(GrounderOperator *op, unsigned int precIndex)
    {
        GrounderAssignment *p = nullptr;
        while(precIndex < op->preconditions.size())
        {
            p = &(op->preconditions[precIndex]);
            if(!p->grounded)
            {
                if(!p->value->isVariable && p->value->index == prepTask->task->CONSTANT_FALSE)
                    p->grounded = true;
                else
                {
#ifdef _GROUNDER_TRACE_ON_
                    std::cout << "Precondition " << precIndex << "("
                              << gTask->task->functions[op->preconditions[precIndex].fncIndex].name << ") ungrounded"
                              << std::endl;
#endif
                    break;
                }
            }
            precIndex++;
        }
        if(precIndex >= op->preconditions.size())
        {  // All preconditions already grounded
#ifdef _GROUNDER_TRACE_ON_
            std::cout << "All preconditions gounded" << std::endl;
#endif
            groundRemainingParameters(*op);
        }
        else
        {
// Continue with the precondition matching
#ifdef _GROUNDER_TRACE_ON_
            std::cout << "Trying to ground precondition " << precIndex << ": "
                      << gTask->task->functions[op->preconditions[precIndex].fncIndex].name << std::endl;
#endif
            std::vector<ProgrammedValue> &vf = valuesByFunction[p->fncIndex];
            for(unsigned int i = 0; i < vf.size(); i++)
            {
                ProgrammedValue &pv = vf[i];
#ifdef _GROUNDER_TRACE_ON_
                std::cout << "    Testing " << gTask->variables[pv.varIndex].toString(gTask->task) << "="
                          << gTask->task->objects[pv.valueIndex].name << std::endl;
                std::cout << "    pv.index = " << pv.index << ", startNewValues = " << startNewValues
                          << ", op->newValueIndex = " << op->newValueIndex << std::endl;
#endif
                if((pv.index < startNewValues || pv.index >= op->newValueIndex) &&
                   precMatches(op, *p, pv.varIndex, pv.valueIndex))
                {
#ifdef _GROUNDER_TRACE_ON_
                    std::cout << "    Match found with " << gTask->variables[pv.varIndex].toString(gTask->task) << "="
                              << gTask->task->objects[pv.valueIndex].name << std::endl;
#endif
                    stackParameters(op, precIndex, pv.varIndex, pv.valueIndex);
                    completeMatch(op, precIndex + 1);
                    unstackParameters(op, precIndex);
                }
            }
        }
#ifdef _GROUNDER_TRACE_ON_
        std::cout << "Finishing" << std::endl;
#endif
    }

    // Check equality conditions
    bool Grounder::checkEqualityConditions(GrounderOperator &op, GroundedAction &a)
    {
        std::vector<OpEquality> &eq = op.op->equality;
        for(unsigned int i = 0; i < eq.size(); i++)
        {
            OpEquality &condition = eq[i];
            if(condition.equal)
            {  // Equal
                if(condition.value1.isVariable)
                {
                    if(condition.value2.isVariable)
                    {  // ?v1 = ?v2
                        if(condition.value1.index != condition.value2.index)
                            return false;
                    }
                    else
                    {  // ?v1 = v2
                        if(a.parameters[condition.value1.index] != condition.value2.index)
                            return false;
                    }
                }
                else
                {
                    if(condition.value2.isVariable)
                    {  // v1 = ?v2
                        if(condition.value1.index != a.parameters[condition.value2.index])
                            return false;
                    }
                    else
                    {  // v1 = v2
                        if(condition.value1.index != condition.value2.index)
                            return false;
                    }
                }
            }
            else
            {  // Distinct
                if(condition.value1.isVariable)
                {
                    if(condition.value2.isVariable)
                    {  // ?v1 != ?v2
                        if(condition.value1.index == condition.value2.index)
                            return false;
                    }
                    else
                    {  // ?v1 != v2
                        if(a.parameters[condition.value1.index] == condition.value2.index)
                            return false;
                    }
                }
                else
                {
                    if(condition.value2.isVariable)
                    {  // v1 != ?v2
                        if(condition.value1.index == a.parameters[condition.value2.index])
                            return false;
                    }
                    else
                    {  // v1 != v2
                        if(condition.value1.index == condition.value2.index)
                            return false;
                    }
                }
            }
        }
        return true;
    }

    // Grounds the action preconditions
    bool Grounder::groundPreconditions(GrounderOperator &op, GroundedAction &a)
    {
        if(!groundPreconditions(op.op->atStart.prec, a.parameters, a.startCond))
        {
            return false;
        }
        if(!groundPreconditions(op.op->overAllPrec, a.parameters, a.overCond))
        {
            return false;
        }
        if(!groundPreconditions(op.op->atEnd.prec, a.parameters, a.endCond))
        {
            return false;
        }
        if(!groundPreconditions(op.op->atStart.numericPrec, a.parameters, a.startNumCond))
        {
            return false;
        }
        if(!groundPreconditions(op.op->overAllNumericPrec, a.parameters, a.overNumCond))
        {
            return false;
        }
        if(!groundPreconditions(op.op->atEnd.numericPrec, a.parameters, a.endNumCond))
        {
            return false;
        }
        return true;
    }

    // Grounds the non-numeric action preconditions
    bool Grounder::groundPreconditions(std::vector<OpFluent> &opCond,
                                       std::vector<unsigned int> &parameters,
                                       std::vector<GroundedCondition> &aCond)
    {
        for(unsigned int i = 0; i < opCond.size(); i++)
        {
            unsigned int varIndex = getVariableIndex(opCond[i].variable, parameters);
            if(varIndex == MAX_UNSIGNED_INT)  // New variable
                varIndex = createNewVariable(opCond[i].variable, parameters);
            unsigned int value = opCond[i].value.isVariable ? parameters[opCond[i].value.index] : opCond[i].value.index;
            aCond.emplace_back(varIndex, value);
        }
        return true;
    }

    // Creates a new variable and returns its index
    unsigned int Grounder::createNewVariable(const Literal &l, const std::vector<unsigned int> &opParameters)
    {
        GroundedVar v;
        v.index     = gTask->variables.size();
        v.fncIndex  = l.fncIndex;
        v.isNumeric = prepTask->task->isNumericFunction(l.fncIndex);
        for(unsigned int i = 0; i < l.params.size(); i++)
            if(l.params[i].isVariable)
                v.params.push_back(opParameters[l.params[i].index]);
            else
                v.params.push_back(l.params[i].index);
        gTask->variables.push_back(v);
        const std::string &name = getVariableName(v.fncIndex, v.params);
        variableIndex[name]     = v.index;
        unsigned int notReached = MAX_UNSIGNED_INT;
        if(v.isNumeric)
            gTask->reachedValues.emplace_back(0, notReached);
        else
            gTask->reachedValues.emplace_back(prepTask->task->objects.size(), notReached);
        return v.index;
    }

    // Grounds the numeric action preconditions
    bool Grounder::groundPreconditions(std::vector<OpNumericPrec> &opCond,
                                       std::vector<unsigned int> &parameters,
                                       std::vector<GroundedNumericCondition> &aCond)
    {
        for(unsigned int i = 0; i < opCond.size(); i++)
        {
            OpNumericPrec &cond = opCond[i];
            GroundedNumericCondition c;
            c.comparator = cond.comparator;
            for(unsigned int j = 0; j < cond.operands.size(); j++)
            {
                GroundedNumericExpression e = groundNumericExpression(cond.operands[j], parameters);
                if(e.type == GE_UNDEFINED)
                    return false;
                c.terms.push_back(e);
            }
            aCond.push_back(c);
        }
        return true;
    }

    // Grounds a numeric expression
    GroundedNumericExpression Grounder::groundNumericExpression(OpEffectExpression &exp,
                                                                std::vector<unsigned int> &parameters)
    {
        GroundedNumericExpression res;
        switch(exp.type)
        {
            case OEET_NUMBER:
                res.type  = GE_NUMBER;
                res.value = exp.value;
                break;
            case OEET_FLUENT:
                res.type  = GE_VAR;
                res.index = getVariableIndex(exp.fluent, parameters);
                if(res.index == MAX_UNSIGNED_INT)  // New variable
                    res.type = GE_UNDEFINED;
                // res.index = createNewVariable(exp.fluent, parameters);
                break;
            case OEET_DURATION:
                res.type = GE_DURATION;
                break;
            case OEET_SHARP_T:
                res.type = GE_SHARP_T;
                break;
            case OEET_TERM:
                res.type  = GE_OBJECT;
                res.index = exp.term.isVariable ? parameters[exp.term.index] : exp.term.index;
                break;
            case OEET_SHARP_T_PRODUCT:
                res.type = GE_SHARP_T;
                res.terms.push_back(groundNumericExpression(exp.operands[0], parameters));
                break;
            case OEET_SUM:
            case OEET_SUB:
            case OEET_DIV:
            case OEET_MUL:
                if(exp.type == OEET_SUM)
                    res.type = GE_SUM;
                else if(exp.type == OEET_SUB)
                    res.type = GE_SUB;
                else if(exp.type == OEET_MUL)
                    res.type = GE_MUL;
                else
                    res.type = GE_DIV;
                for(unsigned int i = 0; i < exp.operands.size(); i++)
                {
                    GroundedNumericExpression e = groundNumericExpression(exp.operands[i], parameters);
                    if(e.type == GE_UNDEFINED)
                    {
                        res.type = GE_UNDEFINED;
                        break;
                    }
                    res.terms.push_back(e);
                }
                break;
        }
        return res;
    }

    // Grounds the action effects
    bool Grounder::groundEffects(GrounderOperator &op, GroundedAction &a)
    {
        if(!groundEffects(op.op->atStart.eff, a.parameters, a.startEff))
        {
            return false;
        }
        if(!groundEffects(op.op->atEnd.eff, a.parameters, a.endEff))
        {
            return false;
        }
        if(!groundEffects(op.op->atStart.numericEff, a, TimeSpecifier::k_at_start))
        {
            return false;
        }
        if(!groundEffects(op.op->atEnd.numericEff, a, TimeSpecifier::k_at_end))
        {
            return false;
        }
        return true;
    }

    // Grounds the non-numeric action effects
    bool Grounder::groundEffects(std::vector<OpFluent> &opEff,
                                 std::vector<unsigned int> &parameters,
                                 std::vector<GroundedCondition> &aEff)
    {
        for(unsigned int i = 0; i < opEff.size(); i++)
        {
            unsigned int varIndex = getVariableIndex(opEff[i].variable, parameters);
            if(varIndex == MAX_UNSIGNED_INT)  // New variable
            {
                varIndex = createNewVariable(opEff[i].variable, parameters);
            }
            unsigned int value = opEff[i].value.isVariable ? parameters[opEff[i].value.index] : opEff[i].value.index;
            bool addEffect     = true;
            for(unsigned int j = 0; j < aEff.size(); j++)
            {
                if(aEff[j].varIndex == varIndex)
                {                                    // Variable already modified
                    if(aEff[j].valueIndex != value)  // Contradictory effects
                        return false;
                    addEffect = false;  // Repeated effect
                    break;
                }
            }
            if(addEffect)
            {
                aEff.emplace_back(varIndex, value);
            }
        }
        return true;
    }

    // Grounds the numeric action effects
    bool Grounder::groundEffects(std::vector<OpEffect> &opEff, GroundedAction &a, TimeSpecifier ts)
    {
        for(unsigned int i = 0; i < opEff.size(); i++)
        {
            OpEffect &e           = opEff[i];
            unsigned int varIndex = getVariableIndex(e.fluent, a.parameters);
            if(varIndex == MAX_UNSIGNED_INT)  // New variable
                return false;
            // varIndex = createNewVariable(e.fluent, a.parameters);
            if(e.assignment == Assignment::AS_ASSIGN && e.exp.type == OEET_TERM)
            {  // (assign variable value)
                unsigned int value = e.exp.term.isVariable ? a.parameters[e.exp.term.index] : e.exp.term.index;
                if(ts == TimeSpecifier::k_at_end)
                {
                    a.endEff.emplace_back(varIndex, value);
                }
                else
                {
                    a.startEff.emplace_back(varIndex, value);
                }
            }
            else
            {
                GroundedNumericEffect n;
                n.assignment = e.assignment;
                n.varIndex   = varIndex;
                n.exp        = groundNumericExpression(e.exp, a.parameters);
                if(n.exp.type == GE_UNDEFINED)
                {
                    return false;
                }
                if(ts == TimeSpecifier::k_at_end)
                {
                    a.endNumEff.push_back(n);
                }
                else
                {
                    a.startNumEff.push_back(n);
                }
            }
        }
        return true;
    }

    // Grounds the action preferences
    bool Grounder::groundPreferences(GrounderOperator &op, GroundedAction &a)
    {
        std::vector<OpPreference> &preferences = op.op->preference;
        for(unsigned int i = 0; i < preferences.size(); i++)
        {
            OpPreference &pref = preferences[i];
            GroundedPreference p;
            std::unordered_map<std::string, unsigned int>::const_iterator it = preferenceIndex.find(pref.name);
            if(it == preferenceIndex.end())
            {
                p.nameIndex                = gTask->preferenceNames.size();
                preferenceIndex[pref.name] = p.nameIndex;
                gTask->preferenceNames.push_back(pref.name);
            }
            else
            {
                p.nameIndex = it->second;
            }
            p.preference = groundGoalDescription(pref.preference, a.parameters);
            a.preferences.push_back(p);
        }
        return true;
    }

    // Grounds the action duration
    bool Grounder::groundDuration(GrounderOperator &op, GroundedAction &a)
    {
        std::vector<Duration> &duration = op.op->duration;
        for(unsigned int i = 0; i < duration.size(); i++)
        {
            GroundedDuration d;
            d.time = duration[i].time;
            d.comp = duration[i].comp;
            d.exp  = groundNumericExpression(duration[i].exp, a.parameters);
            if(d.exp.type == GE_UNDEFINED)
                return false;
            a.duration.push_back(d);
        }
        return true;
    }

    // Grounds a numeric expression
    GroundedNumericExpression Grounder::groundNumericExpression(NumericExpression &exp,
                                                                std::vector<unsigned int> &parameters)
    {
        GroundedNumericExpression res;
        switch(exp.type)
        {
            case NET_NUMBER:
                res.type  = GE_NUMBER;
                res.value = exp.value;
                break;
            case NET_FUNCTION:
                res.type  = GE_VAR;
                res.index = getVariableIndex(exp.function, parameters);
                if(res.index == MAX_UNSIGNED_INT)  // New variable
                    res.type = GE_UNDEFINED;
                // res.index = createNewVariable(exp.function, parameters);
                break;
            case NET_SUM:
            case NET_SUB:
            case NET_DIV:
            case NET_MUL:
                if(exp.type == NET_SUM)
                    res.type = GE_SUM;
                else if(exp.type == NET_SUB)
                    res.type = GE_SUB;
                else if(exp.type == NET_MUL)
                    res.type = GE_MUL;
                else
                    res.type = GE_DIV;
                for(unsigned int i = 0; i < exp.operands.size(); i++)
                {
                    // res.terms.push_back(groundNumericExpression(exp.operands[i], parameters));
                    GroundedNumericExpression e = groundNumericExpression(exp.operands[i], parameters);
                    if(e.type == GE_UNDEFINED)
                    {
                        res.type = GE_UNDEFINED;
                        break;
                    }
                    res.terms.push_back(e);
                }
                break;
            case NET_TERM:
                res.type  = GE_OBJECT;
                res.index = exp.term.isVariable ? parameters[exp.term.index] : exp.term.index;
                break;
            default:
                std::cout << "Unexpected numeric expression" << std::endl;
                assert(false);
        }
        return res;
    }

    // Grounds a goal description
    GroundedGoalDescription Grounder::groundGoalDescription(GoalDescription &g, std::vector<unsigned int> &parameters)
    {
        GroundedGoalDescription res;
        res.time = g.time;
        switch(g.type)
        {
            case GD_LITERAL:
            case GD_NEG_LITERAL:
                addVariableComparison(
                    res,
                    g.literal,
                    false,
                    g.type == GD_LITERAL ? prepTask->task->CONSTANT_TRUE : prepTask->task->CONSTANT_FALSE,
                    parameters,
                    true);
                break;
            case GD_AND:
                res.type = GG_AND;
                break;
            case GD_OR:
                res.type = GG_OR;
                break;
            case GD_NOT:
                res.type = GG_NOT;
                break;
            case GD_IMPLY:
                res.type = GG_IMPLY;
                break;
            case GD_EXISTS:
            case GD_FORALL:
                res.type = g.type == GD_EXISTS ? GG_EXISTS : GG_FORALL;
                for(unsigned int i = 0; i < g.parameters.size(); i++)
                    res.paramTypes.push_back(g.parameters[i].types);
                break;
            case GD_EQUALITY:
            case GD_INEQUALITY:
                res.type = g.type == GD_EQUALITY ? GG_EQUALITY : GG_INEQUALITY;
                for(unsigned int i = 0; i < g.eqTerms.size(); i++)
                    res.addTerm(g.eqTerms[i], parameters);
                break;
            case GD_F_CMP:
                res.type = GG_COMP;
                if((g.comparator == Comparator::k_eq || g.comparator == Comparator::k_neq) && g.exp.size() == 2)
                {
                    if(g.exp[0].type == NET_FUNCTION && g.exp[1].type == NET_TERM)
                    {
                        addVariableComparison(res,
                                              g.exp[0].function,
                                              g.exp[1].term.isVariable,
                                              g.exp[1].term.index,
                                              parameters,
                                              g.comparator == Comparator::k_eq);
                    }
                    else if(g.exp[0].type == NET_TERM && g.exp[1].type == NET_FUNCTION)
                    {
                        addVariableComparison(res,
                                              g.exp[1].function,
                                              g.exp[0].term.isVariable,
                                              g.exp[0].term.index,
                                              parameters,
                                              g.comparator == Comparator::k_eq);
                    }
                    else if(g.exp[0].type == NET_TERM && g.exp[1].type == NET_TERM)
                    {
                        res.type = g.comparator == Comparator::k_eq ? GG_EQUALITY : GG_INEQUALITY;
                        for(unsigned int i = 0; i < g.eqTerms.size(); i++)
                            res.addTerm(g.exp[0].term, parameters);
                        res.addTerm(g.exp[1].term, parameters);
                    }
                }
                if(res.type == GG_COMP)
                {
                    res.comparator = g.comparator;
                    for(unsigned int i = 0; i < g.exp.size(); i++)
                        res.exp.push_back(partiallyGroundNumericExpression(g.exp[i], parameters));
                }
                break;
        }
        for(unsigned int i = 0; i < g.terms.size(); i++)
            res.terms.push_back(groundGoalDescription(g.terms[i], parameters));
        return res;
    }

    // Adds a variable comparison to a grounded goal description
    void Grounder::addVariableComparison(GroundedGoalDescription &g,
                                         Literal &literal,
                                         bool valueIsParam,
                                         unsigned int valueIndex,
                                         std::vector<unsigned int> &parameters,
                                         bool equal)
    {
        if(canGroundVariable(literal, parameters.size()))
        {
            g.type  = GG_FLUENT;
            g.index = getVariableIndex(literal, parameters);
            if(g.index == MAX_UNSIGNED_INT)  // New variable
                g.index = createNewVariable(literal, parameters);
        }
        else
        {
            g.type  = GG_UNGROUNDED_FLUENT;
            g.index = literal.fncIndex;
            for(unsigned int i = 0; i < literal.params.size(); i++)
                g.addTerm(literal.params[i], parameters);
        }
        g.equal        = equal;
        g.valueIsParam = valueIsParam;
        g.value        = valueIndex;
    }

    // Checks if the variable can be grounded. It's not possible if the literal
    // uses the parameters of a forall or exists statement
    bool Grounder::canGroundVariable(Literal &literal, unsigned int numParameters)
    {
        for(unsigned int i = 0; i < literal.params.size(); i++)
            if(literal.params[i].isVariable && literal.params[i].index >= numParameters)
                return false;
        return true;
    }

    // Partially grounds a numeric expression (it can contain unresolved parameters from existential and universal
    // conditions)
    PartiallyGroundedNumericExpression Grounder::partiallyGroundNumericExpression(NumericExpression &exp,
                                                                                  std::vector<unsigned int> &parameters)
    {
        PartiallyGroundedNumericExpression res;
        switch(exp.type)
        {
            case NET_NUMBER:
                res.type  = PGE_NUMBER;
                res.value = exp.value;
                break;
            case NET_FUNCTION:
                if(canGroundVariable(exp.function, parameters.size()))
                {
                    res.type  = PGE_VAR;
                    res.index = getVariableIndex(exp.function, parameters);
                    if(res.index == MAX_UNSIGNED_INT)  // New variable
                        res.index = createNewVariable(exp.function, parameters);
                }
                else
                {
                    res.type  = PGE_UNGROUNDED_VAR;
                    res.index = exp.function.fncIndex;
                    for(unsigned int i = 0; i < exp.function.params.size(); i++)
                        res.addTerm(exp.function.params[i], parameters);
                }
                break;
            case NET_SUM:
            case NET_SUB:
            case NET_DIV:
            case NET_MUL:
                if(exp.type == NET_SUM)
                    res.type = PGE_SUM;
                else if(exp.type == NET_SUB)
                    res.type = PGE_SUB;
                else if(exp.type == NET_MUL)
                    res.type = PGE_MUL;
                else
                    res.type = PGE_DIV;
                for(unsigned int i = 0; i < exp.operands.size(); i++)
                    res.terms.push_back(partiallyGroundNumericExpression(exp.operands[i], parameters));
                break;
            case NET_TERM:
                res.type = PGE_TERM;
                res.addTerm(exp.term, parameters);
                break;
            case NET_NEGATION:
                res.type = PGE_NOT;
                res.terms.push_back(partiallyGroundNumericExpression(exp.operands[0], parameters));
        }
        return res;
    }

    // Checks and removes the static variables after the grounding process
    void Grounder::removeStaticVariables()
    {
        unsigned int numVars = gTask->variables.size(), numActions = gTask->actions.size(),
                     invalidIndex = MAX_UNSIGNED_INT;
        std::vector<bool> staticVar(numVars, true);
        std::vector<unsigned int> newIndex;
        std::vector<VariableValue> value;
        for(unsigned int i = 0; i < numActions; ++i)
        {
            checkStaticVariables(gTask->actions[i], staticVar);
        }
        unsigned int index = 0;
        for(unsigned int i = 0; i < numVars; ++i)
        {
            if(staticVar[i])
            {
                std::vector<Fact *> initValues;  // Initial values for this variable. Can be multiple due to the
                                                 // time-initial literals (TIL)
                getInitialValues(i, initValues);
                VariableValue v;
                if(initValues.size() > 1)
                {
                    staticVar[i] = false;  // TIL are not static
                }
                else if(initValues.size() == 0)
                {
                    // Check if function is boolean. In that case the initial value is false
                    unsigned int fncIndex = gTask->variables[i].fncIndex;
                    if(prepTask->task->isBooleanFunction(fncIndex))
                    {
                        v.valueIsNumeric = false;
                        v.value          = prepTask->task->CONSTANT_FALSE;
                    }
                    else
                    {
                        v.valueIsNumeric = false;
                        v.value          = MAX_UNSIGNED_INT;  // Undefined value
                    }
                }
                else
                {
                    Fact *f = initValues[0];
                    if(f->time > 0)
                    {                          // TIL
                        staticVar[i] = false;  // TIL are not static
                    }
                    else
                    {
                        // Initial-state fluent
                        if(f->valueIsNumeric)
                        {
                            v.valueIsNumeric = true;
                            v.numericValue   = f->numericValue;
                        }
                        else
                        {
                            v.valueIsNumeric = false;
                            v.value          = f->value;
                        }
                    }
                }
                value.push_back(v);
                if(staticVar[i])
                {
                    newIndex.push_back(invalidIndex);  // Remove
                }
                else
                {  // Next variable
                    newIndex.push_back(index);
                    index++;
                }
            }
            else
            {  // Next variable
                value.emplace_back();
                newIndex.push_back(index);
                index++;
            }
        }
        removeStaticVariables(staticVar, newIndex, value);
        for(unsigned int i = 0; i < gTask->variables.size(); i++)
        {
            gTask->variables[i].index = i;
        }
    }

    // Checks the non-static variables (variables that are modified) in an action
    void Grounder::checkStaticVariables(GroundedAction &a, std::vector<bool> &staticVar)
    {
        for(unsigned int i = 0; i < a.startEff.size(); i++)
        {
            staticVar[a.startEff[i].varIndex] = false;
        }
        for(unsigned int i = 0; i < a.endEff.size(); i++)
        {
            staticVar[a.endEff[i].varIndex] = false;
        }
        for(unsigned int i = 0; i < a.startNumEff.size(); i++)
        {
            staticVar[a.startNumEff[i].varIndex] = false;
        }
        for(unsigned int i = 0; i < a.endNumEff.size(); i++)
        {
            staticVar[a.endNumEff[i].varIndex] = false;
        }
    }

    // Stores in the std::vector the initial values for this variable. Can be multiple due to the time-initial literals
    void Grounder::getInitialValues(unsigned int varIndex, std::vector<Fact *> &initValues)
    {
        std::vector<Fact> &init = prepTask->task->init;
        GroundedVar &var        = gTask->variables[varIndex];
        for(unsigned int i = 0; i < init.size(); i++)
            if(init[i].function == var.fncIndex)
            {
                bool equal = true;
                for(unsigned int j = 0; j < var.params.size(); j++)
                    if(var.params[j] != init[i].parameters[j])
                    {
                        equal = false;
                        break;
                    }
                if(equal)
                    initValues.push_back(&init[i]);
            }
    }

    // Removes the static variables after the grounding process
    void Grounder::removeStaticVariables(std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        groupVariables(staticVar, newIndex);
#ifdef _GROUNDER_TRACE_ON_
        for(unsigned int i = 0; i < staticVar.size(); i++)
            std::cout << "Variable: " << gTask->variables[i].toString(prepTask->task) << staticVar[i] << ", " << i
                      << " -> " << newIndex[i] << ", "
                      << (value[i].valueIsNumeric ? value[i].numericValue : value[i].value) << std::endl;
#endif
        unsigned int i = 0;
        while(i < gTask->actions.size())
        {
            GroundedAction &a = gTask->actions[i];
            a.index           = i;
            for(unsigned int j = 0; j < a.duration.size(); j++)
                removeStaticVariables(a.duration[j].exp, staticVar, newIndex, value);
            bool remove = (a.duration.size() == 1 && a.duration[0].exp.value <= 0 &&
                           a.duration[0].exp.type == GE_NUMBER &&  // Delete actions with invalid duration
                           (a.duration[0].comp == Comparator::k_eq || a.duration[0].comp == Comparator::k_less ||
                            a.duration[0].comp == Comparator::k_less_eq));
            if(!remove)
                remove = removeStaticVariables(a.startCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.overCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.overNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startNumEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endNumEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.preferences, staticVar, newIndex, value);
            if(remove)
                gTask->actions.erase(gTask->actions.begin() + i);
            else
                i++;
        }
        i = 0;
        while(i < gTask->goals.size())
        {
            GroundedAction &a = gTask->goals[i];
            a.index           = i;
            for(unsigned int j = 0; j < a.duration.size(); j++)
                removeStaticVariables(a.duration[j].exp, staticVar, newIndex, value);
            bool remove = (a.duration.size() == 1 && a.duration[0].exp.value <= 0 &&
                           a.duration[0].exp.type == GE_NUMBER &&  // Delete actions with invalid duration
                           (a.duration[0].comp == Comparator::k_eq || a.duration[0].comp == Comparator::k_less ||
                            a.duration[0].comp == Comparator::k_less_eq));
            if(!remove)
                remove = removeStaticVariables(a.startCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.overCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.overNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endNumCond, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.startNumEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.endNumEff, staticVar, newIndex, value);
            if(!remove)
                remove = removeStaticVariables(a.preferences, staticVar, newIndex, value);
            if(remove)
                gTask->goals.erase(gTask->goals.begin() + i);
            else
                i++;
        }
        unsigned int numNonStaticVars = 0;
        for(unsigned int i = 0; i < staticVar.size(); i++)
            if(!staticVar[i])
                numNonStaticVars++;
        std::vector<GroundedVar> oldVariables                    = gTask->variables;
        std::vector<std::vector<unsigned int> > oldReachedValues = gTask->reachedValues;
        gTask->variables.resize(numNonStaticVars);
        gTask->reachedValues.resize(numNonStaticVars);
        for(unsigned int i = 0; i < staticVar.size(); i++)
        {
            if(!staticVar[i])
            {
                gTask->variables[newIndex[i]]     = oldVariables[i];
                gTask->reachedValues[newIndex[i]] = oldReachedValues[i];
            }
        }
        i = 0;
        while(i < gTask->constraints.size())
        {
            if(removeStaticVariables(gTask->constraints[i], staticVar, newIndex, value))
                gTask->constraints.erase(gTask->constraints.begin() + i);
            else
                i++;
        }
        if(gTask->metricType != 'X')
            removeStaticVariables(gTask->metric, staticVar, newIndex, value);
    }

    // Removes the static variables in the metric
    void Grounder::removeStaticVariables(GroundedMetric &m,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        switch(m.type)
        {
            case MT_PLUS:
            case MT_MINUS:
            case MT_PROD:
            case MT_DIV:
                for(unsigned int i = 0; i < m.terms.size(); i++)
                    removeStaticVariables(m.terms[i], staticVar, newIndex, value);
                break;
            case MT_FLUENT:
                if(staticVar[m.index])
                {
                    m.type  = MT_NUMBER;
                    m.value = value[m.index].numericValue;
                }
                else
                    m.index = newIndex[m.index];
                break;
            default:;
        }
    }

    // Puts all non-numeric variables together at the first places. The numeric variables will have the last indexes
    void Grounder::groupVariables(std::vector<bool> &staticVar, std::vector<unsigned int> &newIndex)
    {
        int numVars = staticVar.size(), i = 0, j = numVars - 1;
        while(i < j)
        {
            while(i < numVars && (staticVar[i] || !gTask->variables[i].isNumeric))
                i++;
            while(j >= 0 && (staticVar[j] || gTask->variables[j].isNumeric))
                j--;
            if(i < j && i < numVars && j >= 0)
            {  // Exchange indexes
                unsigned int aux = newIndex[i];
                newIndex[i]      = newIndex[j];
                newIndex[j]      = aux;
                i++;
                j--;
            }
        }
    }

    // Removes the static variables and updates the indexes of the other variables in the action duration
    bool Grounder::removeStaticVariables(GroundedNumericExpression &e,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        switch(e.type)
        {
            case GE_VAR:
                if(staticVar[e.index])
                {
                    if(!value[e.index].valueIsNumeric && value[e.index].value == MAX_UNSIGNED_INT)
                        return true;  // Undefined initial value -> remove action
                    e.type  = GE_NUMBER;
                    e.value = value[e.index].numericValue;
                }
                else
                    e.index = newIndex[e.index];
                break;
            case GE_SUM:
            case GE_SUB:
            case GE_DIV:
            case GE_MUL:
            {
                bool canCompute = true;
                for(unsigned int i = 0; i < e.terms.size(); i++)
                {
                    if(removeStaticVariables(e.terms[i], staticVar, newIndex, value))
                        return true;
                    if(e.terms[i].type != GE_NUMBER)
                        canCompute = false;
                }
                if(canCompute)
                {
                    e.value = computeExpressionValue(e);
                    e.type  = GE_NUMBER;
                }
            }
            break;
            default:;
        }
        return false;
    }

    // Computes the value of an operation where all the terms are numbers
    float Grounder::computeExpressionValue(GroundedNumericExpression &e)
    {
        float value = e.terms[0].value;
        switch(e.type)
        {
            case GE_SUM:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value += e.terms[i].value;
                break;
            case GE_SUB:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value -= e.terms[i].value;
                break;
            case GE_MUL:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value *= e.terms[i].value;
                break;
            default:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    if(e.terms[i].value != 0)
                        value /= e.terms[i].value;
                    else
                    {
                        std::cout << "Division by zero" << std::endl;
                        assert(false);
                    }
                break;
        }
        return value;
    }

    // Removes the static variables and updates the indexes of the other variables in the action conditions
    bool Grounder::removeStaticVariables(std::vector<GroundedCondition> &cond,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        unsigned int i = 0;
        while(i < cond.size())
        {
            GroundedCondition &c = cond[i];
            if(staticVar[c.varIndex])
            {  // Remove condition
                if(!value[c.varIndex].valueIsNumeric && value[c.varIndex].value == MAX_UNSIGNED_INT)
                    return true;  // Undefined initial value -> remove action
                if(value[c.varIndex].value == c.valueIndex)
                {
                    cond.erase(cond.begin() + i);
                }
                else
                    return true;  // Remove action: static condition does not hold
            }
            else
            {
                c.varIndex = newIndex[c.varIndex];  // Update index
                i++;
            }
        }
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in the action numeric conditions
    bool Grounder::removeStaticVariables(std::vector<GroundedNumericCondition> &cond,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        unsigned int i = 0;
        GroundedNumericExpressionType type;
        while(i < cond.size())
        {
            GroundedNumericCondition &c = cond[i];
            bool allNumbers             = true;
            for(unsigned int j = 0; j < c.terms.size(); j++)
            {
                if(removeStaticVariables(c.terms[j], staticVar, newIndex, value))
                    return true;
                type = c.terms[j].type;
                if(type != GE_NUMBER)
                    allNumbers = false;
            }
            if(allNumbers)
            {  // Example: 5 < 4
                if(numericComparisonHolds(c))
                    i++;  // Next condition
                else
                    return true;  // Remove action (contradictory precondition)
            }
            else
                i++;  // Next condition
        }
        return false;
    }

    // Checks whether a numeric comparison holds
    bool Grounder::numericComparisonHolds(GroundedNumericCondition &c)
    {
        float v1 = c.terms[0].value, v2 = c.terms[1].value;
        switch(c.comparator)
        {
            case Comparator::k_eq:
                return v1 == v2;
            case Comparator::k_less:
                return v1 < v2;
            case Comparator::k_less_eq:
                return v1 <= v2;
            case Comparator::k_greater:
                return v1 > v2;
            case Comparator::k_greater_eq:
                return v1 >= v2;
            case Comparator::k_neq:
                return v1 != v2;
        }
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in the action numeric effects
    bool Grounder::removeStaticVariables(std::vector<GroundedNumericEffect> &e,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        for(unsigned int i = 0; i < e.size(); i++)
        {
            e[i].varIndex = newIndex[e[i].varIndex];
            if(removeStaticVariables(e[i].exp, staticVar, newIndex, value))
                return true;
        }
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in the action preferences
    bool Grounder::removeStaticVariables(std::vector<GroundedPreference> &p,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        for(unsigned int i = 0; i < p.size(); i++)
            if(removeStaticVariables(p[i].preference, staticVar, newIndex, value))
                return true;
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in the action preference
    bool Grounder::removeStaticVariables(GroundedGoalDescription &g,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        switch(g.type)
        {
            case GG_FLUENT:
                if(staticVar[g.index])
                {
                    if(!value[g.index].valueIsNumeric && value[g.index].value == MAX_UNSIGNED_INT)
                        return true;  // Undefined initial value -> remove action
                    g.type = g.equal ? GG_EQUALITY : GG_INEQUALITY;
                    g.isParameter.push_back(false);
                    g.paramIndex.push_back(value[g.index].value);
                    g.isParameter.push_back(false);
                    g.paramIndex.push_back(g.value);
                }
                else
                    g.index = newIndex[g.index];
                break;
            case GG_AND:
            case GG_OR:
            case GG_NOT:
            case GG_EXISTS:
            case GG_FORALL:
            case GG_IMPLY:
                for(unsigned int i = 0; i < g.terms.size(); i++)
                    if(removeStaticVariables(g.terms[i], staticVar, newIndex, value))
                        return true;
                break;
            case GG_COMP:
                for(unsigned int i = 0; i < g.exp.size(); i++)
                    if(removeStaticVariables(g.exp[i], staticVar, newIndex, value))
                        return true;
                break;
            default:;
        }
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in a numeric condition
    bool Grounder::removeStaticVariables(PartiallyGroundedNumericExpression &e,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        switch(e.type)
        {
            case PGE_VAR:
                if(staticVar[e.index])
                {
                    if(!value[e.index].valueIsNumeric && value[e.index].value == MAX_UNSIGNED_INT)
                        return true;  // Undefined initial value -> remove action
                    e.type  = PGE_NUMBER;
                    e.value = value[e.index].numericValue;
                }
                else
                    e.index = newIndex[e.index];
                break;
            case PGE_SUM:
            case PGE_SUB:
            case PGE_DIV:
            case PGE_MUL:
            {
                bool canCompute = true;
                for(unsigned int i = 0; i < e.terms.size(); i++)
                {
                    if(removeStaticVariables(e.terms[i], staticVar, newIndex, value))
                        return true;
                    if(e.terms[i].type != PGE_NUMBER)
                        canCompute = false;
                }
                if(canCompute)
                {
                    e.value = computeExpressionValue(e);
                    e.type  = PGE_NUMBER;
                }
            }
            break;
            case PGE_NOT:
                if(removeStaticVariables(e.terms[0], staticVar, newIndex, value))
                    return true;
                break;
            default:;
        }
        return false;
    }

    // Removes the static variables and updates the indexes of the other variables in a constraint
    bool Grounder::removeStaticVariables(GroundedConstraint &c,
                                         std::vector<bool> &staticVar,
                                         std::vector<unsigned int> &newIndex,
                                         std::vector<VariableValue> &value)
    {
        switch(c.type)
        {
            case RT_AND:
                for(unsigned int i = 0; i < c.terms.size(); i++)
                {
                    if(removeStaticVariables(c.terms[i], staticVar, newIndex, value))
                        return true;
                }
                break;
            case RT_PREFERENCE:
                if(removeStaticVariables(c.terms[0], staticVar, newIndex, value))
                    return true;
                break;
            case RT_GOAL_PREFERENCE:
                if(removeStaticVariables(c.goal[0], staticVar, newIndex, value))
                    return true;
                break;
            default:
                for(unsigned int i = 0; i < c.goal.size(); i++)
                {
                    if(removeStaticVariables(c.goal[i], staticVar, newIndex, value))
                        return true;
                }
                break;
        }
        return false;
    }

    // Computes the value of an operation where all the terms are numbers
    float Grounder::computeExpressionValue(PartiallyGroundedNumericExpression &e)
    {
        float value = e.terms[0].value;
        switch(e.type)
        {
            case PGE_SUM:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value += e.terms[i].value;
                break;
            case PGE_SUB:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value -= e.terms[i].value;
                break;
            case PGE_MUL:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    value *= e.terms[i].value;
                break;
            default:
                for(unsigned int i = 1; i < e.terms.size(); i++)
                    if(e.terms[i].value != 0)
                        value /= e.terms[i].value;
                    else
                    {
                        std::cout << "Division by zero" << std::endl;
                        assert(false);
                    }
                break;
        }
        return value;
    }

    // Computes the initial-state values of the variables
    void Grounder::computeInitialVariableValues()
    {
        for(unsigned int i = 0; i < gTask->variables.size(); i++)
        {
            std::vector<Fact *>
                initValues;  // Initial values for this variable. Can be multiple due to the time-initial literals (TIL)
            getInitialValues(i, initValues);
            GroundedVar &v = gTask->variables[i];
            for(unsigned int j = 0; j < initValues.size(); j++)
            {
                Fact *f = initValues[j];
                GroundedValue value;
                value.time         = f->time;
                value.value        = f->value;
                value.numericValue = f->numericValue;
                v.initialValues.push_back(value);
                // std::cout << v.toString(prepTask->task) << " = " << value.toString(prepTask->task, v.isNumeric) <<
                // ")" << std::endl;
            }
        }
    }

    // Removes forall and exists conditions in preferences
    void Grounder::removeADLFeaturesInPreferences()
    {
        unsigned int numActions = gTask->actions.size();
        for(unsigned int i = 0; i < numActions; i++)
            if(gTask->actions[i].preferences.size() > 0)
                removeADLFeaturesInPreferences(gTask->actions[i]);
        numActions = gTask->goals.size();
        for(unsigned int i = 0; i < numActions; i++)
            if(gTask->goals[i].preferences.size() > 0)
                removeADLFeaturesInPreferences(gTask->goals[i]);
    }

    // Removes forall and exists conditions in the preferences of an action
    void Grounder::removeADLFeaturesInPreferences(GroundedAction &a)
    {
        for(unsigned int i = 0; i < a.preferences.size(); i++)
            removeADLFeaturesInPreferences(&(a.preferences[i].preference));
    }

    // Removes forall and exists conditions in a preference
    void Grounder::removeADLFeaturesInPreferences(GroundedGoalDescription *pref)
    {
        switch(pref->type)
        {
            case GG_FLUENT:
            case GG_EQUALITY:
            case GG_INEQUALITY:
            case GG_COMP:
                break;
            case GG_UNGROUNDED_FLUENT:
                std::cout << "Ungrounded fluent in preference" << std::endl;
                assert(false);
                break;
            case GG_AND:
            case GG_OR:
            case GG_NOT:
            case GG_IMPLY:
                for(unsigned int i = 0; i < pref->terms.size(); i++)
                    removeADLFeaturesInPreferences(&(pref->terms[i]));
                break;
            case GG_EXISTS:
            case GG_FORALL:
                if(pref->type == GG_EXISTS)
                    pref->type = GG_OR;
                else
                    pref->type = GG_AND;
                std::unordered_map<unsigned int, unsigned int> parameters;  // forall/exists parameter -> problem object
                GroundedGoalDescription condition = pref->terms[0];
                pref->terms.clear();
                replaceADLPreference(pref, 0, 0, &parameters, &condition);
                break;
        }
    }

    // Replaces the parameters of the forall/exists condition by objects
    void Grounder::replaceADLPreference(GroundedGoalDescription *pref,
                                        unsigned int numParam,
                                        unsigned int prevParams,
                                        std::unordered_map<unsigned int, unsigned int> *parameters,
                                        GroundedGoalDescription *condition)
    {
        if(numParam >= pref->paramTypes.size())
        {
            GroundedGoalDescription newCond = groundPreference(condition, numParam + prevParams, parameters);
            if(newCond.type != MAX_UNSIGNED_INT)
            {
                if(pref->type == GG_AND)
                {
                    if(newCond.type == GG_INEQUALITY)
                    {
                        pref->type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                    }
                    else if(newCond.type != GG_EQUALITY)
                    {
                        pref->terms.push_back(newCond);
                    }
                }
                else if(pref->type == GG_OR)
                {
                    if(newCond.type == GG_EQUALITY)
                    {
                        pref->type = GG_EQUALITY;
                        pref->terms.clear();
                    }
                    else if(newCond.type != GG_INEQUALITY)
                    {
                        pref->terms.push_back(newCond);
                    }
                }
                else
                {
                    pref->terms.push_back(newCond);
                }
            }
        }
        else
        {
            for(unsigned int i = 0; i < gTask->task->objects.size(); i++)
            {
                if(gTask->task->compatibleTypes(gTask->task->objects[i].types, pref->paramTypes[numParam]))
                {
                    (*parameters)[numParam + prevParams] = i;
                    replaceADLPreference(pref, numParam + 1, prevParams, parameters, condition);
                }
            }
            if((pref->type == GG_AND || pref->type == GG_OR) && pref->terms.size() == 1)
                *pref = pref->terms[0];
        }
    }

    // Replaces the parameters of the ungrounded conditions in a preference by objects
    GroundedGoalDescription Grounder::groundPreference(GroundedGoalDescription *condition,
                                                       unsigned int numParam,
                                                       std::unordered_map<unsigned int, unsigned int> *parameters)
    {
        GroundedGoalDescription c;
        c.time = condition->time;
        c.type = condition->type;
        switch(condition->type)
        {
            case GG_FLUENT:
                c.index = condition->index;  // variable index
                c.value = condition->value;  // value (object index)
                c.equal = condition->equal;  // equal/distinct
                break;
            case GG_UNGROUNDED_FLUENT:
                for(unsigned int i = 0; i < gTask->variables.size(); i++)
                {
                    GroundedVar &v = gTask->variables[i];
                    if(v.fncIndex == condition->index)
                    {  // Correct function -> check parameters
                        bool found = v.params.size() == condition->paramIndex.size();
                        for(unsigned int j = 0; j < condition->paramIndex.size() && found; j++)
                            if(condition->isParameter[j])
                            {
                                unsigned int objIndex = (*parameters)[condition->paramIndex[j]];
                                found                 = objIndex == v.params[j];
                            }
                            else
                                found = condition->paramIndex[j] == v.params[j];
                        if(found)
                        {
                            c.index        = i;
                            c.valueIsParam = false;
                            c.value = condition->valueIsParam ? (*parameters)[condition->value] : condition->value;
                            c.equal = condition->equal;
                            c.type  = GG_FLUENT;
                            break;
                        }
                    }
                }
                if(c.type == GG_UNGROUNDED_FLUENT)
                    c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                break;
            case GG_AND:
            case GG_OR:
                for(unsigned int i = 0; i < condition->terms.size(); i++)
                {
                    GroundedGoalDescription term = groundPreference(&(condition->terms[i]), numParam, parameters);
                    if(term.type != GG_EQUALITY)
                    {  // Equality conditions that hold are not added
                        if(term.type == GG_INEQUALITY)
                        {  // Equality condition that doesn't hold
                            if(condition->type == GG_AND)
                            {  // A false condition in an AND invalidates all the condition
                                c.type = GG_INEQUALITY;
                                c.terms.clear();
                                break;
                            }  // If it's OR we remove the conditions
                        }
                        else if(term.type == MAX_UNSIGNED_INT)
                        {
                            c.terms.clear();
                            c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                            break;
                        }
                        else
                            c.terms.push_back(term);
                    }
                    else if(condition->type == GG_OR)
                    {  // A true condition makes true the whole OR
                        c.type = GG_EQUALITY;
                        c.terms.clear();
                        break;
                    }
                }
                if(c.terms.size() == 1)
                    c = c.terms[0];
                break;
            case GG_IMPLY:  // P -> Q = NOT(P) OR Q
            {
                assert(condition->terms.size() == 2);
                GroundedGoalDescription term1 = groundPreference(&(condition->terms[0]), numParam, parameters);
                if(term1.type == MAX_UNSIGNED_INT)
                {
                    c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                }
                else
                {
                    if(term1.type == GG_INEQUALITY)
                        c.type = GG_EQUALITY;  // NOT(P) holds, so Q is not required
                    else if(term1.type == GG_EQUALITY)
                    {  // NOT(P) doesn't hold, so Q is required
                        c = groundPreference(&(condition->terms[1]), numParam, parameters);
                    }
                    else
                    {
                        GroundedGoalDescription term2 = groundPreference(&(condition->terms[1]), numParam, parameters);
                        if(term2.type == MAX_UNSIGNED_INT)
                        {
                            c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                        }
                        else
                        {
                            if(term2.type == GG_EQUALITY)
                                c.type = GG_EQUALITY;  // Q holds
                            else if(term2.type == GG_INEQUALITY)
                            {
                                c.type = GG_NOT;
                                c.terms.push_back(term1);  // Q doesn't hold -> NOT(P)
                            }
                            else
                            {
                                c.type = GG_OR;
                                GroundedGoalDescription notTerm1;
                                notTerm1.type = GG_NOT;
                                notTerm1.terms.push_back(term1);
                                c.terms.push_back(notTerm1);
                                c.terms.push_back(term2);
                            }
                        }
                    }
                }
            }
            break;
            case GG_NOT:
            {
                assert(condition->terms.size() == 1);
                GroundedGoalDescription term = groundPreference(&(condition->terms[0]), numParam, parameters);
                if(term.type == MAX_UNSIGNED_INT)
                {
                    c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                }
                else
                {
                    if(term.type == GG_INEQUALITY)
                        c.type = GG_EQUALITY;
                    else if(term.type == GG_EQUALITY)
                        c.type = GG_INEQUALITY;
                    else
                        c.terms.push_back(term);
                }
            }
            break;
            case GG_EXISTS:
            case GG_FORALL:
            {
                if(condition->type == GG_EXISTS)
                    c.type = GG_OR;
                else
                    c.type = GG_AND;
                GroundedGoalDescription adlCondition = condition->terms[0];
                replaceADLPreference(&c, 0, numParam, parameters, &adlCondition);
                if(c.terms.size() == 0 || c.terms[0].type == MAX_UNSIGNED_INT)
                    c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                else if(c.terms.size() == 1)
                    c = c.terms[0];
            }
            break;
            case GG_EQUALITY:
            case GG_INEQUALITY:  // We are going to remove equality conditions
                if(condition->paramIndex.size() == 2)
                {
                    unsigned int obj1 =
                        condition->isParameter[0] ? (*parameters)[condition->paramIndex[0]] : condition->paramIndex[0];
                    unsigned int obj2 =
                        condition->isParameter[1] ? (*parameters)[condition->paramIndex[1]] : condition->paramIndex[1];
                    if(c.type == GG_EQUALITY)
                    {
                        if(obj1 != obj2)
                            c.type = GG_INEQUALITY;  // Condition doesn't hold
                    }
                    else if(obj1 != obj2)
                        c.type = GG_EQUALITY;  // Condition holds
                }
                else
                {
                    std::cout << "Invalid number of parameters in equality condition" << std::endl;
                    assert(false);
                }
                break;
            case GG_COMP:
                c.comparator = condition->comparator;
                for(unsigned int i = 0; i < condition->exp.size(); i++)
                {
                    PartiallyGroundedNumericExpression e =
                        groundPartiallyGroundedNumericExpression(&(condition->exp[i]), parameters);
                    if(e.type == MAX_UNSIGNED_INT)
                    {
                        c.type = (GroundedGoalDescriptionType)MAX_UNSIGNED_INT;
                        break;
                    }
                    else
                        c.exp.push_back(e);
                }
                break;
        }
        return c;
    }

    // Replaces the parameters of a ungrounded numeric expression in a preference by objects
    PartiallyGroundedNumericExpression Grounder::groundPartiallyGroundedNumericExpression(
        PartiallyGroundedNumericExpression *exp,
        std::unordered_map<unsigned int, unsigned int> *parameters)
    {
        PartiallyGroundedNumericExpression e;
        e.type = exp->type;
        switch(exp->type)
        {
            case PGE_NUMBER:
                e.value = exp->value;
                break;
            case PGE_VAR:
                e.index = exp->index;
                break;
            case PGE_UNGROUNDED_VAR:
                for(unsigned int i = 0; i < gTask->variables.size(); i++)
                {
                    GroundedVar &v = gTask->variables[i];
                    if(v.fncIndex == exp->index && v.isNumeric)
                    {  // Correct function -> check parameters
                        bool found = v.params.size() == exp->paramIndex.size();
                        for(unsigned int j = 0; j < exp->paramIndex.size() && found; j++)
                            if(exp->isParameter[j])
                            {
                                unsigned int objIndex = (*parameters)[exp->paramIndex[j]];
                                found                 = objIndex == v.params[j];
                            }
                            else
                                found = exp->paramIndex[j] == v.params[j];
                        if(found)
                        {
                            e.type  = PGE_VAR;
                            e.index = i;
                            break;
                        }
                    }
                }
                if(e.type == PGE_UNGROUNDED_VAR)
                    e.type = (PartiallyGroundedNumericExpressionType)MAX_UNSIGNED_INT;
                break;
            case PGE_SUM:
            case PGE_SUB:
            case PGE_DIV:
            case PGE_MUL:
            case PGE_NOT:
                for(unsigned int i = 0; i < exp->terms.size(); i++)
                {
                    PartiallyGroundedNumericExpression t =
                        groundPartiallyGroundedNumericExpression(&(exp->terms[i]), parameters);
                    if(t.type == MAX_UNSIGNED_INT)
                    {
                        e.type = (PartiallyGroundedNumericExpressionType)MAX_UNSIGNED_INT;
                        break;
                    }
                    else
                        e.terms.push_back(t);
                }
                break;
            case PGE_TERM:
                e.isParameter.push_back(false);
                if(exp->isParameter[0])
                    e.paramIndex.push_back((*parameters)[exp->paramIndex[0]]);
                else
                    e.paramIndex.push_back(exp->paramIndex[0]);
                break;
        }
        return e;
    }

    // Grounds and removes the ADL (forall/exists) features in the constraints
    void Grounder::removeADLFeaturesInConstraints()
    {
        for(unsigned int i = 0; i < gTask->task->constraints.size(); i++)
        {
            Constraint *c = &(gTask->task->constraints[i]);
            std::vector<unsigned int> parameters;
            gTask->constraints.push_back(groundConstraint(c, parameters));
        }
        for(unsigned int i = 0; i < gTask->constraints.size(); i++)
            removeADLFeaturesInConstraint(&(gTask->constraints[i]));
        /*
        for (unsigned int i = 0; i < gTask->constraints.size(); i++)
            std::cout << gTask->constraints[i].toString(gTask->task, gTask->variables, gTask->preferenceNames) <<
        std::endl;*/
    }

    // Grounds a constraint
    GroundedConstraint Grounder::groundConstraint(Constraint *c, std::vector<unsigned int> &parameters)
    {
        GroundedConstraint gc;
        gc.type = c->type;
        switch(c->type)
        {
            case RT_AND:
                for(unsigned int i = 0; i < c->terms.size(); i++)
                    gc.terms.push_back(groundConstraint(&(c->terms[i]), parameters));
                break;
            case RT_FORALL:
                for(unsigned int i = 0; i < c->parameters.size(); i++)
                    gc.paramTypes.push_back(c->parameters[i].types);
                for(unsigned int i = 0; i < c->terms.size(); i++)
                    gc.terms.push_back(groundConstraint(&(c->terms[i]), parameters));
                break;
            case RT_GOAL_PREFERENCE:
            case RT_PREFERENCE:
            {
                std::unordered_map<std::string, unsigned int>::const_iterator it =
                    preferenceIndex.find(c->preferenceName);
                if(it == preferenceIndex.end())
                {
                    gc.preferenceIndex                 = gTask->preferenceNames.size();
                    preferenceIndex[c->preferenceName] = gc.preferenceIndex;
                    gTask->preferenceNames.push_back(c->preferenceName);
                }
                else
                {
                    gc.preferenceIndex = it->second;
                }
                if(c->type == RT_PREFERENCE)
                    gc.terms.push_back(groundConstraint(&(c->terms[0]), parameters));
                else
                    gc.goal.push_back(groundGoalDescription(c->goal[0], parameters));
            }
            break;
            default:
                for(unsigned int i = 0; i < c->time.size(); i++)
                    gc.time.push_back(c->time[i]);
                for(unsigned int i = 0; i < c->goal.size(); i++)
                    gc.goal.push_back(groundGoalDescription(c->goal[i], parameters));
                break;
        }
        return gc;
    }

    // Removes the ADL (forall/exists) features in a constraint
    void Grounder::removeADLFeaturesInConstraint(GroundedConstraint *c)
    {
        switch(c->type)
        {
            case RT_AND:
                for(unsigned int i = 0; i < c->terms.size(); i++)
                    removeADLFeaturesInConstraint(&(c->terms[i]));
                break;
            case RT_FORALL:
            {
                c->type = RT_AND;
                std::unordered_map<unsigned int, unsigned int> parameters;  // forall parameter -> problem object
                GroundedConstraint condition = c->terms[0];
                c->terms.clear();
                replaceADLConstraint(c, 0, 0, &parameters, &condition);
            }
            break;
            default:
            {
                for(unsigned int i = 0; i < c->terms.size(); i++)
                    removeADLFeaturesInConstraint(&(c->terms[i]));
                std::vector<GroundedGoalDescription> newGoals = c->goal;
                c->goal.clear();
                for(unsigned int i = 0; i < newGoals.size(); i++)
                {
                    GroundedGoalDescription g;
                    g.type = GG_AND;
                    std::unordered_map<unsigned int, unsigned int> parameters;  // forall parameter -> problem object
                    replaceADLPreference(&g, 0, 0, &parameters, &(newGoals[i]));
                    if(g.terms.size() > 0)
                    {
                        if(g.terms.size() == 1)
                            c->goal.push_back(g.terms[0]);
                        else
                            c->goal.push_back(g);
                    }
                }
            }
            break;
        }
    }

    // Removes the ADL (forall/exists) features in a constraint
    void Grounder::replaceADLConstraint(GroundedConstraint *c,
                                        unsigned int numParam,
                                        unsigned int prevParams,
                                        std::unordered_map<unsigned int, unsigned int> *parameters,
                                        GroundedConstraint *condition)
    {
        if(numParam >= c->paramTypes.size())
        {
            GroundedConstraint newCond = groundConstraint(condition, numParam + prevParams, parameters);
            if(newCond.type != MAX_UNSIGNED_INT)
            {
                c->terms.push_back(newCond);
            }
        }
        else
        {
            for(unsigned int i = 0; i < gTask->task->objects.size(); i++)
            {
                if(gTask->task->compatibleTypes(gTask->task->objects[i].types, c->paramTypes[numParam]))
                {
                    (*parameters)[numParam + prevParams] = i;
                    replaceADLConstraint(c, numParam + 1, prevParams, parameters, condition);
                }
            }
        }
    }

    // Removes the ADL (forall/exists) features in a constraint
    GroundedConstraint Grounder::groundConstraint(GroundedConstraint *condition,
                                                  unsigned int numParam,
                                                  std::unordered_map<unsigned int, unsigned int> *parameters)
    {
        GroundedConstraint c;
        c.type = condition->type;
        switch(condition->type)
        {
            case RT_AND:
                for(unsigned int i = 0; i < condition->terms.size(); i++)
                {
                    GroundedConstraint term = groundConstraint(&(condition->terms[i]), numParam, parameters);
                    if(term.type == MAX_UNSIGNED_INT)
                    {
                        c.terms.clear();
                        c.type = (ConstraintType)MAX_UNSIGNED_INT;
                        break;
                    }
                    else
                        c.terms.push_back(term);
                }
                if(c.terms.size() == 1)
                    c = c.terms[0];
                break;
            case RT_FORALL:
            {
                c.type                          = RT_AND;
                GroundedConstraint adlCondition = condition->terms[0];
                replaceADLConstraint(&c, 0, numParam, parameters, &adlCondition);
                if(c.terms.size() == 0 || c.terms[0].type == MAX_UNSIGNED_INT)
                    c.type = (ConstraintType)MAX_UNSIGNED_INT;
                else if(c.terms.size() == 1)
                    c = c.terms[0];
            }
            break;
            default:
            {
                for(unsigned int i = 0; i < condition->terms.size(); i++)
                    c.terms.push_back(groundConstraint(&(condition->terms[i]), numParam, parameters));
                c.preferenceIndex = condition->preferenceIndex;
                for(unsigned int i = 0; i < condition->goal.size(); i++)
                {
                    GroundedGoalDescription g;
                    g.type = GG_AND;
                    replaceADLPreference(&g, 0, 0, parameters, &(condition->goal[i]));
                    if(g.terms.size() > 0)
                    {
                        if(g.terms.size() == 1)
                            c.goal.push_back(g.terms[0]);
                        else
                            c.goal.push_back(g);
                    }
                }
                c.time       = condition->time;
                c.paramTypes = condition->paramTypes;
                break;
            }
        }
        return c;
    }

    // Grounds the problem metric
    GroundedMetric Grounder::groundMetric(Metric *m)
    {
        GroundedMetric gm;
        gm.type = m->type;
        switch(m->type)
        {
            case MT_NUMBER:
                gm.value = m->value;
                break;
            case MT_PLUS:
            case MT_MINUS:
            case MT_PROD:
            case MT_DIV:
                for(unsigned int i = 0; i < m->terms.size(); i++)
                    gm.terms.push_back(groundMetric(&(m->terms[i])));
                break;
            case MT_IS_VIOLATED:
                gm.index = preferenceIndex[m->preferenceName];
                break;
            case MT_FLUENT:
                gm.index = variableIndex[getVariableName(m->function, m->parameters)];
                break;
            case MT_TOTAL_TIME:;
        }
        return gm;
    }

    // Checks if there are numeric conditions that can already be evaluated
    void Grounder::checkNumericConditions()
    {
        unsigned int i = 0;
        while(i < gTask->actions.size())
        {
            GroundedAction *a = &(gTask->actions[i]);
            bool removeAction = false;
            unsigned int j    = 0;
            while(j < a->startNumCond.size())
            {
                int checking = checkNumericCondition(&(a->startNumCond[j]));
                if(checking == -1)
                    removeAction = true;  // Condition can be evaluated and does not hold
                else if(checking == 1)
                    a->startNumCond.erase(a->startNumCond.begin() + j);  // Condition can be evaluated and holds
                else
                    j++;
            }
            j = 0;
            while(j < a->overNumCond.size())
            {
                int checking = checkNumericCondition(&(a->overNumCond[j]));
                if(checking == -1)
                    removeAction = true;  // Condition can be evaluated and does not hold
                else if(checking == 1)
                    a->overNumCond.erase(a->overNumCond.begin() + j);  // Condition can be evaluated and holds
                else
                    j++;
            }
            j = 0;
            while(j < a->endNumCond.size())
            {
                int checking = checkNumericCondition(&(a->endNumCond[j]));
                if(checking == -1)
                    removeAction = true;  // Condition can be evaluated and does not hold
                else if(checking == 1)
                    a->endNumCond.erase(a->endNumCond.begin() + j);  // Condition can be evaluated and holds
                else
                    j++;
            }
            if(removeAction)
                gTask->actions.erase(gTask->actions.begin() + i);
            else
                a->index = i++;
        }
    }

    // Checks if a numeric condition can be evaluated. Returns 0 if it is not possible.
    // Otherwise, returns 1 if the condition holds or -1 otherwise.
    int Grounder::checkNumericCondition(GroundedNumericCondition *c)
    {
        if(c->terms.size() != 2)
            return 0;
        GroundedNumericExpression *e1 = &(c->terms[0]);
        if(e1->type != GE_NUMBER)
            return 0;
        GroundedNumericExpression *e2 = &(c->terms[1]);
        if(e2->type != GE_NUMBER)
            return 0;
        if(numericComparisonHolds(*c))
            return 1;
        else
            return -1;
    }
}  // namespace grstaps
