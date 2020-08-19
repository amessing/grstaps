#ifndef GRSTAPS_PARSED_TASK_HPP
#define GRSTAPS_PARSED_TASK_HPP

#include <string>
#include <vector>

#include "grstaps/task_planning/syntax_analyzer.hpp"
#include <unordered_map>

namespace grstaps
{
    class Type
    {
       public:
        unsigned int index;
        std::string name;
        std::vector<unsigned int> parentTypes;
        Type(unsigned int index, std::string name);
        std::string toString();
    };

    class Object
    {
       public:
        unsigned int index;
        std::string name;
        bool isConstant;
        std::vector<unsigned int> types;
        Object(unsigned int index, std::string name, bool isConstant);
        std::string toString();
    };

    class Variable
    {
       public:
        std::string name;
        std::vector<unsigned int> types;
        Variable(std::string name, const std::vector<unsigned int> &types);
        std::string toString(const std::vector<Type> &taskTypes);
    };

    class Function
    {
       public:
        unsigned int index;
        std::string name;
        std::vector<Variable> parameters;
        std::vector<unsigned int> valueTypes;
        Function();
        Function(std::string name, const std::vector<Variable> &parameters);
        void setValueTypes(const std::vector<unsigned int> &valueTypes);
        std::string toString(const std::vector<Type> &taskTypes);
    };

    class Term
    {
       public:
        bool isVariable;     // true if it is a variable, false if it is a constant
        unsigned int index;  // parameter index if it is a variable, object index otherwise
        Term();
        Term(bool isVariable, unsigned int index);
        std::string toString(const std::vector<Variable> &parameters, const std::vector<Object> &objects);
        inline bool equals(Term &t)
        {
            return isVariable == t.isVariable && index == t.index;
        }
    };

    enum NumericExpressionType
    {
        NET_NUMBER   = 0,
        NET_FUNCTION = 1,
        NET_NEGATION = 2,
        NET_SUM      = 3,
        NET_SUB      = 4,
        NET_DIV      = 5,
        NET_MUL      = 6,
        NET_TERM     = 7
    };

    class Literal
    {
       public:
        unsigned int fncIndex;
        std::vector<Term> params;
        std::string toString(const std::vector<Variable> &parameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
        inline bool equals(Literal &l)
        {
            if(fncIndex != l.fncIndex)
                return false;
            for(unsigned int i = 0; i < params.size(); i++)
                if(!params[i].equals(l.params[i]))
                    return false;
            return true;
        }
    };

