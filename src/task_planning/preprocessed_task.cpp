/********************************************************/
/* Simplified task obtained from the parsedTask after   */
/* the preprocess.                                      */
/********************************************************/
#include <iostream>

#include "grstaps/task_planning/preprocessed_task.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: OpPreference                                  */
    /********************************************************/

    std::string OpPreference::toString(const std::vector<Variable> &opParameters, ParsedTask *task)
    {
        return "(" + name + ": " + preference.toString(opParameters, task->functions, task->objects, task->types) + ")";
    }

    /********************************************************/
    /* CLASS: OpEquality                                    */
    /********************************************************/

    std::string OpEquality::toString(const std::vector<Object> &objects)
    {
        std::string s = "(";
        if(value1.isVariable)
        {
            s += "?" + std::to_string(value1.index);
        }
        else
        {
            s += objects[value1.index].name;
        }
        if(equal)
        {
            s += " = ";
        }
        else
        {
            s += " != ";
        }
        if(value2.isVariable)
        {
            s += "?" + std::to_string(value2.index);
        }
        else
        {
            s += objects[value2.index].name;
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: OpFluent                                      */
    /********************************************************/

    std::string OpFluent::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s = "(" + functions[variable.fncIndex].name;
        for(unsigned int i = 0; i < variable.params.size(); i++)
        {
            Term &term = variable.params[i];
            if(term.isVariable)
            {
                s += " ?" + std::to_string(term.index);
            }
            else
            {
                s += " " + objects[term.index].name;
            }
        }
        s += " = ";
        if(value.isVariable)
        {
            s += " ?" + std::to_string(value.index);
        }
        else
        {
            s += " " + objects[value.index].name;
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: OpEffectExpression                            */
    /********************************************************/

    OpEffectExpression::OpEffectExpression(EffectExpression &exp)
    {
        switch(exp.type)
        {
            case EE_NUMBER:
                type  = OEET_NUMBER;
                value = exp.value;
                break;
            case EE_DURATION:
                type = OEET_DURATION;
                break;
            case EE_SHARP_T:
                type = OEET_SHARP_T;
                break;
            case EE_OPERATION:
                type = OpEffectExpression::getOperator(exp.operation);
                for(unsigned int i = 0; i < exp.operands.size(); i++)
                {
                    OpEffectExpression op(exp.operands[i]);
                    operands.push_back(op);
                }
                break;
            case EE_FLUENT:
                type   = OEET_FLUENT;
                fluent = exp.fluent;
                break;
            case EE_TERM:
                type = OEET_TERM;
                term = exp.term;
                break;
            default:;
        }
    }

    OpEffectExpression::OpEffectExpression(ContinuousEffect &exp)
    {
        if(exp.product)
        {
            type = OEET_SHARP_T_PRODUCT;
            OpEffectExpression op(exp.numExp);
            operands.push_back(op);
        }
        else
            type = OEET_SHARP_T;
    }

    OpEffectExpression::OpEffectExpression(NumericExpression &exp)
    {
        switch(exp.type)
        {
            case NET_NUMBER:
                type  = OEET_NUMBER;
                value = exp.value;
                break;
            case NET_FUNCTION:
                type   = OEET_FLUENT;
                fluent = exp.function;
                break;
            case NET_TERM:
                type = OEET_TERM;
                term = exp.term;
                break;
            default:
                type = OpEffectExpression::getNumOperator(exp.type);
                for(unsigned int i = 0; i < exp.operands.size(); i++)
                {
                    OpEffectExpression op(exp.operands[i]);
                    operands.push_back(op);
                }
        }
    }

    std::string OpEffectExpression::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s;
        switch(type)
        {
            case OEET_NUMBER:
                s = std::to_string(value);
                break;
            case OEET_DURATION:
                s = "duration";
                break;
            case OEET_SHARP_T:
                s = "#t";
                break;
            case OEET_SHARP_T_PRODUCT:
                s = "(* #t " + operands[0].toString(functions, objects) + ")";
                break;
            case OEET_FLUENT:
                s = "(" + functions[fluent.fncIndex].name;
                for(unsigned int i = 0; i < fluent.params.size(); i++)
                {
                    Term &term = fluent.params[i];
                    if(term.isVariable)
                        s += " ?" + std::to_string(term.index);
                    else
                        s += " " + objects[term.index].name;
                }
                s += ")";
                break;
            case OEET_TERM:
                if(term.isVariable)
                    s += " ?" + std::to_string(term.index);
                else
                    s += " " + objects[term.index].name;
                s += ")";
                break;
            default:
                switch(type)
                {
                    case OEET_SUM:
                        s = "(+";
                        break;
                    case OEET_SUB:
                        s = "(-";
                        break;
                    case OEET_MUL:
                        s = "(*";
                        break;
                    default:
                        s = "(/";
                        break;
                }
                for(unsigned int i = 0; i < operands.size(); i++)
                    s += " " + operands[i].toString(functions, objects);
                s += ")";
        }
        return s;
    }

    /********************************************************/
    /* CLASS: OpEffect                                      */
    /********************************************************/

    OpEffect::OpEffect(FluentAssignment &f)
        : exp(f.exp)
    {
        assignment = f.type;
        fluent     = f.fluent;
    }

    OpEffect::OpEffect(AssignmentContinuousEffect &f)
        : exp(f.contEff)
    {
        assignment = f.type;
        fluent     = f.fluent;
    }

    std::string OpEffect::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s;
        switch(assignment)
        {
            case Assignment::AS_ASSIGN:
                s = "(assign ";
                break;
            case Assignment::AS_INCREASE:
                s = "(increase ";
                break;
            case Assignment::AS_DECREASE:
                s = "(decrease ";
                break;
            case Assignment::AS_SCALE_UP:
                s = "(scale_up ";
                break;
            case Assignment::AS_SCALE_DOWN:
                s = "(scale_down ";
                break;
        }
        s += " (" + functions[fluent.fncIndex].name;
        for(unsigned int i = 0; i < fluent.params.size(); i++)
        {
            Term &term = fluent.params[i];
            if(term.isVariable)
            {
                s += " ?" + std::to_string(term.index);
            }
            else
            {
                s += " " + objects[term.index].name;
            }
        }
        return s + ") " + exp.toString(functions, objects);
    }

    /********************************************************/
    /* CLASS: OpNumericPrec                                 */
    /********************************************************/

    OpNumericPrec::OpNumericPrec() {}

    OpNumericPrec::OpNumericPrec(GoalDescription &goal)
    {
        comparator = goal.comparator;
        for(unsigned int i = 0; i < goal.exp.size(); i++)
        {
            OpEffectExpression exp(goal.exp[i]);
            operands.push_back(exp);
        }
    }

    std::string OpNumericPrec::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s;
        switch(comparator)
        {
            case Comparator::k_eq:
                s = "(=";
                break;
            case Comparator::k_less:
                s = "(<";
                break;
            case Comparator::k_less_eq:
                s = "(<=";
                break;
            case Comparator::k_greater:
                s = "(>";
                break;
            case Comparator::k_greater_eq:
                s = "(>=";
                break;
            case Comparator::k_neq:
                s = "(!=";
                break;
        }
        for(unsigned int i = 0; i < operands.size(); i++)
            s += " " + operands[i].toString(functions, objects);
        return s + ")";
    }

    /********************************************************/
    /* CLASS: Operator                                      */
    /********************************************************/

    // Adds a precondition
    void Operator::addLiteralToPreconditions(Literal var, Term &value, TimeSpecifier time)
    {
        OpFluent f;
        f.variable = var;
        f.value    = value;
        switch(time)
        {
            case TimeSpecifier::k_at_start:
            case TimeSpecifier::k_none:
                atStart.prec.push_back(f);
                break;
            case TimeSpecifier::k_at_end:
                atEnd.prec.push_back(f);
                break;
            case TimeSpecifier::k_over_all:
                overAllPrec.push_back(f);
                break;
        }
    }

    // Adds an effect
    void Operator::addLiteralToEffects(Literal var, Term &value, TimeSpecifier time)
    {
        OpFluent f;
        f.variable = var;
        f.value    = value;
        if(time == TimeSpecifier::k_at_end || time == TimeSpecifier::k_none)
            atEnd.eff.push_back(f);
        else
            atStart.eff.push_back(f);
    }

    // Adds an equality condition
    void Operator::addEquality(Term &v1, Term &v2, bool equal)
    {
        OpEquality eq;
        eq.equal  = equal;
        eq.value1 = v1;
        eq.value2 = v2;
        equality.push_back(eq);
    }

    // Adds a numeric precondition (goal.type == GD_F_CMP)
    void Operator::addNumericPrecondition(GoalDescription goal, TimeSpecifier time)
    {
        if(goal.comparator == Comparator::k_eq && ((goal.exp[0].type == NET_FUNCTION && goal.exp[1].type == NET_TERM) ||
                                                   (goal.exp[0].type == NET_TERM && goal.exp[1].type == NET_FUNCTION)))
        {  // Fluent
            if(goal.exp[0].type == NET_FUNCTION)
                addLiteralToPreconditions(goal.exp[0].function, goal.exp[1].term, time);
            else
                addLiteralToPreconditions(goal.exp[1].function, goal.exp[0].term, time);
        }
        else
        {  // Comparator
            OpNumericPrec np;
            np.comparator = goal.comparator;
            for(unsigned int i = 0; i < goal.exp.size(); i++)
            {
                OpEffectExpression op(goal.exp[i]);
                np.operands.push_back(op);
            }
            switch(time)
            {
                case TimeSpecifier::k_at_start:
                case TimeSpecifier::k_none:
                    atStart.numericPrec.push_back(np);
                    break;
                case TimeSpecifier::k_at_end:
                    atEnd.numericPrec.push_back(np);
                    break;
                case TimeSpecifier::k_over_all:
                    overAllNumericPrec.push_back(np);
                    break;
            }
        }
    }

    // Adds a numeric effect (or fluent assignment)
    void Operator::addNumericEffect(OpEffect eff, TimeSpecifier time)
    {
        if(time == TimeSpecifier::k_at_start)
        {
            atStart.numericEff.push_back(eff);
        }
        else
        {
            atEnd.numericEff.push_back(eff);
        }
    }

    std::string Operator::toString(ParsedTask *task)
    {
        std::string s = name + "\nDURATION:";
        for(unsigned int i = 0; i < duration.size(); i++)
        {
            s += "\n " + duration[i].toString(parameters, task->functions, task->objects);
        }
        s += "\nPRECONDITIONS:";
        if(atStart.prec.size() + atStart.numericPrec.size() > 0)
        {
            s += "\n* AT-START:";
            for(unsigned int i = 0; i < atStart.prec.size(); i++)
            {
                s += "\n  Fluent: " + atStart.prec[i].toString(task->functions, task->objects);
            }
            for(unsigned int i = 0; i < atStart.numericPrec.size(); i++)
            {
                s += "\n  NumExp: " + atStart.numericPrec[i].toString(task->functions, task->objects);
            }
        }
        for(unsigned int i = 0; i < equality.size(); i++)
        {
            s += "\n  Eq: " + equality[i].toString(task->objects);
        }
        if(overAllPrec.size() + overAllNumericPrec.size() > 0)
        {
            s += "\n* OVER-ALL:";
            for(unsigned int i = 0; i < overAllPrec.size(); i++)
            {
                s += "\n  Fluent: " + overAllPrec[i].toString(task->functions, task->objects);
            }
            for(unsigned int i = 0; i < overAllNumericPrec.size(); i++)
            {
                s += "\n  NumExp: " + overAllNumericPrec[i].toString(task->functions, task->objects);
            }
        }
        if(atEnd.prec.size() + atEnd.numericPrec.size() > 0)
        {
            s += "\n* AT-END:";
            for(unsigned int i = 0; i < atEnd.prec.size(); i++)
                s += "\n  Fluent: " + atEnd.prec[i].toString(task->functions, task->objects);
            for(unsigned int i = 0; i < atEnd.numericPrec.size(); i++)
                s += "\n  NumExp: " + atEnd.numericPrec[i].toString(task->functions, task->objects);
        }
        if(preference.size() > 0)
        {
            s += "\nPREFERENCES:";
            for(unsigned int i = 0; i < preference.size(); i++)
                s += " " + preference[i].toString(parameters, task);
        }
        s += "\nEFFECTS:";
        if(atStart.eff.size() + atStart.numericEff.size() > 0)
        {
            s += "\n* AT-START:";
            for(unsigned int i = 0; i < atStart.eff.size(); i++)
                s += "\n  Fluent: " + atStart.eff[i].toString(task->functions, task->objects);
            for(unsigned int i = 0; i < atStart.numericEff.size(); i++)
                s += "\n  NumExp: " + atStart.numericEff[i].toString(task->functions, task->objects);
        }
        if(atEnd.eff.size() + atEnd.numericEff.size() > 0)
        {
            s += "\n* AT-END:";
            for(unsigned int i = 0; i < atEnd.eff.size(); i++)
                s += "\n  Fluent: " + atEnd.eff[i].toString(task->functions, task->objects);
            for(unsigned int i = 0; i < atEnd.numericEff.size(); i++)
                s += "\n  NumExp: " + atEnd.numericEff[i].toString(task->functions, task->objects);
        }
        return s;
    }

    /********************************************************/
    /* CLASS: PreprocessedTask                              */
    /********************************************************/

    // Creates a new preprocessed task
    PreprocessedTask::PreprocessedTask(ParsedTask *parsedTask)
    {
        task = parsedTask;
    }

    // Disposes the preprocessed task
    PreprocessedTask::~PreprocessedTask() {}

    std::string PreprocessedTask::toString()
    {
        std::string s("");
        for(unsigned int i = 0; i < operators.size(); i++)
        {
            s += "OPERATOR " + std::to_string(i) + ": ";
            s += operators[i].toString(task) + "\n";
        }
        return s;
    }
}  // namespace grstaps
