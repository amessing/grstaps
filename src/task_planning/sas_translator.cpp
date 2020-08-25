/********************************************************/
/* Translates the PDDL literals (boolean fluents) into  */
/* a set of object fluents.                             */
/********************************************************/

//#define DEBUG_SASTRANS_ON

#include <assert.h>
#include <iostream>

#include "grstaps/task_planning/parsed_task.hpp"
#include "grstaps/task_planning/sas_translator.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: LiteralTranslation                            */
    /********************************************************/

    LiteralTranslation::LiteralTranslation(unsigned int numVars)
    {
        numericVariables = new unsigned int[numVars];
        sasVariables     = new unsigned int[numVars];
        literals.resize(numVars);
    }

    LiteralTranslation::~LiteralTranslation()
    {
        delete[] numericVariables;
        delete[] sasVariables;
    }

    /********************************************************/
    /* CLASS: SASTranslator                                 */
    /********************************************************/

    SASTask* SASTranslator::translate(GroundedTask* gTask,
                                      bool onlyGenerateMutex,
                                      bool generateMutexFile,
                                      bool keepStaticData)
    {
        this->gTask = gTask;
        numVars     = gTask->variables.size();
        numActions  = gTask->actions.size();
        getInitialStateLiterals();
        mutex = new bool*[numVars];
        for(unsigned int i = 0; i < numVars; i++)
            mutex[i] = new bool[numVars]{false};
        actions = new bool[numActions]{false};

        literalInFNA = new bool[numVars];
        for(unsigned int i = 0; i < numVars; i++)
            literalInFNA[i] = literalInF[i];

        while(numNewLiterals > 0 || mutexChanges.size() > 0)
        {
#ifdef DEBUG_SASTRANS_ON
            std::cout << "-----------------------------------" << std::endl << "4. F = {";
            for(unsigned int i = 0; i < numVars; i++)
            {
                if(literalInF[i])
                {
                    std::cout << gTask->variables[i].toString(gTask->task);
                }
            }
            std::cout << "}" << std::endl << "4. M = {";
            for(unsigned int i = 0; i < numVars; i++)
            {
                for(unsigned int j = i + 1; j < numVars; j++)
                {
                    if(mutex[i][j])
                        std::cout << "<" << gTask->variables[i].toString(gTask->task) << ","
                                  << gTask->variables[j].toString(gTask->task) << ">";
                }
            }
            std::cout << "}" << std::endl;
#endif
            mutexChanges.clear();
            numNewLiterals = 0;
            for(unsigned int i = 0; i < numActions; i++)
            {
                checkAction(&(gTask->actions[i]));
            }

            for(unsigned int i = 0; i < numVars; i++)
                literalInFNA[i] = literalInF[i];
        }
        delete[] literalInFNA;
        if(generateMutexFile)
        {
            writeMutexFile();
        }
        removeActionsWithMutexConditions();
        SASTask* sTask = new SASTask();
        splitMutex(sTask, onlyGenerateMutex);
        clearMemory();
        sTask->computeInitialState();
        sTask->computeRequirers();
        sTask->computeProducers();
        sTask->computePermanentMutex();
#ifdef DEBUG_SASTRANS_ON
        std::cout << sTask->toString() << std::endl;
#endif
        sTask->computeInitialActionsCost(keepStaticData);
        return sTask;
    }

    // Removes invalid actions (with mutex in their conditions)
    void SASTranslator::removeActionsWithMutexConditions()
    {
        unsigned int i = 0;
        while(i < gTask->actions.size())
        {
            GroundedAction* a = &(gTask->actions[i]);
            if(hasMutexConditions(a))
            {
#ifdef DEBUG_SASTRANS_ON
                std::cout << "Action " << a->getName(gTask->task) << " removed by mutex conditions" << std::endl;
#endif
                gTask->actions.erase(gTask->actions.begin() + i);
            }
            else
            {
                a->index = i++;
            }
        }
        numActions = gTask->actions.size();
    }

    // Checks if a given action has mutex conditions
    bool SASTranslator::hasMutexConditions(GroundedAction* a)
    {
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            for(unsigned int j = i + 1; j < a->startCond.size(); j++)
            {
                if(isMutex(a->startCond[i], a->startCond[j]))
                    return true;
            }
        }
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            for(unsigned int j = i + 1; j < a->endCond.size(); j++)
            {
                if(isMutex(a->endCond[i], a->endCond[j]))
                    return true;
            }
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            for(unsigned int j = i + 1; j < a->overCond.size(); j++)
            {
                if(isMutex(a->overCond[i], a->overCond[j]))
                    return true;
            }
            for(unsigned int j = 0; j < a->startCond.size(); j++)
            {
                if(isMutex(a->overCond[i], a->startCond[j]))
                    return true;
            }
            for(unsigned int j = 0; j < a->endCond.size(); j++)
            {
                if(isMutex(a->overCond[i], a->endCond[j]))
                    return true;
            }
        }
        return false;
    }

    // Checks if two action conditions are mutex
    bool SASTranslator::isMutex(GroundedCondition& c1, GroundedCondition& c2)
    {
        if(isLiteral[c1.varIndex])
        {  // c1 is a literal
            if(c1.valueIndex == gTask->task->CONSTANT_FALSE)
            {  // c1 is a negated literal
                if(!isLiteral[c2.varIndex] || c2.valueIndex == gTask->task->CONSTANT_FALSE)
                    return false;
                return c1.varIndex == c2.varIndex;  // c2 = not c1 -> mutex
            }
            else
            {  // c1 is non-negated literal
                if(!isLiteral[c2.varIndex])
                    return false;
                if(c2.valueIndex == gTask->task->CONSTANT_FALSE)
                    return c1.varIndex == c2.varIndex;  // c2 = not c1 -> mutex
                return mutex[c1.varIndex][c2.varIndex];
            }
        }
        else
        {  // c1 is not a literal
            if(isLiteral[c2.varIndex])
                return false;
            return c1.varIndex == c2.varIndex && c1.valueIndex != c2.valueIndex;
        }
    }

    // Disposes the memory
    void SASTranslator::clearMemory()
    {
        for(unsigned int i = 0; i < numVars; i++)
            delete[] mutex[i];
        delete[] mutex;
        delete[] literalInF;
        delete[] isLiteral;
        delete[] actions;
    }

    // F* <- I
    void SASTranslator::getInitialStateLiterals()
    {
        literalInF     = new bool[numVars]{false};
        isLiteral      = new bool[numVars]{false};
        numNewLiterals = 0;
        for(unsigned int i = 0; i < numVars; i++)
        {
            GroundedVar& v = gTask->variables[i];
            if(gTask->task->isBooleanFunction(v.fncIndex))
            {
                isLiteral[i] = true;
                bool holds   = false;
                for(unsigned int j = 0; j < v.initialValues.size(); j++)
                {
                    if(v.initialValues[j].value == gTask->task->CONSTANT_TRUE)
                    {
                        holds = true;
                        break;
                    }
                }
                if(holds)
                {
                    numNewLiterals++;
                    literalInF[i] = true;
                }
            }
        }
    }

    // Checks id the given action generates new mutex
    void SASTranslator::checkAction(GroundedAction* a)
    {
        std::vector<unsigned int> preconditions;
        unsigned int startEndPrec;
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            if(!holdsCondition(&(a->startCond[i]), &preconditions))
                return;
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            if(!holdsCondition(&(a->overCond[i]), &preconditions))
                return;
        }
        startEndPrec = preconditions.size();
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            if(!holdsCondition(&(a->endCond[i]), &preconditions))
                return;
        }
        unsigned int psize = preconditions.size() > 0 ? preconditions.size() - 1 : 0;
        for(unsigned int p = 0; p < psize; p++)
        {
            for(unsigned int q = p + 1; q < preconditions.size(); q++)
            {
                if(mutex[p][q])
                    return;
            }
        }
        computeMutex(a, preconditions, startEndPrec);
    }

    // Checks if the conditions is a literal that holds in F*
    bool SASTranslator::holdsCondition(const GroundedCondition* c, std::vector<unsigned int>* preconditions)
    {
        if(!isLiteral[c->varIndex])
            return true;  // It's not a literal
        if(c->valueIndex == gTask->task->CONSTANT_FALSE)
            return true;  // Negated literal
        preconditions->push_back(c->varIndex);
        return literalInFNA[c->varIndex];
    }

    // Computes the new mutex that this action generates
    void SASTranslator::computeMutex(GroundedAction* a,
                                     const std::vector<unsigned int> preconditions,
                                     unsigned int startEndPrec)
    {
#ifdef DEBUG_SASTRANS_ON
        std::cout << "5. a = " << a->getName(gTask->task) << std::endl;
#endif
        std::vector<unsigned int> newA, add, del;
        unsigned int varIndex, statAddEndEff, startDelEndEff, startNewEndEff;
        for(unsigned int i = 0; i < a->startEff.size(); i++)
        {  // New(a) <- Add(a) - F*
            varIndex = a->startEff[i].varIndex;
            if(isLiteral[varIndex])
            {
                if(a->startEff[i].valueIndex == gTask->task->CONSTANT_TRUE)
                {
                    add.push_back(varIndex);
                    if(!literalInF[varIndex])
                        newA.push_back(varIndex);
                }
                else
                    del.push_back(varIndex);
            }
        }
        statAddEndEff  = add.size();
        startDelEndEff = del.size();
        startNewEndEff = newA.size();
        for(unsigned int i = 0; i < a->endEff.size(); i++)
        {
            varIndex = a->endEff[i].varIndex;
            if(isLiteral[varIndex])
            {
                if(a->endEff[i].valueIndex == gTask->task->CONSTANT_TRUE)
                {
                    add.push_back(varIndex);
                    if(!literalInF[varIndex])
                        newA.push_back(varIndex);
                }
                else
                    del.push_back(varIndex);
            }
        }

#ifdef DEBUG_SASTRANS_ON
        std::cout << "6.  | New(a) = {";
        for(unsigned int i = 0; i < newA.size(); i++)
        {
            std::cout << gTask->variables[newA[i]].toString(gTask->task);
        }
        std::cout << "}" << std::endl;
#endif

        for(unsigned int f = 0; f < newA.size(); f++)
        {  // forall f in New(a)
#ifdef DEBUG_SASTRANS_ON
            std::cout << "7.  |  | f = " << gTask->variables[newA[f]].toString(gTask->task) << std::endl;
#endif
            for(unsigned int h = 0; h < del.size(); h++)
            {  // forall h in Del(a)
#ifdef DEBUG_SASTRANS_ON
                std::cout << "8.  |  |  | h = " << gTask->variables[del[h]].toString(gTask->task) << std::endl;
#endif
                if(f >= startNewEndEff || h < startDelEndEff)
                {  // f is at-end or h is at-start
#ifdef DEBUG_SASTRANS_ON
                    std::cout << "9.  |  |  |  | time(f)<>at-start or time(h)<>at-end" << std::endl;
#endif
                    if(findInVector(del[h], &preconditions) != -1 ||
                       literalInAtStartAdd(del[h], &add, statAddEndEff) != -1)
                    {  // h in Pre(a) or h in AtStartAdd(a) -> condition added
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "10. |  |  |  |  | h is in Pre(a) or h in AtStartAdd(a)" << std::endl;
#endif
                        addMutex(newA[f], del[h]);
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "11. |  |  |  |  |  | M* <- M* U {"
                                  << gTask->variables[newA[f]].toString(gTask->task) << ","
                                  << gTask->variables[del[h]].toString(gTask->task) << "}" << std::endl;
#endif
                    }
                }
            }
            for(unsigned int p = 0; p < preconditions.size(); p++)
            {  // p in Pre(a)
                for(unsigned int q = 0; q < numVars; q++)
                {  // (p,q) in M* / q not in Del(a)
#ifdef DEBUG_SASTRANS_ON
                    if(isLiteral[q] && q != newA[f] && mutex[preconditions[p]][q] && findInVector(q, &del) == -1)
                    {
                        std::cout << "12. |  |  | p = " << gTask->variables[preconditions[p]].toString(gTask->task)
                                  << " is in Pre(a)" << std::endl;
                        std::cout << "    |  |  | q = " << gTask->variables[q].toString(gTask->task) << " not in Del(a)"
                                  << std::endl;
                    }
#endif
                    if(isLiteral[q] && q != newA[f] && mutex[preconditions[p]][q] &&
                       (p < startEndPrec || f >= startNewEndEff) &&  // p is at-start or over-all, or f is at-end
                       findInVector(q, &del) == -1)
                    {
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "13. |  |  | (p, q) in M* and (time(p)<>at-end or time(f)<>at-start)" << std::endl;
                        std::cout << "14. |  |  |  | M* <- M* U {" << gTask->variables[newA[f]].toString(gTask->task)
                                  << "," << gTask->variables[q].toString(gTask->task) << "}" << std::endl;
#endif
                        addMutex(newA[f], q);
                        // std::cout << "* Mutex2: " << gTask->variables[newA[f]].toString(gTask->task) <<
                        //     " and " << gTask->variables[q].toString(gTask->task) << " (from " <<
                        //	 gTask->variables[preconditions[p]].toString(gTask->task) << ")" << std::endl;
                    }
                }
            }
        }
        if(!actions[a->index])
        {  // a in A
#ifdef DEBUG_SASTRANS_ON
            std::cout << "15. |  | a in A" << std::endl;
#endif
            unsigned int addSize = statAddEndEff > 0 ? statAddEndEff - 1 : statAddEndEff;
            for(unsigned int p = 0; p <= addSize; p++)
            {  // p,q in Add(a) / (p,q) in M*
                for(unsigned int q = p + 1; q < statAddEndEff; q++)
                {
                    if(mutex[add[p]][add[q]])
                    {
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "16. |  |  | p = " << gTask->variables[add[p]].toString(gTask->task)
                                  << " in Add(a) and time(p)=at-start" << std::endl;
                        std::cout << "    |  |  | q = " << gTask->variables[add[q]].toString(gTask->task)
                                  << " in Add(a) and time(q)=at-start" << std::endl;
                        std::cout << "    |  |  | (p, q) in M*" << std::endl;
                        std::cout << "18. |  |  |  | M* <- M* - {" << gTask->variables[add[p]].toString(gTask->task)
                                  << "," << gTask->variables[add[q]].toString(gTask->task) << "}" << std::endl;
#endif
                        deleteMutex(add[p], add[q]);
                    }
                }
                for(unsigned int q = statAddEndEff; q < add.size(); q++)
                {
                    if(mutex[add[p]][add[q]])
                    {
                        if(mutex[add[p]][add[q]] && findInVector(add[p], &del) == -1)
                        {
#ifdef DEBUG_SASTRANS_ON
                            std::cout << "16. |  |  | p = " << gTask->variables[add[p]].toString(gTask->task)
                                      << " in Add(a) and time(p)=at-start" << std::endl;
                            std::cout << "    |  |  | q = " << gTask->variables[add[q]].toString(gTask->task)
                                      << " in Add(a) and time(q)=at-end" << std::endl;
                            std::cout << "    |  |  | (p, q) in M* and p not in Del(a)" << std::endl;
                            std::cout << "18. |  |  |  | M* <- M* - {" << gTask->variables[add[p]].toString(gTask->task)
                                      << "," << gTask->variables[add[q]].toString(gTask->task) << "}" << std::endl;
#endif
                            deleteMutex(add[p], add[q]);
                        }
                    }
                }
            }
            addSize = add.size() > 0 ? add.size() - 1 : add.size();
            for(unsigned int p = statAddEndEff; p < addSize; p++)  // p,q in Add(a) / (p,q) in M*
                for(unsigned int q = p + 1; q < add.size(); q++)
                {
                    if(mutex[add[p]][add[q]])
                    {
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "16. |  |  | p = " << gTask->variables[add[p]].toString(gTask->task)
                                  << " in Add(a) and time(p)=at-end" << std::endl;
                        std::cout << "    |  |  | q = " << gTask->variables[add[q]].toString(gTask->task)
                                  << " in Add(a) and time(q)=at-end" << std::endl;
                        std::cout << "    |  |  | (p, q) in M*" << std::endl;
                        std::cout << "18. |  |  |  | M* <- M* - {" << gTask->variables[add[p]].toString(gTask->task)
                                  << "," << gTask->variables[add[q]].toString(gTask->task) << "}" << std::endl;
#endif
                        deleteMutex(add[p], add[q]);
                    }
                }
        }
#ifdef DEBUG_SASTRANS_ON
        std::cout << "19. | L = {";
        for(unsigned int i = 0; i < add.size(); i++)
        {
            if(findInVector(add[i], &newA) == -1)
            {
                std::cout << gTask->variables[add[i]].toString(gTask->task);
            }
        }
        std::cout << "}" << std::endl;
#endif
        for(unsigned int i = 0; i < add.size(); i++)
        {  // L <- Add(a) - New(a)
            if(findInVector(add[i], &newA) == -1)
            {  // i in L
                for(unsigned int q = 0; q < numVars; q++)
                {
#ifdef DEBUG_SASTRANS_ON
                    if(isLiteral[q] && mutex[add[i]][q])
                    {
                        std::cout << "19. |  | i = " << gTask->variables[add[i]].toString(gTask->task) << " in L"
                                  << std::endl;
                        std::cout << "    |  | q = " << gTask->variables[q].toString(gTask->task) << ", (i,q) in M*"
                                  << std::endl;
                    }
#endif
                    if(isLiteral[q] && mutex[add[i]][q] &&  // (i,q) in M*
                       findInVector(q, &del) == -1)
                    {  // q not in Del(a)
#ifdef DEBUG_SASTRANS_ON
                        std::cout << "21. |  | q not in Pre(a)" << std::endl;
#endif
                        bool existsP = false;  // not exits p in Pre(a) / (p,q) in M*
                        for(unsigned p = 0; p < preconditions.size(); p++)
                        {
                            if(mutex[preconditions[p]][q])
                            {
#ifdef DEBUG_SASTRANS_ON
                                std::cout
                                    << "    |  | q "
                                    << " is mutex with p = " << gTask->variables[preconditions[p]].toString(gTask->task)
                                    << " in Pre(a)" << std::endl;
#endif
                                existsP = true;
                                break;
                            }
                        }
                        if(!existsP)
                        {
#ifdef DEBUG_SASTRANS_ON
                            std::cout << "    |  | not(exists p in Pre(a) / (p,q) in M*)" << std::endl;
                            std::cout << "22. |  |  | M* <- M* - {" << gTask->variables[add[i]].toString(gTask->task)
                                      << "," << gTask->variables[q].toString(gTask->task) << "}" << std::endl;
#endif
                            deleteMutex(add[i], q);
                        }
                    }
                }
            }
        }
        for(unsigned int i = 0; i < newA.size(); i++)
        {  // F* <- F* U New(a)
            literalInF[newA[i]] = true;
            numNewLiterals++;
#ifdef DEBUG_SASTRANS_ON
            std::cout << "23. | F* <- F* U {" << gTask->variables[newA[i]].toString(gTask->task) << "}" << std::endl;
#endif
        }
        actions[a->index] = true;  // A <- A U {a}
#ifdef DEBUG_SASTRANS_ON
        std::cout << "24. | A <- A U {" << a->getName(gTask->task) << "}" << std::endl;
#endif
    }

    // Makes partitions to divide the graph into different subsets of mutually exclusive literals
    void SASTranslator::splitMutex(SASTask* sTask, bool onlyGenerateMutex)
    {
        unsigned int numLiterals = 0;
        MutexGraph graph;  // Build the mutex graph
        for(unsigned int i = 0; i < numVars; i++)
        {
            if(isLiteral[i])
            {
                numLiterals++;
                graph.addVertex(i);
            }
        }
        for(unsigned int i = 0; i < numVars; i++)
        {
            if(isLiteral[i])
            {
                for(unsigned int j = 0; j < numVars; j++)
                {
                    if(mutex[i][j] && isLiteral[j])
                    {
                        graph.addAdjacent(i, j);
                        if(onlyGenerateMutex)
                            sTask->addMutex(i, gTask->task->CONSTANT_TRUE, j, gTask->task->CONSTANT_TRUE);
                    }
                }
            }
        }
        negatedPrecs   = false;  // Check if there are negative preconditions
        negatedLiteral = new bool[numVars];
        for(unsigned int i = 0; i < numVars; i++)
            negatedLiteral[i] = false;
        for(unsigned int i = 0; i < numActions; i++)
            checkNegatedPreconditionLiterals(&(gTask->actions[i]));
        LiteralTranslation trans(numVars);  // Object for the traslation of literals to SAS variables
        createNumericAndFiniteDomainVariables(sTask, &trans);
        if(!onlyGenerateMutex)
        {
            graph.split();  // Split the graph into finite-domain variables
            updateDomain(sTask, &graph, &trans);
        }
        else
        {
            simplifyDomain(sTask, &trans);
        }
        removeMultipleValues(sTask, &trans);
        setInitialValuesForVariables(sTask, &trans);  // Initial state processing
        sTask->preferenceNames = gTask->preferenceNames;
        for(unsigned int i = 0; i < numActions; i++)  // Actions processing
            createAction(&(gTask->actions[i]), sTask, &trans, false);
        for(unsigned int i = 0; i < gTask->goals.size(); i++)  // Goals processing
            createAction(&(gTask->goals[i]), sTask, &trans, true);
        for(unsigned int i = 0; i < gTask->constraints.size(); i++)  // Constraints processing
            sTask->constraints.push_back(createConstraint(&(gTask->constraints[i]), sTask, &trans));
        sTask->metricType = gTask->metricType;  // Metric processing
        if(sTask->metricType != 'X')
            sTask->metric = createMetric(&(gTask->metric), &trans);
        translateMutex(sTask, &trans);  // Mutex processing
        delete[] negatedLiteral;
    }

    void SASTranslator::translateMutex(SASTask* sTask, LiteralTranslation* trans)
    {
        TVariable* sasVars = new TVariable[numVars];
        TValue* sasValues  = new TValue[numVars];
        for(unsigned int i = 0; i < numVars; i++)
        {
            if(trans->literals[i].size() == 1)
            {
                unsigned int code = trans->literals[i][0];
                sasVars[i]        = SASTask::getVariableIndex(code);
                sasValues[i]      = SASTask::getValueIndex(code);
                // std::cout << gTask->variables[i].toString(gTask->task) << " -> (" <<
                // sTask->variables[sasVars[i]].name <<
                // "=" << sTask->values[sasValues[i]].name << ")" << std::endl;
            }
            else
            {
                sasVars[i] = MAX_UINT16;
            }
        }
        for(unsigned int i = 0; i < numVars; i++)
        {
            for(unsigned int j = i + 1; j < numVars; j++)
            {
                if(mutex[i][j] && sasVars[i] != MAX_UINT16 && sasVars[j] != MAX_UINT16)
                {
                    sTask->addMutex(sasVars[i], sasValues[i], sasVars[j], sasValues[j]);
                }
            }
        }
        delete[] sasVars;
        delete[] sasValues;
    }

    void SASTranslator::removeMultipleValues(SASTask* sTask, LiteralTranslation* trans)
    {
        for(unsigned int i = 0; i < numVars; i++)
        {
            unsigned int size = trans->literals[i].size();
            if(size > 1)
            {
                // std::cout << "MULTIPLE VAR. FOR LITERAL: " << std::endl;
                for(unsigned int j = 1; j < size; j++)
                {
                    unsigned int code     = trans->literals[i][j];
                    unsigned int sasVar   = SASTask::getVariableIndex(code);
                    unsigned int sasValue = SASTask::getValueIndex(code);
                    // std::cout << " * " << sasVar << " = " << sasValue << std::endl;
                    SASVariable& v     = sTask->variables[sasVar];
                    int valueIndex     = v.getPossibleValueIndex(sasValue);
                    int undefinedIndex = v.getPossibleValueIndex(SASTask::OBJECT_UNDEFINED);
                    if(undefinedIndex == -1)
                    {
                        v.possibleValues[valueIndex] = SASTask::OBJECT_UNDEFINED;
                    }
                    else
                    {
                        v.possibleValues.erase(v.possibleValues.begin() + valueIndex);
                    }
                }
                trans->literals[i].erase(trans->literals[i].begin() + 1, trans->literals[i].end());
            }
        }
    }

    // Changes the variable indexes in the metric
    SASMetric SASTranslator::createMetric(GroundedMetric* metric, LiteralTranslation* trans)
    {
        SASMetric m;
        switch(metric->type)
        {
            case MT_NUMBER:
                m.type  = 'N';
                m.value = metric->value;
                break;
            case MT_TOTAL_TIME:
                m.type = 'T';
                break;
            case MT_IS_VIOLATED:
                m.type  = 'V';
                m.index = metric->index;
                break;
            case MT_FLUENT:
                m.type  = 'F';
                m.index = trans->numericVariables[metric->index];
                break;
            default:
                if(metric->type == MT_PLUS)
                    m.type = '+';
                else if(metric->type == MT_MINUS)
                    m.type = '-';
                else if(metric->type == MT_PROD)
                    m.type = '*';
                else
                    m.type = '/';
                for(unsigned int i = 0; i < metric->terms.size(); i++)
                    m.terms.push_back(createMetric(&(metric->terms[i]), trans));
                break;
        }
        return m;
    }

    // Checks the literals which are negated preconditions in the action
    void SASTranslator::checkNegatedPreconditionLiterals(GroundedAction* a)
    {
        unsigned int falseValue = gTask->task->CONSTANT_FALSE;
        for(unsigned int i = 0; i < a->startCond.size(); i++)
        {
            if(a->startCond[i].valueIndex == falseValue)
            {
                negatedLiteral[a->startCond[i].varIndex] = true;
                negatedPrecs                             = true;
            }
        }
        for(unsigned int i = 0; i < a->overCond.size(); i++)
        {
            if(a->overCond[i].valueIndex == falseValue)
            {
                negatedLiteral[a->overCond[i].varIndex] = true;
                negatedPrecs                            = true;
            }
        }
        for(unsigned int i = 0; i < a->endCond.size(); i++)
        {
            if(a->endCond[i].valueIndex == falseValue)
            {
                negatedLiteral[a->endCond[i].varIndex] = true;
                negatedPrecs                           = true;
            }
        }
    }

    // Changes the literals in the domain by the computed finite-domain variables
    void SASTranslator::updateDomain(SASTask* sTask, MutexGraph* graph, LiteralTranslation* trans)
    {
        std::vector<unsigned int> values;
        for(unsigned int i = 0; i < graph->numVariables(); i++)
        {
            graph->getVariable(i, values, MAX_UNSIGNED_INT);
            SASVariable* v;
            /*
            std::cout << "VAR " << i << std::endl;
            for (unsigned int j = 0; j < values.size(); j++)
               std::cout << "* " << gTask->variables[values[j]].toString(gTask->task) << std::endl;
            */
            if(values.size() == 1)
            {  // Single literal -> two values: true or false
                v = sTask->createNewVariable(gTask->variables[values[0]].toString(gTask->task));
                v->addPossibleValue(SASTask::OBJECT_FALSE);
                v->addPossibleValue(SASTask::OBJECT_TRUE);
                trans->literals[values[0]].push_back(SASTask::getVariableValueCode(v->index, SASTask::OBJECT_TRUE));
            }
            else
            {  // New variable with different values
                bool keepGroup = true;
                if(negatedPrecs)
                {
                    for(unsigned int j = 0; j < values.size(); j++)
                        if(negatedLiteral[values[j]])
                        {
                            keepGroup = false;
                            break;
                        }
                }
                if(keepGroup)
                {  // Keep the mutex group of literals as a new single SAS variable
                    v = sTask->createNewVariable();
                    for(unsigned int j = 0; j < values.size(); j++)
                    {
                        GroundedVar& gv       = gTask->variables[values[j]];
                        unsigned int sasValue = sTask->createNewValue(gv.toString(gTask->task), gv.fncIndex);
                        v->addPossibleValue(sasValue);
                        trans->literals[values[j]].push_back(SASTask::getVariableValueCode(v->index, sasValue));
                    }
                }
                else
                {  // Replace each literal in the mutex group by a new boolean SAS variable (if it doesn't exist yet)
                    for(unsigned int j = 0; j < values.size(); j++)
                    {
                        if(trans->literals[values[j]].size() == 0)
                        {  // Literal not added yet
                            v = sTask->createNewVariable(gTask->variables[values[j]].toString(gTask->task));
                            v->addPossibleValue(SASTask::OBJECT_FALSE);
                            v->addPossibleValue(SASTask::OBJECT_TRUE);
                            trans->literals[values[j]].push_back(
                                SASTask::getVariableValueCode(v->index, SASTask::OBJECT_TRUE));
                        }
                    }
                }
            }
        }
    }

    // Simplifies the data structures to store variables and actions
    void SASTranslator::simplifyDomain(SASTask* sTask, LiteralTranslation* trans)
    {
        sTask->values.clear();
        unsigned int size = gTask->task->objects.size();  // Copy the parsed objects to SAS values
        for(unsigned int i = 0; i < size; i++)
        {
            std::cout << "NEW VALUE: " << gTask->task->objects[i].name << std::endl;
            sTask->createNewValue(gTask->task->objects[i].name, FICTITIOUS_FUNCTION);
        }
        for(unsigned int i = 0; i < numVars; i++)
        {
            GroundedVar& gv = gTask->variables[i];
            if(gv.isNumeric)
                break;
            if(gTask->task->isBooleanFunction(gv.fncIndex))
            {
                SASVariable* v = sTask->createNewVariable(gv.toString(gTask->task));
                v->addPossibleValue(SASTask::OBJECT_FALSE);
                v->addPossibleValue(SASTask::OBJECT_TRUE);
                // std::cout << i << " -> <" << v->index << "," << SASTask::OBJECT_TRUE << ">" << std::endl;
                trans->literals[i].push_back(SASTask::getVariableValueCode(v->index, SASTask::OBJECT_TRUE));
            }
        }
    }

    // Copies the numeric and (already) finite-domain variables
    void SASTranslator::createNumericAndFiniteDomainVariables(SASTask* sTask, LiteralTranslation* trans)
    {
        std::vector<Object>& objects = gTask->task->objects;
        for(unsigned int i = 0; i < gTask->variables.size(); i++)
        {
            GroundedVar& gv = gTask->variables[i];
            if(gv.isNumeric)
            {  // Numeric variable
                NumericVariable* v         = sTask->createNewNumericVariable(gv.toString(gTask->task));
                trans->numericVariables[i] = v->index;
            }
            else if(!gTask->task->isBooleanFunction(gv.fncIndex))
            {  // Already finite-domain variable
                SASVariable* v                        = sTask->createNewVariable(gv.toString(gTask->task));
                std::vector<unsigned int>& valueTypes = gTask->task->functions[gv.fncIndex].valueTypes;
                for(unsigned int j = 0; j < objects.size(); j++)
                {
                    if(gTask->task->compatibleTypes(objects[j].types, valueTypes))
                    {
                        unsigned int sasValue = sTask->findOrCreateNewValue(objects[j].name, FICTITIOUS_FUNCTION);
                        v->addPossibleValue(sasValue);
                    }
                }
                trans->sasVariables[i] = v->index;
            }
        }
    }

    // Sets the values of the variables in the initial state (and in the TILs)
    void SASTranslator::setInitialValuesForVariables(SASTask* sTask, LiteralTranslation* trans)
    {
        for(unsigned int i = 0; i < gTask->variables.size(); i++)
        {
            GroundedVar& gv = gTask->variables[i];
            if(gv.isNumeric)
            {  // Numeric variable
                NumericVariable& v = sTask->numVariables[trans->numericVariables[i]];
                for(unsigned int j = 0; j < gv.initialValues.size(); j++)
                {
                    v.addInitialValue(gv.initialValues[j].numericValue, gv.initialValues[j].time);
                }
            }
            else if(gTask->task->isBooleanFunction(gv.fncIndex))
            {  // Literal
                for(unsigned int k = 0; k < trans->literals[i].size(); k++)
                {
                    unsigned int code  = trans->literals[i][k];
                    unsigned int var   = SASTask::getVariableIndex(code);
                    unsigned int value = SASTask::getValueIndex(code);
                    SASVariable& v     = sTask->variables[var];
                    for(unsigned int j = 0; j < gv.initialValues.size(); j++)
                    {
                        v.addInitialValue(
                            value, gv.initialValues[j].value == gTask->task->CONSTANT_TRUE, gv.initialValues[j].time);
                    }
                }
            }
            else
            {  // Already finite-domain variable
                unsigned int var = trans->sasVariables[i];
                SASVariable& v   = sTask->variables[var];
                for(unsigned int j = 0; j < gv.initialValues.size(); j++)
                {
                    unsigned int value = sTask->getValueByName(gTask->task->objects[gv.initialValues[j].value].name);
                    v.addInitialValue(value, true, gv.initialValues[j].time);
                }
            }
        }
        for(unsigned int i = 0; i < sTask->variables.size(); i++)
        {
            SASVariable& v       = sTask->variables[i];
            bool hasInitialValue = false;
            for(unsigned int j = 0; j < v.time.size(); j++)
            {
                if(v.time[j] == 0)
                {
                    hasInitialValue = true;
                    break;
                }
            }
            if(!hasInitialValue)
            {
                if(v.possibleValues.size() == 2 && v.possibleValues[0] == SASTask::OBJECT_FALSE &&
                   v.possibleValues[1] == SASTask::OBJECT_TRUE)
                {
                    v.addInitialValue(SASTask::OBJECT_TRUE, false, 0);
                }
                else
                {
                    if(findInVector(SASTask::OBJECT_UNDEFINED, &v.possibleValues) == -1)
                    {
                        v.addPossibleValue(SASTask::OBJECT_UNDEFINED);
                    }
                    v.addInitialValue(SASTask::OBJECT_UNDEFINED, true, 0);
                }
            }
        }
    }

    // Copies the grounded action, replacing the literals by the SAS variables
    void SASTranslator::createAction(GroundedAction* ga, SASTask* sTask, LiteralTranslation* trans, bool isGoal)
    {
        SASAction* a = isGoal ? sTask->createNewGoal() : sTask->createNewAction(ga->getName(gTask->task));
        for(unsigned int i = 0; i < ga->duration.size(); i++)
            generateDuration(a, &(ga->duration[i]), trans);
        for(unsigned int i = 0; i < ga->startCond.size(); i++)
            generateCondition(&(ga->startCond[i]), sTask, trans, &(a->startCond));
        for(unsigned int i = 0; i < ga->endCond.size(); i++)
            generateCondition(&(ga->endCond[i]), sTask, trans, &(a->endCond));
        for(unsigned int i = 0; i < ga->overCond.size(); i++)
            generateCondition(&(ga->overCond[i]), sTask, trans, &(a->overCond));
        for(unsigned int i = 0; i < ga->startEff.size(); i++)
            generateEffect(&(ga->startEff), i, sTask, trans, &(a->startEff));
        for(unsigned int i = 0; i < ga->endEff.size(); i++)
            generateEffect(&(ga->endEff), i, sTask, trans, &(a->endEff));

        for(unsigned int i = 0; i < a->startCond.size(); i++)
            checkModifiedVariable(&(a->startCond[i]), a);
        for(unsigned int i = 0; i < a->overCond.size(); i++)
            checkModifiedVariable(&(a->overCond[i]), a);
        for(unsigned int i = 0; i < a->endCond.size(); i++)
            checkModifiedVariable(&(a->endCond[i]), a);

        for(unsigned int i = 0; i < ga->startNumCond.size(); i++)
            a->startNumCond.push_back(generateNumericCondition(&(ga->startNumCond[i]), trans));
        for(unsigned int i = 0; i < ga->overNumCond.size(); i++)
            a->overNumCond.push_back(generateNumericCondition(&(ga->overNumCond[i]), trans));
        for(unsigned int i = 0; i < ga->endNumCond.size(); i++)
            a->endNumCond.push_back(generateNumericCondition(&(ga->endNumCond[i]), trans));
        for(unsigned int i = 0; i < ga->startNumEff.size(); i++)
            a->startNumEff.push_back(generateNumericEffect(&(ga->startNumEff[i]), trans));
        for(unsigned int i = 0; i < ga->endNumEff.size(); i++)
            a->endNumEff.push_back(generateNumericEffect(&(ga->endNumEff[i]), trans));
        for(unsigned int i = 0; i < ga->preferences.size(); i++)
            a->preferences.push_back(generatePreference(&(ga->preferences[i]), sTask, trans));
    }

    // Checks if a precondition is mofified by the efects
    void SASTranslator::checkModifiedVariable(SASCondition* c, SASAction* a)
    {
        for(unsigned int i = 0; i < a->endEff.size(); i++)
            if(c->var == a->endEff[i].var && c->value != a->endEff[i].value)
            {
                c->isModified = true;
                break;
            }
        for(unsigned int i = 0; i < a->startEff.size(); i++)
            if(c->var == a->startEff[i].var && c->value != a->startEff[i].value)
            {
                c->isModified = true;
                break;
            }
    }

    // Copies the action duration, replacing the literals by the SAS variables
    void SASTranslator::generateDuration(SASAction* a, GroundedDuration* gd, LiteralTranslation* trans)
    {
        SASDuration d;
        d.time = generateTime(gd->time);
        d.comp = generateComparator(gd->comp);
        d.exp  = generateNumericExpression(&(gd->exp), trans);
        a->duration.push_back(d);
    }

    // Replaces the comparator by a char code
    char SASTranslator::generateComparator(Comparator comp)
    {
        char c;
        switch(comp)
        {
            case Comparator::k_eq:
                c = '=';
                break;
            case Comparator::k_less:
                c = '<';
                break;
            case Comparator::k_less_eq:
                c = 'L';
                break;
            case Comparator::k_greater:
                c = '>';
                break;
            case Comparator::k_greater_eq:
                c = 'G';
                break;
            default:
                c = 'N';
                break;
        }
        return c;
    }

    // Replaces the time specifier by a char code
    char SASTranslator::generateTime(TimeSpecifier time)
    {
        char c;
        switch(time)
        {
            case TimeSpecifier::k_at_start:
                c = 'S';
                break;
            case TimeSpecifier::k_at_end:
                c = 'E';
                break;
            case TimeSpecifier::k_over_all:
                c = 'A';
                break;
            default:
                c = 'N';
                break;
        }
        return c;
    }

    // Copies the numeric expression, replacing the literals by the SAS variables
    SASNumericExpression SASTranslator::generateNumericExpression(GroundedNumericExpression* gn,
                                                                  LiteralTranslation* trans)
    {
        SASNumericExpression e;
        e.type = generateNumericExpressionType(gn->type);
        switch(e.type)
        {
            case 'N':
                e.value = gn->value;
                break;
            case 'V':
                e.var = trans->numericVariables[gn->index];
                break;
            case '+':
            case '-':
            case '/':
            case '*':
            case '#':
                for(unsigned int i = 0; i < gn->terms.size(); i++)
                    e.terms.push_back(generateNumericExpression(&(gn->terms[i]), trans));
                break;
        }
        return e;
    }

    // Copies the numeric expression, replacing the literals by the SAS variables
    SASNumericExpression SASTranslator::generateNumericExpression(PartiallyGroundedNumericExpression* gn,
                                                                  LiteralTranslation* trans)
    {
        SASNumericExpression e;
        e.type = generatePartiallyNumericExpressionType(gn->type);
        switch(e.type)
        {
            case 'N':
                e.value = gn->value;
                break;
            case 'V':
                e.var = trans->numericVariables[gn->index];
                break;
            case '+':
            case '-':
            case '/':
            case '*':
                for(unsigned int i = 0; i < gn->terms.size(); i++)
                    e.terms.push_back(generateNumericExpression(&(gn->terms[i]), trans));
                break;
        }
        return e;
    }

    // Replaces the numeric expression type by a char code
    char SASTranslator::generateNumericExpressionType(int type)
    {
        char c;
        switch(type)
        {
            case GE_NUMBER:
                c = 'N';
                break;
            case GE_VAR:
                c = 'V';
                break;
            case GE_SUM:
                c = '+';
                break;
            case GE_SUB:
                c = '-';
                break;
            case GE_DIV:
                c = '/';
                break;
            case GE_MUL:
                c = '*';
                break;
            case GE_DURATION:
                c = 'D';
                break;
            case GE_SHARP_T:
                c = '#';
                break;
            default:
                std::cout << "Invalid numeric expression type" << std::endl;
                assert(false);
                c = '\0';
        }
        return c;
    }

    // Replaces the numeric expression type by a char code
    char SASTranslator::generatePartiallyNumericExpressionType(int type)
    {
        char c;
        switch(type)
        {
            case PGE_NUMBER:
                c = 'N';
                break;
            case PGE_VAR:
                c = 'V';
                break;
            case PGE_SUM:
                c = '+';
                break;
            case PGE_SUB:
                c = '-';
                break;
            case PGE_DIV:
                c = '/';
                break;
            case PGE_MUL:
                c = '*';
                break;
            default:
                std::cout << "Invalid numeric expression type" << std::endl;
                assert(false);
                c = '\0';
        }
        return c;
    }

    // Copies the condtion, replacing the literals by the SAS variables
    void SASTranslator::generateCondition(GroundedCondition* cond,
                                          SASTask* sTask,
                                          LiteralTranslation* trans,
                                          std::vector<SASCondition>* conditionSet)
    {
        unsigned int sasVar, sasValue, code;
        if(trans->literals[cond->varIndex].size() > 0)
        {  // Literal converted to one value of one or more SAS variables
            bool truePrec = cond->valueIndex == gTask->task->CONSTANT_TRUE;
            for(unsigned int i = 0; i < trans->literals[cond->varIndex].size(); i++)
            {
                code     = trans->literals[cond->varIndex][i];
                sasVar   = SASTask::getVariableIndex(code);
                sasValue = SASTask::getValueIndex(code);
                if(truePrec)
                {
                    conditionSet->emplace_back(sasVar, sasValue);
                }
                else
                {  // Negated literal precondtion
                    conditionSet->emplace_back(sasVar, sTask->variables[sasVar].getOppositeValue(sasValue));
                }
            }
        }
        else
        {  // Already defined SAS variable
            sasVar   = trans->sasVariables[cond->varIndex];
            sasValue = sTask->getValueByName(gTask->task->objects[cond->valueIndex].name);
            conditionSet->emplace_back(sasVar, sasValue);
        }
    }

    // Copies the effect, replacing the literals by the SAS variables
    void SASTranslator::generateEffect(std::vector<GroundedCondition>* effects,
                                       unsigned int effIndex,
                                       SASTask* sTask,
                                       LiteralTranslation* trans,
                                       std::vector<SASCondition>* conditionSet)
    {
        unsigned int sasVar, sasValue, code;
        GroundedCondition* cond = &(effects->at(effIndex));
        if(trans->literals[cond->varIndex].size() > 0)
        {  // Literal converted to one value of one or more SAS variables
            bool addEff = cond->valueIndex == gTask->task->CONSTANT_TRUE;
            for(unsigned int i = 0; i < trans->literals[cond->varIndex].size(); i++)
            {
                code     = trans->literals[cond->varIndex][i];
                sasVar   = SASTask::getVariableIndex(code);
                sasValue = SASTask::getValueIndex(code);
                if(addEff)
                {  // Add effect
                    conditionSet->emplace_back(sasVar, sasValue);
                }
                else
                {  // Delete effect
                    if(!modifiedVariable(sasVar, effects, effIndex, trans))
                    {
                        sasValue = SASTask::OBJECT_UNDEFINED;
                        conditionSet->emplace_back(sasVar, sasValue);
                        SASVariable& v = sTask->variables[sasVar];
                        if(findInVector(sasValue, &v.possibleValues) == -1)
                            v.addPossibleValue(sasValue);
                    }
                }
            }
        }
        else
        {  // Already defined SAS variable
            sasVar   = trans->sasVariables[cond->varIndex];
            sasValue = sTask->getValueByName(gTask->task->objects[cond->valueIndex].name);
            conditionSet->emplace_back(sasVar, sasValue);
        }
    }

    // Checks if the variable is modified at the same time (add and delete effects at the same time)
    bool SASTranslator::modifiedVariable(unsigned int sasVar,
                                         std::vector<GroundedCondition>* effects,
                                         unsigned int effIndex,
                                         LiteralTranslation* trans)
    {
        for(unsigned int i = 0; i < effects->size(); i++)
        {
            if(i != effIndex)
            {
                GroundedCondition* cond = &(effects->at(i));
                if(trans->literals[cond->varIndex].size() > 0 && cond->valueIndex == gTask->task->CONSTANT_TRUE)
                {  // Add effect
                    for(unsigned int j = 0; j < trans->literals[cond->varIndex].size(); j++)
                    {
                        unsigned int v = SASTask::getVariableIndex(trans->literals[cond->varIndex][j]);
                        if(v == sasVar)
                            return true;
                    }
                }
            }
        }
        return false;
    }

    // Generates the equivalent SASNumericCondition from the given GroundedNumericCondition
    SASNumericCondition SASTranslator::generateNumericCondition(GroundedNumericCondition* cond,
                                                                LiteralTranslation* trans)
    {
        SASNumericCondition c;
        c.comp = generateComparator(cond->comparator);
        for(unsigned int i = 0; i < cond->terms.size(); i++)
        {
            c.terms.push_back(generateNumericExpression(&(cond->terms[i]), trans));
        }
        return c;
    }

    // Generates the equivalent SASNumericEffect from the given GroundedNumericEffect
    SASNumericEffect SASTranslator::generateNumericEffect(GroundedNumericEffect* eff, LiteralTranslation* trans)
    {
        SASNumericEffect e;
        e.op  = generateAssignment(eff->assignment);
        e.var = trans->numericVariables[eff->varIndex];
        e.exp = generateNumericExpression(&(eff->exp), trans);
        return e;
    }

    // Replaces the numeric assignment by a char code
    char SASTranslator::generateAssignment(Assignment assignment)
    {
        char c;
        switch(assignment)
        {
            case Assignment::AS_ASSIGN:
                c = '=';
                break;
            case Assignment::AS_INCREASE:
                c = '+';
                break;
            case Assignment::AS_DECREASE:
                c = '-';
                break;
            case Assignment::AS_SCALE_UP:
                c = '*';
                break;
            case Assignment::AS_SCALE_DOWN:
                c = '/';
                break;
            default:
                std::cout << "Invalid assignment operator" << std::endl;
                assert(false);
                c = '\0';
        }
        return c;
    }

    // Generates the equivalent SASPreference from the given GroundedPreference
    SASPreference SASTranslator::generatePreference(GroundedPreference* pref, SASTask* sTask, LiteralTranslation* trans)
    {
        SASPreference p;
        p.index      = pref->nameIndex;
        p.preference = generateGoalDescription(&(pref->preference), sTask, trans);
        return p;
    }

    // Generates the equivalent SASGoalDescription from the given GroundedGoalDescription
    SASGoalDescription SASTranslator::generateGoalDescription(GroundedGoalDescription* gd,
                                                              SASTask* sTask,
                                                              LiteralTranslation* trans)
    {
        SASGoalDescription g;
        g.time = generateTime(gd->time);
        switch(gd->type)
        {
            case GG_FLUENT:
                if(!gd->equal)
                {                     // (!= var value) -> (not (= var value))
                    g.type    = '!';  // Not
                    gd->equal = true;
                    g.terms.push_back(generateGoalDescription(gd, sTask, trans));
                    gd->equal = false;
                }
                else
                {
                    g.type = 'V';  // (= var value)
                    if(trans->literals[gd->index].size() > 0)
                    {  // Literal converted to one value of one or more SAS variables
                        unsigned int code =
                            trans->literals[gd->index][0];  // Only the first (variable, value) pair is enough
                        g.var   = SASTask::getVariableIndex(code);
                        g.value = SASTask::getValueIndex(code);
                    }
                    else
                    {  // Already defined SAS variable
                        g.var   = trans->sasVariables[gd->index];
                        g.value = sTask->getValueByName(gTask->task->objects[gd->value].name);
                    }
                }
                break;
            case GG_AND:
            case GG_OR:
                g.type = gd->type == GG_AND ? '&' : '|';
                for(unsigned int i = 0; i < gd->terms.size(); i++)
                {
                    g.terms.push_back(generateGoalDescription(&(gd->terms[i]), sTask, trans));
                }
                break;
            case GG_NOT:
            {
                g.type                  = '!';
                SASGoalDescription term = generateGoalDescription(&(gd->terms[0]), sTask, trans);
                switch(term.type)
                {
                    case 'V':
                    case '&':
                    case '|':
                        g.terms.push_back(term);
                        break;
                    case '!':
                        g = term.terms[0];
                        break;  // Double negation removed
                    default:    // Comparision
                        g = term;
                        switch(term.type)
                        {
                            case '=':
                                g.type = 'N';
                                break;
                            case '<':
                                g.type = 'G';
                                break;
                            case 'L':
                                g.type = '>';
                                break;
                            case '>':
                                g.type = 'L';
                                break;
                            case 'G':
                                g.type = '<';
                                break;
                            case 'N':
                                g.type = '=';
                                break;
                        }
                }
            }
            break;
            case GG_COMP:
                g.type = generateComparator(gd->comparator);
                for(unsigned int i = 0; i < gd->exp.size(); i++)
                {
                    g.exp.push_back(generateNumericExpression(&(gd->exp[i]), trans));
                }
                break;
            default:
                std::cout << "Unexpected goal description type: " << gd->type << std::endl;
                // std::cout << gd->toString(gTask->task, gTask->variables) << std::endl;
                assert(false);
                g.type = '\0';
        }
        return g;
    }

    // Generates the equivalent SASConstraint from the given GroundedConstraint
    SASConstraint SASTranslator::createConstraint(GroundedConstraint* gc, SASTask* sTask, LiteralTranslation* trans)
    {
        SASConstraint c;
        switch(gc->type)
        {
            case RT_AND:
                c.type = '&';
                for(unsigned int i = 0; i < gc->terms.size(); i++)
                    c.terms.push_back(createConstraint(&(gc->terms[i]), sTask, trans));
                break;
            case RT_PREFERENCE:
                c.type            = 'P';
                c.preferenceIndex = gc->preferenceIndex;
                c.terms.push_back(createConstraint(&(gc->terms[0]), sTask, trans));
                break;
            case RT_AT_END:
                c.type = 'E';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_ALWAYS:
                c.type = 'A';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_SOMETIME:
                c.type = 'S';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_WITHIN:
                c.type = 'W';
                c.time.push_back(gc->time[0]);
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_AT_MOST_ONCE:
                c.type = 'O';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_SOMETIME_AFTER:
                c.type = 'F';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                c.goal.push_back(generateGoalDescription(&(gc->goal[1]), sTask, trans));
                break;
            case RT_SOMETIME_BEFORE:
                c.type = 'B';
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                c.goal.push_back(generateGoalDescription(&(gc->goal[1]), sTask, trans));
                break;
            case RT_ALWAYS_WITHIN:
                c.type = 'T';
                c.time.push_back(gc->time[0]);
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                c.goal.push_back(generateGoalDescription(&(gc->goal[1]), sTask, trans));
                break;
            case RT_HOLD_DURING:
                c.type = 'D';
                c.time.push_back(gc->time[0]);
                c.time.push_back(gc->time[1]);
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_HOLD_AFTER:
                c.type = 'H';
                c.time.push_back(gc->time[0]);
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            case RT_GOAL_PREFERENCE:
                c.type            = 'G';
                c.preferenceIndex = gc->preferenceIndex;
                c.goal.push_back(generateGoalDescription(&(gc->goal[0]), sTask, trans));
                break;
            default:
                std::cout << "Unexpected constraint type " << gc->type << std::endl;
                assert(false);
                c.type = '\0';
        }
        return c;
    }

    // Writes the file with the list of static mutex
    void SASTranslator::writeMutexFile()
    {
        ParsedTask* task = gTask->task;
        std::ofstream f;
        f.open("mutex.txt");
        for(unsigned int v1 = 0; v1 < numVars; v1++)
        {
            for(unsigned int v2 = v1 + 1; v2 < numVars; v2++)
            {
                if(mutex[v1][v2])
                {
                    f << gTask->variables[v1].toString(task) << " " << gTask->variables[v2].toString(task) << std::endl;
                }
            }
        }
        f.close();
    }
}  // namespace grstaps