    class NumericExpression
    {
       public:
        NumericExpressionType type;
        float value;                              // if type==NET_NUMBER
        Literal function;                         // if type==NET_FUNCTION
        Term term;                                // if type==NET_TERM
        std::vector<NumericExpression> operands;  // otherwise
        NumericExpression();
        NumericExpression(float value);
        NumericExpression(unsigned int fncIndex, const std::vector<Term> &fncParams);
        NumericExpression(Symbol s, const std::vector<NumericExpression> &operands, SyntaxAnalyzer *syn);
        std::string toString(const std::vector<Variable> &parameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    enum class TimeSpecifier
    {
        k_at_start = 0,
        k_at_end   = 1,
        k_over_all = 2,
        k_none     = 3
    };
    std::string toString(TimeSpecifier t);

    enum class Comparator
    {
        k_eq         = 0,
        k_less       = 1,
        k_less_eq    = 2,
        k_greater    = 3,
        k_greater_eq = 4,
        k_neq        = 5
    };
    std::string toString(Comparator c);

    class Duration
    {
       public:
        TimeSpecifier time;
        Comparator comp;
        NumericExpression exp;
        Duration(Symbol s, const NumericExpression &exp);
        std::string toString(const std::vector<Variable> &parameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    enum GoalDescriptionType
    {
        GD_LITERAL     = 0,
        GD_AND         = 1,
        GD_NOT         = 2,
        GD_OR          = 3,
        GD_IMPLY       = 4,
        GD_EXISTS      = 5,
        GD_FORALL      = 6,
        GD_F_CMP       = 7,
        GD_EQUALITY    = 8,
        GD_INEQUALITY  = 9,
        GD_NEG_LITERAL = 10
    };

    class GoalDescription
    {
       public:
        TimeSpecifier time;
        GoalDescriptionType type;
        Literal literal;                     // if type==GD_LITERAL
        std::vector<GoalDescription> terms;  // if type==GD_AND | GD_NOT | GD_OR | GD_IMPLY | GD_EXISTS | GD_FORALL
        std::vector<Variable> parameters;    // if type==GD_EXISTS | GD_FORALL
        Comparator comparator;               // if type==GD_F_CMP
        std::vector<NumericExpression> exp;  // if type==GD_F_CMP
        std::vector<Term> eqTerms;           // if type==GD_EQUALITY | GD_INEQUALITY
        void setLiteral(Literal literal);
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    enum ConditionType
    {
        CT_AND        = 0,
        CT_GOAL       = 1,
        CT_FORALL     = 2,
        CT_PREFERENCE = 3
    };

    class DurativeCondition
    {
       public:
        ConditionType type;
        std::vector<DurativeCondition> conditions;  // if type==CT_AND | CT_FORALL
        GoalDescription goal;                       // if type==CT_GOAL | CT_PREFERENCE
        std::vector<Variable> parameters;           // if type==CT_FORALL
        std::string preferenceName;                 // if type==CT_PREFERENCE
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    enum EffectExpressionType
    {
        EE_NUMBER    = 0,
        EE_DURATION  = 1,
        EE_TERM      = 2,
        EE_UNDEFINED = 3,
        EE_SHARP_T   = 4,
        EE_OPERATION = 5,
        EE_FLUENT    = 6
    };

    enum OperationType
    {
        OT_SUM = 0,
        OT_SUB = 1,
        OT_DIV = 2,
        OT_MUL = 3
    };

    class EffectExpression
    {
       public:
        EffectExpressionType type;
        float value;                             // if type==EE_NUMBER
        Term term;                               // if type==EE_TERM
        OperationType operation;                 // if type==EE_OPERATION
        std::vector<EffectExpression> operands;  // if type==EE_OPERATION
        Literal fluent;                          // if type==EE_FLUENT
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    enum class Assignment
    {
        AS_ASSIGN     = 0,
        AS_INCREASE   = 1,
        AS_DECREASE   = 2,
        AS_SCALE_UP   = 3,
        AS_SCALE_DOWN = 4
    };
    std::string toString(Assignment a);

    class FluentAssignment
    {
       public:
        Assignment type;
        Literal fluent;
        EffectExpression exp;
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    enum TimedEffectType
    {
        TE_AND         = 0,
        TE_NOT         = 1,
        TE_LITERAL     = 2,
        TE_ASSIGNMENT  = 3,
        TE_NEG_LITERAL = 4,
        TE_OR          = 5
    };

    class TimedEffect
    {
       public:
        TimedEffectType type;
        TimeSpecifier time;
        std::vector<TimedEffect> terms;  // if type==TE_AND | TE_NOT
        Literal literal;                 // if type==TE_LITERAL | TE_NEG_LITERAL
        FluentAssignment assignment;     // if type==TE_ASSIGNMENT
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    enum DurativeEffectType
    {
        DET_AND          = 0,
        DET_TIMED_EFFECT = 1,
        DET_FORALL       = 2,
        DET_WHEN         = 3,
        DET_ASSIGNMENT   = 4
    };

    class ContinuousEffect
    {
       public:
        bool product;
        NumericExpression numExp;  // if product == true
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    class AssignmentContinuousEffect
    {
       public:
        Assignment type;
        Literal fluent;
        ContinuousEffect contEff;
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects);
    };

    class DurativeEffect
    {
       public:
        DurativeEffectType type;
        std::vector<DurativeEffect> terms;      // if type==DET_AND | DET_FORALL
        TimedEffect timedEffect;                // if type==DET_TIMED_EFFECT | DET_WHEN;
        std::vector<Variable> parameters;       // if type==DET_FORALL
        DurativeCondition condition;            // if type==DET_WHEN
        AssignmentContinuousEffect assignment;  // if type==DET_ASSIGNMENT
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    class DurativeAction
    {
       public:
        unsigned int index;
        std::string name;
        std::vector<Variable> parameters;
        std::vector<Duration> duration;
        DurativeCondition condition;
        DurativeEffect effect;
        std::string toString(const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    enum PreconditionType
    {
        PT_LITERAL     = 0,
        PT_AND         = 1,
        PT_NOT         = 2,
        PT_OR          = 3,
        PT_IMPLY       = 4,
        PT_EXISTS      = 5,
        PT_FORALL      = 6,
        PT_F_CMP       = 7,
        PT_EQUALITY    = 8,
        PT_PREFERENCE  = 9,
        PT_NEG_LITERAL = 10,
        PT_GOAL        = 11
    };

    class Precondition
    {
       public:
        PreconditionType type;
        Literal literal;                   // if type==PT_LITERAL | PT_NEG_LITERAL
        std::vector<Precondition> terms;   // if type==PT_AND | PT_NOT | PT_IMPLY | PT_EXISTS | PT_FORALL
        std::vector<Variable> parameters;  // if type==PT_EXISTS | PT_FORALL
        GoalDescription goal;              // if type==PT_F_CMP | PT_EQUALITY | PT_PREFERENCE | PT_GOAL
        std::string preferenceName;        // if type==PT_PREFERENCE
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    enum EffectType
    {
        ET_LITERAL     = 0,
        ET_AND         = 1,
        ET_NOT         = 2,
        ET_FORALL      = 3,
        ET_WHEN        = 4,
        ET_ASSIGNMENT  = 5,
        ET_NEG_LITERAL = 6
    };

    class Effect
    {
       public:
        EffectType type;
        std::vector<Effect> terms;         // if type==ET_AND | ET_NOT | ET_FORALL | ET_WHEN
        Literal literal;                   // if type==ET_LITERAL
        FluentAssignment assignment;       // if type==ET_ASSIGNMENT
        std::vector<Variable> parameters;  // if type==ET_FORALL
        GoalDescription goal;              // if type==ET_WHEN
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    class Action
    {
       public:
        int index;
        std::string name;
        std::vector<Variable> parameters;
        Precondition precondition;
        Effect effect;
        std::string toString(const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    class Fact
    {
       public:
        bool valueIsNumeric;
        unsigned int function;
        unsigned int value;
        float numericValue;
        float time;  // 0 if it is not a time-initial literal
        std::vector<unsigned int> parameters;
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
    };

    enum MetricType
    {
        MT_MINIMIZE = 0,
        MT_MAXIMIZE = 1,
        MT_NONE     = 2
    };

    enum MetricExpressionType
    {
        MT_PLUS        = 0,
        MT_MINUS       = 1,
        MT_PROD        = 2,
        MT_DIV         = 3,
        MT_NUMBER      = 4,
        MT_TOTAL_TIME  = 5,
        MT_IS_VIOLATED = 6,
        MT_FLUENT      = 7
    };

    class Metric
    {
       public:
        MetricExpressionType type;
        std::vector<Metric> terms;             // if type==MT_PLUS | MT_MINUS | MT_PROD | MT_DIV
        float value;                           // if type==MT_NUMBER
        std::string preferenceName;            // if type==MT_IS_VIOLATED
        unsigned int function;                 // if type==MT_FLUENT
        std::vector<unsigned int> parameters;  // if type==MT_FLUENT
        std::string toString(const std::vector<Function> &functions, const std::vector<Object> &objects);
    };

    enum ConstraintType
    {
        RT_AND             = 0,
        RT_FORALL          = 1,
        RT_PREFERENCE      = 2,
        RT_AT_END          = 3,
        RT_ALWAYS          = 4,
        RT_SOMETIME        = 5,
        RT_WITHIN          = 6,
        RT_AT_MOST_ONCE    = 7,
        RT_SOMETIME_AFTER  = 8,
        RT_SOMETIME_BEFORE = 9,
        RT_ALWAYS_WITHIN   = 10,
        RT_HOLD_DURING     = 11,
        RT_HOLD_AFTER      = 12,
        RT_GOAL_PREFERENCE = 13
    };

    class Constraint
    {
       private:
        std::string toString(const std::vector<Variable> &opParameters,
                             const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);

       public:
        ConstraintType type;
        std::vector<Constraint> terms;      // if type==RT_AND | RT_PREFERENCE | RT_FORALL
        std::vector<Variable> parameters;   // if type==RT_FORALL
        std::string preferenceName;         // if type==RT_PREFERENCE
        std::vector<GoalDescription> goal;  // if type!=RT_AND & RT_PREFERENCE & RT_FORALL
        std::vector<float> time;            // if type==RT_WITHIN | RT_ALWAYS_WITHIN | RT_HOLD_DURING | RT_HOLD_AFTER
        std::string toString(const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    class DerivedPredicate
    {
       public:
        Function function;
        GoalDescription goal;
        std::string toString(const std::vector<Function> &functions,
                             const std::vector<Object> &objects,
                             const std::vector<Type> &taskTypes);
    };

    class ParsedTask
    {
       private:
        std::unordered_map<std::string, unsigned int> typesByName;
        std::unordered_map<std::string, unsigned int> objectsByName;
        std::unordered_map<std::string, unsigned int> functionsByName;
        std::unordered_map<std::string, unsigned int> preferencesByName;
        std::vector<Constraint> preferences;

       public:
        unsigned int BOOLEAN_TYPE;
        unsigned int NUMBER_TYPE;
        unsigned int CONSTANT_FALSE;
        unsigned int CONSTANT_TRUE;
        std::string domainName;
        std::string problemName;
        std::vector<std::string> requirements;
        std::vector<Type> types;
        std::vector<Object> objects;
        std::vector<Function> functions;
        std::vector<DurativeAction> durativeActions;
        std::vector<Action> actions;
        std::vector<Fact> init;
        std::vector<Constraint> constraints;
        std::vector<DerivedPredicate> derivedPredicates;
        Precondition goal;
        MetricType metricType;
        Metric metric;
        int serialLength;
        int parallelLength;
        void setDomainName(std::string name);
        void setProblemName(std::string name);
        void setRequirement(std::string name);
        unsigned int getTypeIndex(std::string const &name);
        unsigned int addType(std::string name, std::vector<unsigned int> &parentTypes, SyntaxAnalyzer *syn);
        unsigned int getObjectIndex(std::string const &name);
        unsigned int addConstant(std::string name, std::vector<unsigned int> &types, SyntaxAnalyzer *syn);
        unsigned int addObject(std::string name, std::vector<unsigned int> &types, SyntaxAnalyzer *syn);
        unsigned int getFunctionIndex(std::string const &name);
        unsigned int addPredicate(Function fnc, SyntaxAnalyzer *syn);
        unsigned int addFunction(Function fnc, const std::vector<unsigned int> &valueTypes, SyntaxAnalyzer *syn);
        unsigned int addPreference(std::string name, const GoalDescription &goal, SyntaxAnalyzer *syn);
        unsigned int addPreference(const Constraint &c, SyntaxAnalyzer *syn);
        unsigned int getPreferenceIndex(std::string const &name);
        bool isNumericFunction(unsigned int fncIndex);
        bool isBooleanFunction(unsigned int fncIndex);
        bool compatibleTypes(const std::vector<unsigned int> &types, const std::vector<unsigned int> &validTypes);
        bool compatibleTypes(unsigned int t1, unsigned int t2);
        unsigned int addAction(std::string name,
                               const std::vector<Variable> &parameters,
                               const std::vector<Duration> &duration,
                               const DurativeCondition &condition,
                               const DurativeEffect &effect,
                               SyntaxAnalyzer *syn);
        unsigned int addAction(std::string name,
                               const std::vector<Variable> &parameters,
                               const Precondition &precondition,
                               const Effect &effect,
                               SyntaxAnalyzer *syn);
        std::string toString();
        static std::string comparatorToString(Comparator cmp);
        static std::string assignmentToString(Assignment a);
        static std::string timeToString(TimeSpecifier t);
    };
}  // namespace grstaps
#endif  // GRSTAPS_PARSED_TASK_HPP
