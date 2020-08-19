/********************************************************/
/* Stores the data parsed from the domain and problem   */
/* files.                                               */
/********************************************************/

#include <iostream>

#include "grstaps/task_planning/parsed_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: Type (PDDL type)                              */
    /********************************************************/

    Type::Type(unsigned int index, std::string name)
    {
        this->index = index;
        this->name  = name;
    }

    std::string Type::toString()
    {
        return name + "(" + std::to_string(index) + ")";
    }

    /********************************************************/
    /* CLASS: Variable (?name - type list)                  */
    /********************************************************/

    Variable::Variable(std::string name, const std::vector<unsigned int> &types)
    {
        this->name = name;
        for(unsigned int i = 0; i < types.size(); i++)
            this->types.push_back(types[i]);
    }

    std::string Variable::toString(const std::vector<Type> &taskTypes)
    {
        std::string res = name + " - ";
        if(types.size() == 1)
            res += taskTypes[types[0]].name;
        else
        {
            res += "(either";
            for(unsigned int i = 0; i < types.size(); i++)
                res += " " + taskTypes[types[i]].name;
            res += ")";
        }
        return res;
    }

    /********************************************************/
    /* CLASS: Object (PDDL object or constant)              */
    /********************************************************/

    Object::Object(unsigned int index, std::string name, bool isConstant)
    {
        this->index      = index;
        this->name       = name;
        this->isConstant = isConstant;
    }

    std::string Object::toString()
    {
        std::string res = name + "-";
        if(types.size() == 1)
            res += std::to_string(types[0]);
        else
        {
            res += "(either";
            for(unsigned int i = 0; i < types.size(); i++)
                res += " " + std::to_string(types[i]);
            res += ")";
        }
        return res;
    }

    /********************************************************/
    /* CLASS: Function (PDDL function or predicate)         */
    /********************************************************/

    Function::Function() {}

    Function::Function(std::string name, const std::vector<Variable> &parameters)
    {
        this->name = name;
        for(unsigned int i = 0; i < parameters.size(); i++)
            this->parameters.push_back(parameters[i]);
    }

    void Function::setValueTypes(const std::vector<unsigned int> &valueTypes)
    {
        for(unsigned int i = 0; i < valueTypes.size(); i++)
            this->valueTypes.push_back(valueTypes[i]);
    }

    std::string Function::toString(const std::vector<Type> &taskTypes)
    {
        std::string res = "(" + name;
        for(unsigned int i = 0; i < parameters.size(); i++)
            res += " " + parameters[i].toString(taskTypes);
        return res + ")";
    }

    /********************************************************/
    /* CLASS: Term (variable or constant)                   */
    /********************************************************/

    Term::Term() {}

    Term::Term(bool isVariable, unsigned int index)
    {
        this->isVariable = isVariable;
        this->index      = index;
    }

    std::string Term::toString(const std::vector<Variable> &parameters, const std::vector<Object> &objects)
    {
        if(isVariable)
            return parameters[index].name;
        else
            return objects[index].name;
    }

    /********************************************************/
    /* CLASS: Literal (atomic formula(term))                */
    /********************************************************/

    std::string Literal::toString(const std::vector<Variable> &parameters,
                                  const std::vector<Function> &functions,
                                  const std::vector<Object> &objects)
    {
        std::string s = "(" + functions[fncIndex].name;
        for(unsigned int i = 0; i < params.size(); i++)
            s += " " + params[i].toString(parameters, objects);
        return s + ")";
    }

    /********************************************************/
    /* CLASS: NumericExpression (numeric expression)        */
    /********************************************************/

    NumericExpression::NumericExpression() {}

    NumericExpression::NumericExpression(float value)
    {
        type        = NumericExpressionType::NET_NUMBER;
        this->value = value;
    }

    NumericExpression::NumericExpression(unsigned int fncIndex, const std::vector<Term> &fncParams)
    {
        type              = NumericExpressionType::NET_FUNCTION;
        function.fncIndex = fncIndex;
        for(unsigned int i = 0; i < fncParams.size(); i++)
            function.params.push_back(fncParams[i]);
    }

    NumericExpression::NumericExpression(Symbol s, const std::vector<NumericExpression> &operands, SyntaxAnalyzer *syn)
    {
        switch(s)
        {
            case Symbol::MINUS:
                if(operands.size() == 1)
                    type = NET_NEGATION;
                else if(operands.size() == 2)
                    type = NET_SUB;
                else
                    syn->notifyError("Invalid number of operands in subtraction");
                break;
            case Symbol::PLUS:
                if(operands.size() >= 2)
                    type = NET_SUM;
                else
                    syn->notifyError("Invalid number of operands in addition");
                break;
            case Symbol::PROD:
                if(operands.size() >= 2)
                    type = NET_MUL;
                else
                    syn->notifyError("Invalid number of operands in product");
                break;
            case Symbol::DIV:
                if(operands.size() == 2)
                    type = NET_DIV;
                else
                    syn->notifyError("Invalid number of operands in division");
                break;
            default:
                syn->notifyError("Invalid expression type");
        }
        for(unsigned int i = 0; i < operands.size(); i++)
            this->operands.push_back(operands[i]);
    }

    std::string NumericExpression::toString(const std::vector<Variable> &parameters,
                                            const std::vector<Function> &functions,
                                            const std::vector<Object> &objects)
    {
        if(type == NumericExpressionType::NET_NUMBER)
            return std::to_string(value);
        if(type == NumericExpressionType::NET_FUNCTION)
            return function.toString(parameters, functions, objects);
        if(type == NumericExpressionType::NET_TERM)
            return term.toString(parameters, objects);
        std::string s;
        switch(type)
        {
            case NET_NEGATION:
            case NET_SUB:
                s = "-";
                break;
            case NET_SUM:
                s = "+";
                break;
            case NET_MUL:
                s = "*";
                break;
            case NET_DIV:
                s = "/";
                break;
            default:
                s = "?";
        }
        for(unsigned int i = 0; i < operands.size(); i++)
            s += " " + operands[i].toString(parameters, functions, objects);
        return s;
    }

    /********************************************************/
    /* CLASS: Duration (duration constraint)                */
    /********************************************************/

    Duration::Duration(Symbol s, const NumericExpression &exp)
    {
        time      = TimeSpecifier::k_none;
        this->exp = exp;
        if(s == Symbol::EQUAL)
        {
            comp = Comparator::k_eq;
        }
        else if(s == Symbol::LESS_EQ)
        {
            comp = Comparator::k_less_eq;
        }
        else
        {
            comp = Comparator::k_greater_eq;
        }
    }

    std::string Duration::toString(const std::vector<Variable> &parameters,
                                   const std::vector<Function> &functions,
                                   const std::vector<Object> &objects)
    {
        std::string s = "(";
        if(time == TimeSpecifier::k_at_start)
        {
            s = "at start (";
        }
        else if(time == TimeSpecifier::k_at_end)
        {
            s = "at end (";
        }
        if(comp == Comparator::k_eq)
        {
            s += "=";
        }
        else if(comp == Comparator::k_less_eq)
        {
            s += "<=";
        }
        else if(comp == Comparator::k_greater_eq)
        {
            s += ">=";
        }
        s += " ?duration " + exp.toString(parameters, functions, objects);
        if(time == TimeSpecifier::k_at_start || time == TimeSpecifier::k_at_end)
        {
            s += ")";
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: GoalDescription (GD)                          */
    /********************************************************/

    void GoalDescription::setLiteral(Literal literal)
    {
        type          = GoalDescriptionType::GD_LITERAL;
        this->literal = literal;
    }

    std::string GoalDescription::toString(const std::vector<Variable> &opParameters,
                                          const std::vector<Function> &functions,
                                          const std::vector<Object> &objects,
                                          const std::vector<Type> &taskTypes)
    {
        std::string s;
        switch(time)
        {
            case TimeSpecifier::k_at_start:
                s = "AT START ";
                break;
            case TimeSpecifier::k_at_end:
                s = "AT END ";
                break;
            case TimeSpecifier::k_over_all:
                s = "OVER ALL ";
                break;
            default:
                s = "";
        }
        switch(type)
        {
            case GD_LITERAL:
                s += literal.toString(opParameters, functions, objects);
                break;
            case GD_AND:
                s += "(AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects, taskTypes);
                s += ")";
                break;
            case GD_NOT:
                s += "(NOT " + terms[0].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case GD_OR:
                s += "(OR";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " (" + terms[i].toString(opParameters, functions, objects, taskTypes) + ")";
                s += ")";
                break;
            case GD_IMPLY:
                s += "(IMPLY " + terms[0].toString(opParameters, functions, objects, taskTypes) + " " +
                     terms[1].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case GD_EXISTS:
            case GD_FORALL:
                s += type == GD_EXISTS ? "(EXISTS (" : "(FORALL (";
                for(unsigned int i = 0; i < parameters.size(); i++)
                {
                    if(i > 0)
                        s += " ";
                    s += parameters[i].toString(taskTypes);
                }
                s += ") " + terms[0].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case GD_F_CMP:
                switch(comparator)
                {
                    case Comparator::k_eq:
                        s += "(= (";
                        break;
                    case Comparator::k_less:
                        s += "(< (";
                        break;
                    case Comparator::k_less_eq:
                        s += "(<= (";
                        break;
                    case Comparator::k_greater:
                        s += "(> (";
                        break;
                    case Comparator::k_greater_eq:
                        s += "(>= (";
                        break;
                    case Comparator::k_neq:
                        s += "(!= (";
                        break;
                }
                s += exp[0].toString(opParameters, functions, objects) + ") (" +
                     exp[1].toString(opParameters, functions, objects) + "))";
                break;
            case GD_EQUALITY:
                s += "(= " + eqTerms[0].toString(opParameters, objects) + " " +
                     eqTerms[1].toString(opParameters, objects) + ")";
                break;
            case GD_INEQUALITY:
                s += "(!= " + eqTerms[0].toString(opParameters, objects) + " " +
                     eqTerms[1].toString(opParameters, objects) + ")";
                break;
            case GD_NEG_LITERAL:
                s += "~" + literal.toString(opParameters, functions, objects);
                break;
        }
        return s;
    }

    /********************************************************/
    /* CLASS: DurativeCondition (<da-GD>)                   */
    /********************************************************/

    std::string DurativeCondition::toString(const std::vector<Variable> &opParameters,
                                            const std::vector<Function> &functions,
                                            const std::vector<Object> &objects,
                                            const std::vector<Type> &taskTypes)
    {
        std::string s = "(";
        if(type == CT_AND)
        {
            s += "AND";
            for(unsigned int i = 0; i < conditions.size(); i++)
                s += " " + conditions[i].toString(opParameters, functions, objects, taskTypes);
        }
        else if(type == CT_GOAL)
        {
            s += goal.toString(opParameters, functions, objects, taskTypes);
        }
        else if(type == CT_FORALL)
        {
            s += "FORALL (";
            std::vector<Variable> mergedParameters;
            for(unsigned int i = 0; i < opParameters.size(); i++)
                mergedParameters.push_back(opParameters[i]);
            for(unsigned int i = 0; i < parameters.size(); i++)
            {
                if(i > 0)
                    s += " ";
                s += parameters[i].toString(taskTypes);
                mergedParameters.push_back(parameters[i]);
            }
            s += ") " + conditions[0].toString(mergedParameters, functions, objects, taskTypes);
        }
        else
        {  // CT_PREFERENCE
            s +=
                "PREFERENCE " + preferenceName + "(" + goal.toString(opParameters, functions, objects, taskTypes) + ")";
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: EffectExpression (<f-exp-da>)                 */
    /********************************************************/

    std::string EffectExpression::toString(const std::vector<Variable> &opParameters,
                                           const std::vector<Function> &functions,
                                           const std::vector<Object> &objects)
    {
        std::string s;
        switch(type)
        {
            case EE_NUMBER:
                s = std::to_string(value);
                break;
            case EE_DURATION:
                s = "?duration";
                break;
            case EE_TERM:
                s = term.toString(opParameters, objects);
                break;
            case EE_SHARP_T:
                s = "#t";
                break;
            case EE_OPERATION:
                switch(operation)
                {
                    case OT_SUM:
                        s = "+ ";
                        break;
                    case OT_SUB:
                        s = "- ";
                        break;
                    case OT_DIV:
                        s = "/ ";
                        break;
                    case OT_MUL:
                        s = "* ";
                        break;
                }
                for(unsigned int i = 0; i < operands.size(); i++)
                    s += " " + operands[i].toString(opParameters, functions, objects);
                break;
            case EE_FLUENT:
                s = fluent.toString(opParameters, functions, objects);
                break;
            default:
                s = "undefined";
        }
        return s;
    }

    /********************************************************/
    /* CLASS: FluentAssignment (<p-effect>)                 */
    /********************************************************/

    std::string FluentAssignment::toString(const std::vector<Variable> &opParameters,
                                           const std::vector<Function> &functions,
                                           const std::vector<Object> &objects)
    {
        std::string s;
        switch(type)
        {
            case Assignment::AS_ASSIGN:
                s = "ASSIGN ";
                break;
            case Assignment::AS_INCREASE:
                s = "INCREASE ";
                break;
            case Assignment::AS_DECREASE:
                s = "DECREASE ";
                break;
            case Assignment::AS_SCALE_UP:
                s = "SCALE-UP ";
                break;
            default:
                s = "SCALE-DOWN ";
        }
        return s + fluent.toString(opParameters, functions, objects) + " " +
               exp.toString(opParameters, functions, objects);
    }

    /********************************************************/
    /* CLASS: TimedEffect (<timed-effect>)                  */
    /********************************************************/

    std::string TimedEffect::toString(const std::vector<Variable> &opParameters,
                                      const std::vector<Function> &functions,
                                      const std::vector<Object> &objects)
    {
        std::string s;
        if(time == TimeSpecifier::k_at_start)
            s = "AT START ";
        else if(time == TimeSpecifier::k_at_end)
            s = "AT END ";
        else
            s = "";
        switch(type)
        {
            case TE_AND:
                s += "AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects);
                break;
            case TE_NOT:
                s += "(NOT " + terms[0].toString(opParameters, functions, objects) + ")";
                break;
            case TE_LITERAL:
                s += literal.toString(opParameters, functions, objects);
                break;
            case TE_ASSIGNMENT:
                s += assignment.toString(opParameters, functions, objects);
                break;
            default:;
        }
        return s;
    }

    /********************************************************/
    /* CLASS: ContinuousEffect (<f-exp-t>)                  */
    /********************************************************/

    std::string ContinuousEffect::toString(const std::vector<Variable> &opParameters,
                                           const std::vector<Function> &functions,
                                           const std::vector<Object> &objects)
    {
        if(product)
        {
            return "(* #t " + numExp.toString(opParameters, functions, objects) + ")";
        }
        else
        {
            return "#t";
        }
    }

    /************************************************************************/
    /* CLASS: AssignmentContinuousEffect (<assign-op-t> <f-head> <f-exp-t>) */
    /************************************************************************/

    std::string AssignmentContinuousEffect::toString(const std::vector<Variable> &opParameters,
                                                     const std::vector<Function> &functions,
                                                     const std::vector<Object> &objects)
    {
        std::string s = type == Assignment::AS_INCREASE ? "INCREASE " : "DECREASE ";
        return s + fluent.toString(opParameters, functions, objects) + " " +
               contEff.toString(opParameters, functions, objects);
    }

    /********************************************************/
    /* CLASS: DurativeEffect (<da-effect>)                  */
    /********************************************************/

    std::string DurativeEffect::toString(const std::vector<Variable> &opParameters,
                                         const std::vector<Function> &functions,
                                         const std::vector<Object> &objects,
                                         const std::vector<Type> &taskTypes)
    {
        std::string s = "(";
        switch(type)
        {
            case DET_AND:
                s += "AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects, taskTypes);
                break;
            case DET_TIMED_EFFECT:
                s += timedEffect.toString(opParameters, functions, objects);
                break;
            case DET_FORALL:
            {
                s += "FORALL (";
                std::vector<Variable> mergedParameters;
                for(unsigned int i = 0; i < opParameters.size(); i++)
                    mergedParameters.push_back(opParameters[i]);
                for(unsigned int i = 0; i < parameters.size(); i++)
                {
                    if(i > 0)
                        s += " ";
                    s += parameters[i].toString(taskTypes);
                    mergedParameters.push_back(parameters[i]);
                }
                s += ") " + terms[0].toString(mergedParameters, functions, objects, taskTypes);
            }
            break;
            case DET_WHEN:
                s += "WHEN " + condition.toString(opParameters, functions, objects, taskTypes) + " (" +
                     timedEffect.toString(opParameters, functions, objects) + ")";
                break;
            case DET_ASSIGNMENT:
                s += assignment.toString(opParameters, functions, objects);
                break;
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: DurativeAction (PDDL durative action)         */
    /********************************************************/

    std::string DurativeAction::toString(const std::vector<Function> &functions,
                                         const std::vector<Object> &objects,
                                         const std::vector<Type> &taskTypes)
    {
        std::string s = "DURATIVE-ACTION " + name + "\n* PARAMETERS (";
        for(unsigned int i = 0; i < parameters.size(); i++)
        {
            if(i > 0)
                s += " ";
            s += parameters[i].toString(taskTypes);
        }
        s += ")\n* DURATION (";
        for(unsigned int i = 0; i < duration.size(); i++)
        {
            if(i > 0)
                s += " ";
            s += duration[i].toString(parameters, functions, objects);
        }
        return s + "\n* CONDITION " + condition.toString(parameters, functions, objects, taskTypes) + "\n* EFFECT " +
               effect.toString(parameters, functions, objects, taskTypes);
    }

    /********************************************************/
    /* CLASS: Precondition (<pre-GD>)                       */
    /********************************************************/

    std::string Precondition::toString(const std::vector<Variable> &opParameters,
                                       const std::vector<Function> &functions,
                                       const std::vector<Object> &objects,
                                       const std::vector<Type> &taskTypes)
    {
        std::string s;
        switch(type)
        {
            case PT_LITERAL:
                s = literal.toString(opParameters, functions, objects);
                break;
            case PT_AND:
                s = "(AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects, taskTypes);
                s += ")";
                break;
            case PT_NOT:
                s = "(NOT " + terms[0].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case PT_OR:
                s = "(OR";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects, taskTypes);
                s += ")";
                break;
            case PT_IMPLY:
                s = "(IMPLY " + terms[0].toString(opParameters, functions, objects, taskTypes) +
                    terms[1].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case PT_EXISTS:
            case PT_FORALL:
            {
                s = type == PT_EXISTS ? "(EXISTS (" : "(FORALL (";
                std::vector<Variable> mergedParameters;
                for(unsigned int i = 0; i < opParameters.size(); i++)
                    mergedParameters.push_back(opParameters[i]);
                for(unsigned int i = 0; i < parameters.size(); i++)
                {
                    if(i > 0)
                        s += " ";
                    s += parameters[i].toString(taskTypes);
                    mergedParameters.push_back(parameters[i]);
                }
                s += ") " + terms[0].toString(mergedParameters, functions, objects, taskTypes) + ")";
            }
            break;
            case PT_F_CMP:
            case PT_EQUALITY:
            case PT_PREFERENCE:
            case PT_GOAL:
                if(type == PT_PREFERENCE)
                    s = "(PREFERENCE " + preferenceName + " ";
                else
                    s = "";
                s += goal.toString(opParameters, functions, objects, taskTypes);
                if(type == PT_PREFERENCE)
                    s += ")";
                break;
            case PT_NEG_LITERAL:
                s = "~" + literal.toString(opParameters, functions, objects);
                break;
        }
        return s;
    }

    /********************************************************/
    /* CLASS: Effect (<effect>)                             */
    /********************************************************/

    std::string Effect::toString(const std::vector<Variable> &opParameters,
                                 const std::vector<Function> &functions,
                                 const std::vector<Object> &objects,
                                 const std::vector<Type> &taskTypes)
    {
        std::string s;
        switch(type)
        {
            case ET_LITERAL:
                s = literal.toString(opParameters, functions, objects);
                break;
            case ET_AND:
                s = "(AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(opParameters, functions, objects, taskTypes);
                s += ")";
                break;
            case ET_NOT:
                s = "(NOT " + terms[0].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case ET_FORALL:
            {
                s = "(FORALL (";
                std::vector<Variable> mergedParameters;
                for(unsigned int i = 0; i < opParameters.size(); i++)
                    mergedParameters.push_back(opParameters[i]);
                for(unsigned int i = 0; i < parameters.size(); i++)
                {
                    if(i > 0)
                        s += " ";
                    s += parameters[i].toString(taskTypes);
                    mergedParameters.push_back(parameters[i]);
                }
                s += ") " + terms[0].toString(mergedParameters, functions, objects, taskTypes) + ")";
            }
            break;
            case ET_WHEN:
                s = "(WHEN " + goal.toString(opParameters, functions, objects, taskTypes) + " " +
                    terms[0].toString(opParameters, functions, objects, taskTypes) + ")";
                break;
            case ET_ASSIGNMENT:
                s = "(" + assignment.toString(opParameters, functions, objects) + ")";
                break;
            case ET_NEG_LITERAL:
                s = "~" + literal.toString(opParameters, functions, objects);
                break;
        }
        return s;
    }

    /********************************************************/
    /* CLASS: Action (PDDL action)                          */
    /********************************************************/

    std::string Action::toString(const std::vector<Function> &functions,
                                 const std::vector<Object> &objects,
                                 const std::vector<Type> &taskTypes)
    {
        std::string s = "ACTION " + name + "\n* PARAMETERS (";
        for(unsigned int i = 0; i < parameters.size(); i++)
        {
            if(i > 0)
                s += " ";
            s += parameters[i].toString(taskTypes);
        }
        return s + ")\n* PRECONDITION " + precondition.toString(parameters, functions, objects, taskTypes) +
               "\n* EFFECT " + effect.toString(parameters, functions, objects, taskTypes);
    }

    /********************************************************/
    /* CLASS: Fact (PDDL initial fact)                      */
    /********************************************************/

    std::string Fact::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s = "(";
        if(time != 0)
            s += "AT " + std::to_string(time) + " (";
        s += "= (" + functions[function].name;
        for(unsigned int i = 0; i < parameters.size(); i++)
            s += " " + objects[parameters[i]].name;
        s += ") ";
        if(valueIsNumeric)
            s += std::to_string(numericValue);
        else
            s += objects[value].name;
        if(time != 0)
            s += ")";
        return s + ")";
    }

    /********************************************************/
    /* CLASS: Metric (PDDL metric expression)               */
    /********************************************************/

    std::string Metric::toString(const std::vector<Function> &functions, const std::vector<Object> &objects)
    {
        std::string s;
        switch(type)
        {
            case MT_PLUS:
            case MT_MINUS:
            case MT_PROD:
            case MT_DIV:
                s = "(";
                if(type == MT_PLUS)
                    s += "+";
                else if(type == MT_MINUS)
                    s += "-";
                else if(type == MT_PROD)
                    s += "*";
                else
                    s += "/";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(functions, objects);
                s += ")";
                break;
            case MT_NUMBER:
                s = std::to_string(value);
                break;
            case MT_TOTAL_TIME:
                s = "total-time";
                break;
            case MT_IS_VIOLATED:
                s = "is-violated " + preferenceName;
                break;
            case MT_FLUENT:
                s = functions[function].name;
                for(unsigned int i = 0; i < parameters.size(); i++)
                    s += " " + objects[parameters[i]].name;
        }
        return s;
    }

    /********************************************************/
    /* CLASS: Constraint (PDDL constraint)                  */
    /********************************************************/

    std::string Constraint::toString(const std::vector<Function> &functions,
                                     const std::vector<Object> &objects,
                                     const std::vector<Type> &taskTypes)
    {
        std::vector<Variable> parameters;
        return toString(parameters, functions, objects, taskTypes);
    }

    std::string Constraint::toString(const std::vector<Variable> &opParameters,
                                     const std::vector<Function> &functions,
                                     const std::vector<Object> &objects,
                                     const std::vector<Type> &taskTypes)
    {
        std::string s = "(";
        switch(type)
        {
            case RT_AND:
                s += "AND";
                for(unsigned int i = 0; i < terms.size(); i++)
                    s += " " + terms[i].toString(parameters, functions, objects, taskTypes);
                break;
            case RT_FORALL:
            {
                s = "FORALL (";
                std::vector<Variable> mergedParameters;
                for(unsigned int i = 0; i < opParameters.size(); i++)
                    mergedParameters.push_back(opParameters[i]);
                for(unsigned int i = 0; i < parameters.size(); i++)
                {
                    if(i > 0)
                        s += " ";
                    s += parameters[i].toString(taskTypes);
                    mergedParameters.push_back(parameters[i]);
                }
                s += ") " + terms[0].toString(mergedParameters, functions, objects, taskTypes) + ")";
            }
            break;
            case RT_PREFERENCE:
                s += "PREFERENCE " + preferenceName + " " +
                     terms[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_AT_END:
                s += "AT END " + goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_ALWAYS:
                s += "ALWAYS " + goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_SOMETIME:
                s += "SOMETIME " + goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_WITHIN:
                s += "WITHIN " + std::to_string(time[0]) + " " +
                     goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_AT_MOST_ONCE:
                s += "AT-MOST-ONCE " + goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_SOMETIME_AFTER:
                s += "SOMETIME-AFTER " + goal[0].toString(opParameters, functions, objects, taskTypes) + " " +
                     goal[1].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_SOMETIME_BEFORE:
                s += "SOMETIME-BEFORE " + goal[0].toString(opParameters, functions, objects, taskTypes) + " " +
                     goal[1].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_ALWAYS_WITHIN:
                s += "ALWAYS-WITHIN " + std::to_string(time[0]) + " " +
                     goal[0].toString(opParameters, functions, objects, taskTypes) + " " +
                     goal[1].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_HOLD_DURING:
                s += "HOLD-DURING " + std::to_string(time[0]) + " " + std::to_string(time[1]) + " " +
                     goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_HOLD_AFTER:
                s += "HOLD-AFTER " + std::to_string(time[0]) + " " +
                     goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
            case RT_GOAL_PREFERENCE:
                s += "PREFERENCE " + preferenceName + " " +
                     goal[0].toString(opParameters, functions, objects, taskTypes);
                break;
        }
        return s + ")";
    }

    /********************************************************/
    /* CLASS: DerivedPredicate (derived predicate)          */
    /********************************************************/

    std::string DerivedPredicate::toString(const std::vector<Function> &functions,
                                           const std::vector<Object> &objects,
                                           const std::vector<Type> &taskTypes)
    {
        return "(DERIVED " + function.toString(taskTypes) + " " +
               goal.toString(function.parameters, functions, objects, taskTypes) + ")";
    }

    /********************************************************/
    /* CLASS: ParsedTask (PDDL planning task)               */
    /********************************************************/

    // Sets the name of the domain
    void ParsedTask::setDomainName(std::string name)
    {
        domainName = name;
        std::vector<unsigned int> parentTypes;
        BOOLEAN_TYPE = addType("#boolean", parentTypes, nullptr);
        NUMBER_TYPE  = addType("number", parentTypes, nullptr);
        parentTypes.push_back(BOOLEAN_TYPE);
        CONSTANT_FALSE = addConstant("#false", parentTypes, nullptr);
        CONSTANT_TRUE  = addConstant("#true", parentTypes, nullptr);
    }

    // Sets the name of the problem
    void ParsedTask::setProblemName(std::string name)
    {
        problemName = name;
    }

    // Sets a requirement key
    void ParsedTask::setRequirement(std::string name)
    {
        requirements.push_back(name);
    }

    // Returns the index of a type through its name
    unsigned int ParsedTask::getTypeIndex(std::string const &name)
    {
        std::unordered_map<std::string, unsigned int>::const_iterator index = typesByName.find(name);
        if(index == typesByName.end())
        {  // Type not found
            if(name.compare("#object") == 0)
            {
                unsigned int i = types.size();
                Type t(i, name);
                types.push_back(t);
                typesByName[name] = i;
                return i;
            }
            else
                return MAX_UNSIGNED_INT;
        }
        return index->second;
    }

    // Stores a PDDL type and returns its index
    unsigned int ParsedTask::addType(std::string name, std::vector<unsigned int> &parentTypes, SyntaxAnalyzer *syn)
    {
        unsigned int index = getTypeIndex(name);
        Type *t;
        if(index != MAX_UNSIGNED_INT)
        {  // Type redefined -> update its parent types
            t = &(types.at(index));
        }
        else
        {
            index = types.size();
            Type newType(index, name);
            types.push_back(newType);
            typesByName[name] = index;
            t                 = &(types.back());
        }
        for(unsigned int i = 0; i < parentTypes.size(); i++)
        {
            t->parentTypes.push_back(parentTypes[i]);
        }
        return index;
        /*
            if (getTypeIndex(name) != MAX_UNSIGNED_INT)
                syn->notifyError("Type '" + name + "' redefined");
            unsigned int index = types.size();
            Type t(index, name);
            for (unsigned int i = 0; i < parentTypes.size(); i++)
                t.parentTypes.push_back(parentTypes[i]);
            types.push_back(t);
            typesByName[name] = index;
            return index;*/
    }

    // Returns the index of an object through its name
    unsigned int ParsedTask::getObjectIndex(std::string const &name)
    {
        std::unordered_map<std::string, unsigned int>::const_iterator index = objectsByName.find(name);
        if(index == typesByName.end())
            return MAX_UNSIGNED_INT;
        else
            return index->second;
    }

    // Stores a PDDL constant and returns its index
    unsigned int ParsedTask::addConstant(std::string name, std::vector<unsigned int> &types, SyntaxAnalyzer *syn)
    {
        if(getObjectIndex(name) != MAX_UNSIGNED_INT)
            syn->notifyError("Constant '" + name + "' redefined");
        unsigned int index = objects.size();
        Object obj(index, name, true);
        for(unsigned int i = 0; i < types.size(); i++)
            obj.types.push_back(types[i]);
        objects.push_back(obj);
        objectsByName[name] = index;
        return index;
    }

    // Stores a PDDL object and returns its index
    unsigned int ParsedTask::addObject(std::string name, std::vector<unsigned int> &types, SyntaxAnalyzer *syn)
    {
        Object *obj;
        unsigned int index = getObjectIndex(name);
        if(getObjectIndex(name) != MAX_UNSIGNED_INT)
        {
            obj = &(objects.at(index));
        }
        else
        {
            index = objects.size();
            Object newObj(index, name, false);
            objects.push_back(newObj);
            objectsByName[name] = index;
            obj                 = &(objects.back());
        }
        for(unsigned int i = 0; i < types.size(); i++)
        {
            obj->types.push_back(types[i]);
        }
        return index;
    }

    // Returns the index of a function through its name
    unsigned int ParsedTask::getFunctionIndex(std::string const &name)
    {
        std::unordered_map<std::string, unsigned int>::const_iterator index = functionsByName.find(name);
        if(index == functionsByName.end())
            return MAX_UNSIGNED_INT;
        else
            return index->second;
    }

    // Returns the index of a preference through its name
    unsigned int ParsedTask::getPreferenceIndex(std::string const &name)
    {
        std::unordered_map<std::string, unsigned int>::const_iterator index = preferencesByName.find(name);
        if(index == preferencesByName.end())
            return MAX_UNSIGNED_INT;
        else
            return index->second;
    }

    // Stores a predicate and returns its index
    unsigned int ParsedTask::addPredicate(Function fnc, SyntaxAnalyzer *syn)
    {
        if(getFunctionIndex(fnc.name) != MAX_UNSIGNED_INT)
            syn->notifyError("Predicate '" + fnc.name + "' redefined");
        unsigned int index = functions.size();
        fnc.index          = index;
        fnc.valueTypes.push_back(BOOLEAN_TYPE);
        functions.push_back(fnc);
        functionsByName[fnc.name] = index;
        return index;
    }

    // Stores a function and returns its index
    unsigned int ParsedTask::addFunction(Function fnc, const std::vector<unsigned int> &valueTypes, SyntaxAnalyzer *syn)
    {
        if(getFunctionIndex(fnc.name) != MAX_UNSIGNED_INT)
            syn->notifyError("Function '" + fnc.name + "' redefined");
        unsigned int index = functions.size();
        fnc.index          = index;
        fnc.setValueTypes(valueTypes);
        functions.push_back(fnc);
        functionsByName[fnc.name] = index;
        return index;
    }

    // Stores a durative action and returns its index
    unsigned int ParsedTask::addAction(std::string name,
                                       const std::vector<Variable> &parameters,
                                       const std::vector<Duration> &duration,
                                       const DurativeCondition &condition,
                                       const DurativeEffect &effect,
                                       SyntaxAnalyzer *syn)
    {
        for(unsigned int i = 0; i < durativeActions.size(); i++)
            if(durativeActions[i].name.compare(name) == 0)
                syn->notifyError("Action '" + name + "' redefined");
        for(unsigned int i = 0; i < actions.size(); i++)
            if(actions[i].name.compare(name) == 0)
                syn->notifyError("Action '" + name + "' redefined");
        DurativeAction a;
        a.index = durativeActions.size();
        a.name  = name;
        for(unsigned int i = 0; i < parameters.size(); i++)
            a.parameters.push_back(parameters[i]);
        for(unsigned int i = 0; i < duration.size(); i++)
            a.duration.push_back(duration[i]);
        a.condition = condition;
        a.effect    = effect;
        durativeActions.push_back(a);
        return a.index;
    }

    // Stores an action and returns its index
    unsigned int ParsedTask::addAction(std::string name,
                                       const std::vector<Variable> &parameters,
                                       const Precondition &precondition,
                                       const Effect &effect,
                                       SyntaxAnalyzer *syn)
    {
        for(unsigned int i = 0; i < durativeActions.size(); i++)
            if(durativeActions[i].name.compare(name) == 0)
                syn->notifyError("Action '" + name + "' redefined");
        for(unsigned int i = 0; i < actions.size(); i++)
            if(actions[i].name.compare(name) == 0)
                syn->notifyError("Action '" + name + "' redefined");
        Action a;
        a.index = actions.size();
        a.name  = name;
        for(unsigned int i = 0; i < parameters.size(); i++)
            a.parameters.push_back(parameters[i]);
        a.precondition = precondition;
        a.effect       = effect;
        actions.push_back(a);
        return a.index;
    }

    // Stores a preference
    unsigned int ParsedTask::addPreference(std::string name, const GoalDescription &goal, SyntaxAnalyzer *syn)
    {
        if(getPreferenceIndex(name) != MAX_UNSIGNED_INT)
            syn->notifyError("Preference '" + name + "' redefined");
        unsigned int index      = preferences.size();
        preferencesByName[name] = index;
        Constraint c;
        c.type           = RT_GOAL_PREFERENCE;
        c.preferenceName = name;
        c.goal.push_back(goal);
        preferences.push_back(c);
        return index;
    }

    // Stores a preference
    unsigned int ParsedTask::addPreference(const Constraint &c, SyntaxAnalyzer *syn)
    {
        if(getPreferenceIndex(c.preferenceName) != MAX_UNSIGNED_INT)
            syn->notifyError("Preference '" + c.preferenceName + "' redefined");
        unsigned int index                  = preferences.size();
        preferencesByName[c.preferenceName] = index;
        preferences.push_back(c);
        return index;
    }

    // Checks whether the given function is numeric
    bool ParsedTask::isNumericFunction(unsigned int fncIndex)
    {
        Function &f = functions[fncIndex];
        if(f.valueTypes.size() != 1)
            return false;
        return f.valueTypes[0] == NUMBER_TYPE;
    }

    // Checks whether the given function is boolean (a predicate)
    bool ParsedTask::isBooleanFunction(unsigned int fncIndex)
    {
        Function &f = functions[fncIndex];
        if(f.valueTypes.size() != 1)
            return false;
        return f.valueTypes[0] == BOOLEAN_TYPE;
    }

    // Returns a description of this planning task
    std::string ParsedTask::toString()
    {
        std::string res = "Domain: " + domainName;
        res += "\nRequirements:";
        for(unsigned int i = 0; i < requirements.size(); i++)
            res += " " + requirements[i];
        res += "\nTypes:";
        for(unsigned int i = 0; i < types.size(); i++)
            res += "\n* " + types[i].toString();
        res += "\nObjects:";
        for(unsigned int i = 0; i < objects.size(); i++)
            res += "\n* " + objects[i].toString();
        res += "\nFunctions:";
        for(unsigned int i = 0; i < functions.size(); i++)
            res += "\n* " + functions[i].toString(types);
        for(unsigned int i = 0; i < durativeActions.size(); i++)
            res += "\n" + durativeActions[i].toString(functions, objects, types);
        for(unsigned int i = 0; i < actions.size(); i++)
            res += "\n" + actions[i].toString(functions, objects, types);
        res += "\nInit:";
        for(unsigned int i = 0; i < init.size(); i++)
            res += "\n* " + init[i].toString(functions, objects);
        std::vector<Variable> parameters;
        res += "\nGoal:\n* " + goal.toString(parameters, functions, objects, types);
        if(metricType != MT_NONE)
        {
            res += "\nMetric: ";
            if(metricType == MT_MINIMIZE)
                res += "MINIMIZE ";
            else
                res += "MAXIMIZE ";
            res += metric.toString(functions, objects);
        }
        for(unsigned int i = 0; i < constraints.size(); i++)
            res += "\nConstraint:\n* " + constraints[i].toString(functions, objects, types);
        for(unsigned int i = 0; i < derivedPredicates.size(); i++)
            res += "\n" + derivedPredicates[i].toString(functions, objects, types);
        return res;
    }

    // Checks if one of the types is compatible with one of the valid ones
    bool ParsedTask::compatibleTypes(const std::vector<unsigned int> &types,
                                     const std::vector<unsigned int> &validTypes)
    {
        unsigned int t1, t2;
        for(unsigned int i = 0; i < types.size(); i++)
        {
            t1 = types[i];
            for(unsigned int j = 0; j < validTypes.size(); j++)
            {
                t2 = validTypes[j];
                if(compatibleTypes(t1, t2))
                    return true;
            }
        }
        return false;
    }

    // Checks if a given type t1 is compatible with another one t2
    bool ParsedTask::compatibleTypes(unsigned int t1, unsigned int t2)
    {
        if(t1 == t2)
            return true;
        Type &refT1 = types[t1];
        for(unsigned int i = 0; i < refT1.parentTypes.size(); i++)
            if(compatibleTypes(refT1.parentTypes[i], t2))
                return true;
        return false;
    }

    // Returns a std::string representation of a comparator
    std::string toString(Comparator cmp)
    {
        switch(cmp)
        {
            case Comparator::k_eq:
                return "=";
            case Comparator::k_less:
                return "<";
            case Comparator::k_less_eq:
                return "<=";
            case Comparator::k_greater:
                return ">";
            case Comparator::k_greater_eq:
                return ">=";
            case Comparator::k_neq:
                return "!=";
        }
        return "";
    }

    // Returns a std::string representation of an assignment
    std::string toString(Assignment a)
    {
        switch(a)
        {
            case Assignment::AS_ASSIGN:
                return "assign";
            case Assignment::AS_INCREASE:
                return "increase";
            case Assignment::AS_DECREASE:
                return "decrease";
            case Assignment::AS_SCALE_UP:
                return "scale-up";
            case Assignment::AS_SCALE_DOWN:
                return "scale-down";
        }
        return "";
    }

    // Returns a std::string representation of a time specifier
    std::string toString(TimeSpecifier t)
    {
        switch(t)
        {
            case TimeSpecifier::k_at_start:
                return "at start";
            case TimeSpecifier::k_at_end:
                return "at end";
            case TimeSpecifier::k_over_all:
                return "over all";
            default:
                return "";
        }
    }
}  // namespace grstaps
