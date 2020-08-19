#ifndef GRSTAPS_PREPROCESSED_TASK_HPP
#define GRSTAPS_PREPROCESSED_TASK_HPP

#include "grstaps/task_planning/parsed_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class OpEquality
    {
       public:
        bool equal;
        Term value1;
        Term value2;
        std::string toString(const std::vector<Object> &objects);
    };

    class OpFluent
    {
       public:
        Literal variable;
        Term value;
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
        inline std::string getVarName(unsigned int paramValues[])
        {
            unsigned int index;
            std::string name(std::to_string(variable.fncIndex));
            for(unsigned int i = 0; i < variable.params.size(); i++)
            {
                index = variable.params[i].index;
                if(variable.params[i].isVariable)
                {
                    if(paramValues[index] == MAX_UNSIGNED_INT)
                        name += " ?" + std::to_string(index);
                    else
                        name += " " + std::to_string(paramValues[index]);
                }
                else
                    name += " " + std::to_string(index);
            }
            return name;
        }
        inline std::string getValueName(unsigned int paramValues[])
        {
            if(value.isVariable)
            {
                if(paramValues[value.index] == MAX_UNSIGNED_INT)
                    return "?" + std::to_string(value.index);
                else
                    return std::to_string(paramValues[value.index]);
            }
            else
                return std::to_string(value.index);
        }
    };

    class OpPreference
    {
       public:
        std::string name;
        GoalDescription preference;
        std::string toString(const std::vector<Variable> &opParameters, ParsedTask *task);
    };

    enum OpEffectExpressionType
    {
        OEET_NUMBER          = 0,
        OEET_DURATION        = 1,
        OEET_SHARP_T         = 2,
        OEET_SHARP_T_PRODUCT = 3,
        OEET_FLUENT          = 4,
        OEET_TERM            = 5,
        OEET_SUM             = 6,
        OEET_SUB             = 7,
        OEET_DIV             = 8,
        OEET_MUL             = 9
    };

    class OpEffectExpression
    {
       private:
        static OpEffectExpressionType getOperator(OperationType op)
        {
            switch(op)
            {
                case OT_SUM:
                    return OEET_SUM;
                case OT_SUB:
                    return OEET_SUB;
                case OT_DIV:
                    return OEET_DIV;
                default:
                    return OEET_MUL;
            }
        }
        static OpEffectExpressionType getNumOperator(NumericExpressionType op)
        {
            switch(op)
            {
                case NET_SUM:
                    return OEET_SUM;
                case NET_SUB:
                    return OEET_SUB;
                case NET_DIV:
                    return OEET_DIV;
                default:
                    return OEET_MUL;
            }
        }

       public:
        OpEffectExpressionType type;
        float value;  // if type == OEET_NUMBER
        std::vector<OpEffectExpression>
            operands;    // if type == OEET_SHARP_T_PRODUCT | OEET_SUM | OEET_SUB | OEET_DIV | OEET_MUL
        Literal fluent;  // if type == OEET_FLUENT
        Term term;       // if type == OEET_TERM
        OpEffectExpression(EffectExpression &exp);
        OpEffectExpression(ContinuousEffect &exp);
        OpEffectExpression(NumericExpression &exp);
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
    };

    class OpEffect
    {
       public:
        Assignment assignment;
        Literal fluent;
        OpEffectExpression exp;
        OpEffect(FluentAssignment &f);
        OpEffect(AssignmentContinuousEffect &f);
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
    };

    class OpNumericPrec
    {
       public:
        Comparator comparator;
        std::vector<OpEffectExpression> operands;
        OpNumericPrec();
        OpNumericPrec(GoalDescription &goal);
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
    };

    class OpCondition
    {
       public:
        std::vector<OpFluent> prec;
        std::vector<OpNumericPrec> numericPrec;
        std::vector<OpFluent> eff;
        std::vector<OpEffect> numericEff;
    };

    class Operator
    {
       public:
        std::string name;
        std::vector<Variable> parameters;
        std::vector<Duration> duration;
        OpCondition atStart;
        OpCondition atEnd;
        std::vector<OpFluent> overAllPrec;
        std::vector<OpNumericPrec> overAllNumericPrec;
        std::vector<OpEquality> equality;
        std::vector<OpPreference> preference;
        bool isGoal;
        void addLiteralToPreconditions(Literal var, Term &value, TimeSpecifier time);
        void addEquality(Term &v1, Term &v2, bool equal);
        void addNumericPrecondition(GoalDescription goal, TimeSpecifier time);
        void addLiteralToEffects(Literal var, Term &value, TimeSpecifier time);
        void addNumericEffect(OpEffect eff, TimeSpecifier time);
        std::string toString(ParsedTask *task);
    };

    class PreprocessedTask
    {
       private:
       public:
        ParsedTask *task;
        std::vector<Operator> operators;
        PreprocessedTask(ParsedTask *parsedTask);
        ~PreprocessedTask();
        std::string toString();
    };
}  // namespace grstaps
#endif  // GRSTAPS_PREPROCESSED_TASK_HPP
