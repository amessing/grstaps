#ifndef GRSTAPS_SAS_TASK_HPP
#define GRSTAPS_SAS_TASK_HPP

#include <cstdint>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
#define FICTITIOUS_FUNCTION        999999U

    class SASValue
    {
    public:
        unsigned int index;
        unsigned int fncIndex;
        std::string name;

        std::string toString();
    };

    class SASVariable
    {
    public:
        unsigned int index;
        std::string name;
        std::vector<unsigned int> possibleValues;
        std::vector<unsigned int> value;
        std::vector<float> time;

        void addPossibleValue(unsigned int value);

        void addInitialValue(unsigned int sasValue, bool isTrue, float timePoint);

        std::string toString();

        std::string toString(std::vector<SASValue>& values);

        std::string toStringInitialState(std::vector<SASValue>& values);

        unsigned int getOppositeValue(unsigned int v);

        TVarValue getInitialStateValue();

        int getPossibleValueIndex(unsigned int value);
    };

    class NumericVariable
    {
    public:
        unsigned int index;
        std::string name;
        std::vector<float> value;
        std::vector<float> time;

        void addInitialValue(float value, float time);

        std::string toString();

        std::string toStringInitialState();

        float getInitialStateValue();
    };

    class SASNumericExpression
    {
    public:
        char type;                    // 'N' = GE_NUMBER, 'V' = GE_VAR, '+' = GE_SUM, '-' = GE_SUB, '/' = GE_DIV, '*' = GE_MUL, 'D' = GE_DURATION, '#' = GE_SHARP_T
        float value;                                // if type == 'N'
        uint16_t var;
        std::vector<SASNumericExpression> terms;    // if type == '+' | '-' | '*' | '/' | '#'
        std::string toString(std::vector<NumericVariable>* numVariables);
    };

    class SASDuration
    {
    public:
        char time;                    // 'S' = AT_START, 'E' = AT_END, 'A' = OVER_ALL, 'N' = NONE
        char comp;                    // '=' = CMP_EQ, '<' = CMP_LESS, 'L' = CMP_LESS_EQ, '>' = CMP_GREATER, 'G' = CMP_GREATER_EQ, 'N' = CMP_NEQ
        SASNumericExpression exp;

        std::string toString(std::vector<NumericVariable>* numVariables);
    };

    class SASCondition
    {
    public:
        unsigned int var;
        unsigned int value;
        bool isModified;

        SASCondition(unsigned int var, unsigned int value);
    };

    class SASNumericCondition
    {
    public:
        char comp;                    // '=' = CMP_EQ, '<' = CMP_LESS, 'L' = CMP_LESS_EQ, '>' = CMP_GREATER, 'G' = CMP_GREATER_EQ, 'N' = CMP_NEQ
        std::vector<SASNumericExpression> terms;

        std::string toString(std::vector<NumericVariable>* numVariables);
    };

    class SASNumericEffect
    {
    public:
        char op;                    // '=' = AS_ASSIGN, '+' = AS_INCREASE, '-' = AS_DECREASE, '*' = AS_SCALE_UP, '/' = AS_SCALE_DOWN
        unsigned int var;
        SASNumericExpression exp;

        std::string toString(std::vector<NumericVariable>* numVariables);
    };

    class SASGoalDescription
    {
    public:
        char time;                    // 'S' = AT_START, 'E' = AT_END, 'A' = OVER_ALL, 'N' = NONE
        char type;                    // 'V' = (= var value), '&' = AND, '|' = OR, '!' = NOT, COMPARISON (see comp in SASNumericCondition)
        unsigned int var, value;                // (= var value)
        std::vector<SASGoalDescription> terms;    // AND, OR, NOT
        std::vector<SASNumericExpression> exp;    // Comparison
    };

    class SASPreference
    {
    public:
        unsigned int index;
        SASGoalDescription preference;
    };

    class SASAction
    {
    public:
        unsigned int index;
        std::string name;
        std::vector<SASDuration> duration;
        std::vector<SASCondition> startCond;
        std::vector<SASCondition> endCond;
        std::vector<SASCondition> overCond;
        std::vector<SASNumericCondition> startNumCond;
        std::vector<SASNumericCondition> overNumCond;
        std::vector<SASNumericCondition> endNumCond;
        std::vector<SASCondition> startEff;
        std::vector<SASCondition> endEff;
        std::vector<SASNumericEffect> startNumEff;
        std::vector<SASNumericEffect> endNumEff;
        std::vector<SASPreference> preferences;
        bool isGoal;
        bool isTIL;
        bool fixedDuration;                        // True if the duration of the action does not depend on the state
        std::vector<float> fixedDurationValue;    // Fixed duration of the action (only if fixedDuration)
        bool fixedCost;                            // True if the cost of the action does not depend on the state
        float fixedCostValue;                    // Fixed cost of the action according to the metric (only if fixedCost)

        void setGoalCost()
        {                    // Sets the cost and duration of a ficttious goal action
            fixedDuration = true;
            fixedDurationValue.push_back(EPSILON);
            fixedCost = true;
            fixedCostValue = 0;
        }
    };

    class SASConstraint
    {
    public:
        char type;        // '&' = RT_AND, 'P' = RT_PREFERENCE, 'G' = RT_GOAL_PREFERENCE, 'E' = RT_AT_END, 'A' = RT_ALWAYS = 4, 'S' = RT_SOMETIME, 'W' = RT_WITHIN
        // 'O' = RT_AT_MOST_ONCE, 'F' = RT_SOMETIME_AFTER, 'B' = RT_SOMETIME_BEFORE, 'T' = RT_ALWAYS_WITHIN, 'D' = RT_HOLD_DURING, 'H' = RT_HOLD_AFTER
        std::vector<SASConstraint> terms;                // '&', 'P'
        unsigned int preferenceIndex;                // 'P', 'G'
        std::vector<SASGoalDescription> goal;            // 'G' | 'E' | 'A' | 'S' | 'W' | 'O' | 'F' | 'B' | 'T' | 'D' | 'H'
        std::vector<float> time;                        // 'W' | 'T' | 'D' | 'H'
    };

    class SASMetric
    {
    public:
        char type;        // '+' = MT_PLUS, '-' = MT_MINUS, '*' = MT_PROD, '/' = MT_DIV, 'N' = MT_NUMBER, 'T' = MT_TOTAL_TIME, 'V' = MT_IS_VIOLATED, 'F' = MT_FLUENT
        float value;                    // 'N'
        unsigned int index;                // 'V' | 'F'
        std::vector<SASMetric> terms;    // '+' | '-' | '*' | '/'
    };

    class GoalDeadline
    {
    public:
        float time;
        std::vector<TVarValue> goals;
    };

    class SASTask
    {
    private:
        std::unordered_map<TMutex, bool> mutex;
        std::unordered_map<TVarValue, std::vector<TVarValue>*> mutexWithVarValue;
        std::unordered_map<TMutex, bool> permanentMutex;
        std::unordered_map<TMutex, bool> permanentMutexActions;
        std::unordered_map<std::string, unsigned int> valuesByName;
        std::vector<TVarValue> goalList;
        bool* staticNumFunctions;
        std::vector<GoalDeadline> goalDeadlines;

        inline static TMutex getMutexCode(TVariable var1, TValue value1, TVariable var2, TValue value2)
        {
            TMutex code = (var1 << 16) + value1;
            code = (code << 16) + var2;
            code = (code << 16) + value2;
            return code;
        }

        void computeActionCost(SASAction* a, bool* variablesOnMetric);

        bool checkVariablesUsedInMetric(SASMetric* m, bool* variablesOnMetric);

        bool checkVariableExpression(SASNumericExpression* e, bool* variablesOnMetric);

        float computeFixedExpression(SASNumericExpression* e);

        float evaluateMetric(SASMetric* m, float* numState, float makespan);

        void updateNumericState(float* s, SASNumericEffect* e, float duration);

        bool checkActionMutex(SASAction* a1, SASAction* a2);

        bool checkActionOrdering(SASAction* a1, SASAction* a2);

        void computeMutexWithVarValues();

        void checkReachability(TVarValue vv, std::unordered_map<TVarValue, bool>* goals);

        void checkEffectReached(SASCondition* c,
                                std::unordered_map<TVarValue, bool>* goals,
                                std::unordered_map<TVarValue, bool>* visitedVarValue,
                                std::vector<TVarValue>* state);

        void addGoalToList(SASCondition* c);

    public:
        static const unsigned int OBJECT_TRUE = 0;
        static const unsigned int OBJECT_FALSE = 1;
        static const unsigned int OBJECT_UNDEFINED = 2;
        std::vector<SASVariable> variables;
        std::vector<SASValue> values;
        std::vector<NumericVariable> numVariables;
        std::vector<SASAction> actions;
        std::vector<std::string> preferenceNames;
        std::vector<SASAction> goals;
        std::vector<SASConstraint> constraints;
        char metricType;                                // '>' = Maximize, '<' = Minimize , 'X' = no metric specified
        SASMetric metric;
        bool metricDependsOnDuration;                    // True if the metric depends on the plan duration
        std::vector<SASAction*>** requirers;
        std::vector<SASAction*>** producers;
        std::vector<SASAction*> actionsWithoutConditions;
        TValue* initialState;                            // Values of the SAS variables in the initial state
        float* numInitialState;                            // Values of the numeric variables in the initial state
        bool variableCosts;                                // True if there are actions with a cost that depends on the state
        int numGoalsInPlateau;
        char domainType;
        bool tilActions;

        SASTask();

        ~SASTask();

        void addMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2);

        bool isMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2);

        bool isPermanentMutex(unsigned int var1, unsigned int value1, unsigned int var2, unsigned int value2);

        bool isPermanentMutex(SASAction* a1, SASAction* a2);

        SASVariable* createNewVariable();

        SASVariable* createNewVariable(std::string name);

        inline static TVarValue getVariableValueCode(unsigned int var, unsigned int value)
        {
            return (var << 16) + value;
        }

        inline static TVariable getVariableIndex(TVarValue code)
        {
            return code >> 16;
        }

        inline static TValue getValueIndex(TVarValue code)
        {
            return code & 0xFFFF;
        }

        unsigned int createNewValue(std::string name, unsigned int fncIndex);

        unsigned int findOrCreateNewValue(std::string name, unsigned int fncIndex);

        unsigned int getValueByName(const std::string& name);

        NumericVariable* createNewNumericVariable(std::string name);

        SASAction* createNewAction(std::string name);

        SASAction* createNewGoal();

        void computeInitialState();

        void computeRequirers();

        void computeProducers();

        void computePermanentMutex();

        void addToRequirers(TVariable v, TValue val, SASAction* a);

        void addToProducers(TVariable v, TValue val, SASAction* a);

        void computeInitialActionsCost(bool keepStaticData);

        float computeActionCost(SASAction* a, float* numState, float makespan);

        float evaluateNumericExpression(const SASNumericExpression* e, float* s, float duration) const;

        float getActionDuration(SASAction* a, float* s) const;

        bool holdsNumericCondition(SASNumericCondition& cond, float* s, float duration) const;

        inline float evaluateMetric(float* numState, float makespan)
        {
            return evaluateMetric(&metric, numState, makespan);
        }

        inline bool hasPermanentMutexAction() const
        {
            return !permanentMutexActions.empty();
        }

        std::vector<TVarValue>* getListOfGoals();

        std::string toString();

        inline static std::string toStringTime(char time)
        {
            switch(time)
            {
                case 'S':
                    return "at start";
                case 'E':
                    return "at end";
                case 'A':
                    return "over all";
                default:
                    return "";
            }
        }

        inline static std::string toStringComparator(char cmp)
        {
            switch(cmp)
            {
                case '=':
                    return "=";
                case '<':
                    return "<";
                case 'L':
                    return "<=";
                case '>':
                    return ">";
                case 'G':
                    return ">=";
                default:
                    return "!=";
            }
        }

        inline static std::string toStringAssignment(char op)
        {
            switch(op)
            {
                case '=':
                    return "assign";
                case '+':
                    return "increase";
                case '-':
                    return "decrease";
                case '*':
                    return "scale-up";
                default:
                    return "scale-down";
            }
        }

        std::string toStringCondition(SASCondition& c);

        std::string toStringPreference(SASPreference* pref);

        std::string toStringGoalDescription(SASGoalDescription* g);

        std::string toStringConstraint(SASConstraint* c);

        std::string toStringMetric(SASMetric* m);

        std::string toStringAction(SASAction& a);

        void addGoalDeadline(float time, TVarValue goal);

        inline bool areGoalDeadlines()
        { return !goalDeadlines.empty(); }

        inline std::vector<GoalDeadline>* getGoalDeadlines()
        { return &goalDeadlines; };
    };
}
#endif //GRSTAPS_SAS_TASK_HPP
