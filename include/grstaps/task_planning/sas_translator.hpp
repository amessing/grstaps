#ifndef SAS_TRANSLATOR_H
#define SAS_TRANSLATOR_H

#include "grstaps/task_planning/grounded_task.hpp"
#include "grstaps/task_planning/mutex_graph.hpp"
#include "grstaps/task_planning/sas_task.hpp"

namespace grstaps
{
    class LiteralTranslation
    {
       public:
        unsigned int* numericVariables;                    // New indexes for numeric variables
        unsigned int* sasVariables;                        // Already defined SAS variables
        std::vector<std::vector<unsigned int> > literals;  // literal -> <sasVariable, sasValue>
        LiteralTranslation(unsigned int numVars);
        ~LiteralTranslation();
    };

    class SASTranslator
    {
       private:
        GroundedTask* gTask;
        bool** mutex;
        bool* actions;
        bool* isLiteral;
        bool* literalInFNA;
        bool* literalInF;
        bool* negatedLiteral;
        bool negatedPrecs;
        unsigned int numNewLiterals;
        unsigned int numVars;
        unsigned int numActions;
        std::unordered_map<unsigned long long, bool> mutexChanges;

        void getInitialStateLiterals();
        void clearMemory();
        void checkAction(GroundedAction* a);
        bool holdsCondition(const GroundedCondition* c, std::vector<unsigned int>* preconditions);
        void computeMutex(GroundedAction* a, const std::vector<unsigned int> preconditions, unsigned int startEndPrec);
        void splitMutex(SASTask* sTask, bool onlyGenerateMutex);
        inline static int findInVector(unsigned int value, const std::vector<unsigned int>* v)
        {
            for(unsigned int i = 0; i < v->size(); i++)
            {
                if((*v)[i] == value)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }
        inline static int literalInAtStartAdd(unsigned int value,
                                              std::vector<unsigned int>* add,
                                              unsigned int statAddEndEff)
        {
            for(unsigned int i = 0; i < statAddEndEff; i++)
            {
                if((*add)[i] == value)
                {
                    return (int)i;
                }
            }
            return -1;
        }
        inline unsigned long long mutexIndex(unsigned int v1, unsigned int v2)
        {
            if(v1 > v2)
            {
                unsigned int aux = v1;
                v1               = v2;
                v2               = aux;
            }
            return (((unsigned long long)v1) << 32) + v2;
        }
        inline void addMutex(unsigned int v1, unsigned int v2)
        {
            if(!mutex[v1][v2])
            {
                mutex[v1][v2]                                                    = true;
                mutex[v2][v1]                                                    = true;
                unsigned long long code                                          = mutexIndex(v1, v2);
                std::unordered_map<unsigned long long, bool>::const_iterator got = mutexChanges.find(code);
                if(got == mutexChanges.end() || !got->second)
                {
                    mutexChanges[code] = true;
                }
            }
        }
        inline void deleteMutex(unsigned int v1, unsigned int v2)
        {
            if(mutex[v1][v2])
            {
                mutex[v1][v2]                                                    = false;
                mutex[v2][v1]                                                    = false;
                unsigned long long code                                          = mutexIndex(v1, v2);
                std::unordered_map<unsigned long long, bool>::const_iterator got = mutexChanges.find(code);
                if(got == mutexChanges.end() || got->second)
                {
                    mutexChanges[code] = false;
                }
            }
        }
        void updateDomain(SASTask* sTask, MutexGraph* graph, LiteralTranslation* trans);
        void simplifyDomain(SASTask* sTask, LiteralTranslation* trans);
        void removeMultipleValues(SASTask* sTask, LiteralTranslation* trans);
        void createNumericAndFiniteDomainVariables(SASTask* sTask, LiteralTranslation* trans);
        void setInitialValuesForVariables(SASTask* sTask, LiteralTranslation* trans);
        void createAction(GroundedAction* ga, SASTask* sTask, LiteralTranslation* trans, bool isGoal);
        void generateDuration(SASAction* a, GroundedDuration* gd, LiteralTranslation* trans);
        char generateComparator(Comparator comp);
        char generateTime(TimeSpecifier time);
        SASNumericExpression generateNumericExpression(GroundedNumericExpression* gn, LiteralTranslation* trans);
        SASNumericExpression generateNumericExpression(PartiallyGroundedNumericExpression* gn,
                                                       LiteralTranslation* trans);
        char generateNumericExpressionType(int type);
        char generatePartiallyNumericExpressionType(int type);
        void generateCondition(GroundedCondition* cond,
                               SASTask* sTask,
                               LiteralTranslation* trans,
                               std::vector<SASCondition>* conditionSet);
        void generateEffect(std::vector<GroundedCondition>* effects,
                            unsigned int effIndex,
                            SASTask* sTask,
                            LiteralTranslation* trans,
                            std::vector<SASCondition>* conditionSet);
        bool modifiedVariable(unsigned int sasVar,
                              std::vector<GroundedCondition>* effects,
                              unsigned int effIndex,
                              LiteralTranslation* trans);
        void checkModifiedVariable(SASCondition* c, SASAction* a);
        void checkNegatedPreconditionLiterals(GroundedAction* a);
        SASNumericCondition generateNumericCondition(GroundedNumericCondition* cond, LiteralTranslation* trans);
        SASNumericEffect generateNumericEffect(GroundedNumericEffect* cond, LiteralTranslation* trans);
        char generateAssignment(Assignment assignment);
        SASPreference generatePreference(GroundedPreference* pref, SASTask* sTask, LiteralTranslation* trans);
        SASGoalDescription generateGoalDescription(GroundedGoalDescription* gd,
                                                   SASTask* sTask,
                                                   LiteralTranslation* trans);
        SASConstraint createConstraint(GroundedConstraint* gc, SASTask* sTask, LiteralTranslation* trans);
        SASMetric createMetric(GroundedMetric* metric, LiteralTranslation* trans);
        void writeMutexFile();
        void removeActionsWithMutexConditions();
        bool hasMutexConditions(GroundedAction* a);
        bool isMutex(GroundedCondition& c1, GroundedCondition& c2);
        void translateMutex(SASTask* sTask, LiteralTranslation* trans);

       public:
        SASTask* translate(GroundedTask* gTask, bool onlyGenerateMutex, bool generateMutexFile, bool keepStaticData);
    };
}  // namespace grstaps
#endif
