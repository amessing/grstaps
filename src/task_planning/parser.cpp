/********************************************************/
/* Parses the domain and problem files. Stores the      */
/* result in a variable of type ParsedTask.             */
/********************************************************/

#include "grstaps/task_planning/parser.hpp"
#include <iostream>

#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    // Creates a new parser
    Parser::Parser()
        : task(nullptr)
        , syn(nullptr)
    {}

    // Disposes the parser
    Parser::~Parser() {}

    // <domain> ::= (define (domain <name>) [<require-def>] [<types-def>]:typing
    //  [<constants-def>] [<predicates-def>] [<functions-def>]:fluents
    //  [<constraints>] <structure-def>*)
    ParsedTask* Parser::parseDomain(char* domainFileName)
    {
        task = new ParsedTask();
        syn  = new SyntaxAnalyzer(domainFileName);
        syn->openPar();
        syn->readSymbol(Symbol::DEFINE);
        syn->openPar();
        syn->readSymbol(Symbol::PDDLDOMAIN);
        task->setDomainName(syn->readName());
        syn->closePar();
        Token* token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        while(syn->isSym(token, Symbol::OPEN_PAR))
        {
            syn->readColon();
            token = syn->readSymbol(9,
                                    Symbol::REQUIREMENTS,
                                    Symbol::TYPES,
                                    Symbol::CONSTANTS,
                                    Symbol::PREDICATES,
                                    Symbol::FUNCTIONS,
                                    Symbol::CONSTRAINTS,
                                    Symbol::ACTION,
                                    Symbol::DURATIVE_ACTION,
                                    Symbol::DERIVED);
            switch(token->symbol)
            {
                case Symbol::REQUIREMENTS:
                    parseRequirements();
                    break;
                case Symbol::TYPES:
                    parseTypes();
                    break;
                case Symbol::CONSTANTS:
                    parseConstants();
                    break;
                case Symbol::PREDICATES:
                    parsePredicates();
                    break;
                case Symbol::FUNCTIONS:
                    parseFunctions();
                    break;
                case Symbol::DURATIVE_ACTION:
                    parseDurativeAction();
                    break;
                case Symbol::ACTION:
                    parseAction();
                    break;
                case Symbol::CONSTRAINTS:
                    parseConstraints();
                    break;
                case Symbol::DERIVED:
                    parseDerivedPredicates();
                    break;
                default:;
            }
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        }
        delete syn;
        return task;
    }

    // <problem> ::= (define (problem <name>)
    //               (:domain <name>)
    //              [<require-def>] [<object declaration>]
    //              <init> <goal>
    //              [<constraints>]:constraints
    //              [<metric-spec>]:numeric-fluents
    //              [<length-spec>])
    ParsedTask* Parser::parseProblem(char* problemFileName)
    {
        task->metricType     = MT_NONE;
        task->serialLength   = -1;
        task->parallelLength = -1;
        syn                  = new SyntaxAnalyzer(problemFileName);
        syn->openPar();
        syn->readSymbol(Symbol::DEFINE);
        syn->openPar();
        syn->readSymbol(Symbol::PROBLEM);
        task->setProblemName(syn->readName());
        syn->closePar();
        syn->openPar();
        syn->readColon();
        syn->readSymbol(Symbol::PDDLDOMAIN);
        syn->readName();
        syn->closePar();
        Token* token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        while(syn->isSym(token, Symbol::OPEN_PAR))
        {
            syn->readColon();
            token = syn->readSymbol(7,
                                    Symbol::REQUIREMENTS,
                                    Symbol::OBJECTS,
                                    Symbol::INIT,
                                    Symbol::GOAL,
                                    Symbol::CONSTRAINTS,
                                    Symbol::METRIC,
                                    Symbol::LENGTH);
            switch(token->symbol)
            {
                case Symbol::REQUIREMENTS:
                    parseRequirements();
                    break;
                case Symbol::OBJECTS:
                    parseObjects();
                    break;
                case Symbol::INIT:
                    parseInit();
                    break;
                case Symbol::GOAL:
                    parseGoal();
                    break;
                case Symbol::CONSTRAINTS:
                    parseConstraints();
                    break;
                case Symbol::METRIC:
                    parseMetric();
                    break;
                case Symbol::LENGTH:
                    parseLength();
                    break;
                default:;
            }
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        }
        delete syn;
        return task;
    }

    // <require-def> ::= (:requirements <require-key>+)
    void Parser::parseRequirements()
    {
        Token* token = syn->readSymbol(2, Symbol::COLON, Symbol::CLOSE_PAR);
        while(syn->isSym(token, Symbol::COLON))
        {
            task->setRequirement(syn->readName());
            token = syn->readSymbol(2, Symbol::COLON, Symbol::CLOSE_PAR);
        }
    }

    // <types-def> ::= (:types <typed list (name)>)
    // <typed list (x)> ::= x*
    // <typed list (x)> ::= :typing x+ - <type> <typed list(x)>
    // <primitive-type> ::= <name>
    // <primitive-type> ::= object
    void Parser::parseTypes()
    {
        Token* token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::NAME);
        std::vector<std::string> typeNames;
        std::vector<unsigned int> parentTypes;
        while(!syn->isSym(token, Symbol::CLOSE_PAR))
        {
            while(syn->isSym(token, Symbol::NAME))
            {
                typeNames.push_back(token->description);
                token = syn->readSymbol(3, Symbol::MINUS, Symbol::CLOSE_PAR, Symbol::NAME);
            }
            if(syn->isSym(token, Symbol::MINUS))
            {
                parseParentTypes(parentTypes, false);
                token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::NAME);
            }
            else
                parentTypes.push_back(task->getTypeIndex("#object"));
            for(unsigned int i = 0; i < typeNames.size(); i++)
                task->addType(typeNames[i], parentTypes, syn);
            typeNames.clear();
            parentTypes.clear();
        }
    }

    // <type> ::= (either <primitive-type>+)
    // <type> ::= <primitive-type>
    void Parser::parseParentTypes(std::vector<unsigned int>& types, bool allowNumber)
    {
        Token* token = !allowNumber ? syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::NAME)
                                    : syn->readSymbol(3, Symbol::OPEN_PAR, Symbol::NUMBER_TYPE, Symbol::NAME);
        unsigned int index;
        if(syn->isSym(token, Symbol::NAME))
        {
            index = task->getTypeIndex(token->description);
            if(index == MAX_UNSIGNED_INT)
            {
                index = task->getTypeIndex("#object");
                if(token->description.compare("object") != 0)
                {
                    std::vector<unsigned int> parentTypes;
                    parentTypes.push_back(index);
                    index = task->addType(token->description, parentTypes, syn);
                }
            }
            types.push_back(index);
        }
        else if(syn->isSym(token, Symbol::NUMBER_TYPE))
        {
            types.push_back(task->NUMBER_TYPE);
        }
        else
        {
            syn->readSymbol(Symbol::EITHER);
            do
            {
                token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::NAME);
                if(syn->isSym(token, Symbol::NAME))
                {
                    index = task->getTypeIndex(token->description);
                    if(index == MAX_UNSIGNED_INT)
                    {
                        index = task->getTypeIndex("#object");
                        if(token->description.compare("object") != 0)
                        {
                            std::vector<unsigned int> parentTypes;
                            parentTypes.push_back(index);
                            index = task->addType(token->description, parentTypes, syn);
                        }
                    }
                    types.push_back(index);
                }
            } while(!syn->isSym(token, Symbol::CLOSE_PAR));
        }
    }

    // <constants-def> ::= (:constants <typed list (name)>)
    void Parser::parseConstants()
    {
        Token* token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::NAME);
        std::vector<std::string> constNames;
        std::vector<unsigned int> types;
        while(!syn->isSym(token, Symbol::CLOSE_PAR))
        {
            while(syn->isSym(token, Symbol::NAME))
            {
                constNames.push_back(token->description);
                token = syn->readSymbol(3, Symbol::MINUS, Symbol::CLOSE_PAR, Symbol::NAME);
            }
            if(syn->isSym(token, Symbol::MINUS))
            {
                parseParentTypes(types, false);
                token = syn->readSymbol(2, Symbol::NAME, Symbol::CLOSE_PAR);
            }
            else
                types.push_back(task->getTypeIndex("#object"));
            for(unsigned int i = 0; i < constNames.size(); i++)
            {
                task->addConstant(constNames[i], types, syn);
            }
            constNames.clear();
            types.clear();
        }
    }

    // (:predicates <atomic formula skeleton>+)
    void Parser::parsePredicates()
    {
        Token* token;
        do
        {
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            if(syn->isSym(token, Symbol::OPEN_PAR))
            {
                syn->undoToken();
                Function const& fnc = parsePredicate();
                task->addPredicate(fnc, syn);
            }
        } while(!syn->isSym(token, Symbol::CLOSE_PAR));
    }

    // <atomic formula skeleton> ::= (<predicate> <typed list (variable)>)
    // <predicate> ::= <name>
    Function Parser::parsePredicate()
    {
        syn->openPar();
        std::string name = syn->readName();
        if(task->getFunctionIndex(name) != MAX_UNSIGNED_INT)
        {
            syn->notifyError("Predicate '" + name + "' redefined");
        }
        std::vector<Variable> parameters;
        parseVariableList(parameters);
        syn->closePar();
        Function fnc(name, parameters);
        return fnc;
    }

    // <typed list (variable)> ::= variable*
    // <typed list (variable)> ::= :typing variable+ - <type> <typed list(variable)>
    void Parser::parseVariableList(std::vector<Variable>& parameters)
    {
        std::vector<std::string> varNames;
        std::vector<unsigned int> types;
        Token* token;
        do
        {
            token = syn->readSymbol(2, Symbol::VARIABLE, Symbol::CLOSE_PAR);
            if(!syn->isSym(token, Symbol::CLOSE_PAR))
            {
                do
                {
                    varNames.push_back(token->description);
                    token = syn->readSymbol(3, Symbol::VARIABLE, Symbol::MINUS, Symbol::CLOSE_PAR);
                } while(syn->isSym(token, Symbol::VARIABLE));
                if(syn->isSym(token, Symbol::MINUS))
                    parseParentTypes(types, false);
                else
                    types.push_back(task->getTypeIndex("#object"));
                for(unsigned int i = 0; i < varNames.size(); i++)
                {
                    Variable v(varNames[i], types);
                    parameters.push_back(v);
                }
                varNames.clear();
                types.clear();
            }
        } while(!syn->isSym(token, Symbol::CLOSE_PAR));
        syn->undoToken();
    }

    // <functions-def> ::=:fluents (:functions <function typed list (atomic function skeleton)>)
    // <function typed list (x)> ::= x+ - <function type> <function typed list(x)>
    // <function typed list (x)> ::=
    // <function type> ::=:numeric-fluents number
    // <function type> ::=:typing + :object-fluents <type>
    void Parser::parseFunctions()
    {
        std::vector<Function> functions;
        std::vector<unsigned int> types;
        Token *token, *aux;
        do
        {
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            if(syn->isSym(token, Symbol::OPEN_PAR))
            {
                do
                {
                    syn->undoToken();
                    Function fnc = parsePredicate();
                    functions.push_back(fnc);
                    aux = syn->readSymbol(3, Symbol::OPEN_PAR, Symbol::MINUS, Symbol::CLOSE_PAR);
                } while(syn->isSym(aux, Symbol::OPEN_PAR));
                if(syn->isSym(aux, Symbol::MINUS))
                    parseParentTypes(types, true);
                else
                {
                    types.push_back(task->NUMBER_TYPE);
                    token->symbol = Symbol::CLOSE_PAR;
                }
                for(unsigned int i = 0; i < functions.size(); i++)
                    task->addFunction(functions[i], types, syn);
                functions.clear();
                types.clear();
            }
        } while(!syn->isSym(token, Symbol::CLOSE_PAR));
    }

    // <durative-action-def> ::= (:durative-action <da-symbol>
    //      :parameters (<typed list (variable)>) <da-def body>)
    // <da-def body> ::= :duration <duration-constraint>
    //      :condition <emptyOr (da-GD)> :effect <emptyOr (da-effect)>
    void Parser::parseDurativeAction()
    {
        std::string name = syn->readName();
        syn->readColon();
        syn->readSymbol(Symbol::PARAMETERS);
        syn->openPar();
        std::vector<Variable> parameters;
        parseVariableList(parameters);
        syn->closePar();
        syn->readColon();
        syn->readSymbol(Symbol::DURATION);
        std::vector<Duration> duration;
        parseDurationConstraint(duration, parameters);
        syn->readColon();
        Token* token = syn->readSymbol(2, Symbol::CONDITION, Symbol::EFFECT);
        DurativeCondition condition;
        if(syn->isSym(token, Symbol::CONDITION))
        {
            condition = parseDurativeCondition(parameters);
            syn->readColon();
        }
        else
        {
            condition.type = CT_AND;
            syn->undoToken();
        }
        syn->readSymbol(Symbol::EFFECT);
        DurativeEffect effect = parseDurativeEffect(parameters);
        task->addAction(name, parameters, duration, condition, effect, syn);
        syn->closePar();
    }

    // <duration-constraint> ::=:duration-inequalities (and <simple-duration-constraint>+)
    // <duration-constraint> ::= ()
    // <duration-constraint> ::= <simple-duration-constraint>
    // <simple-duration-constraint> ::= (<d-op> ?duration <d-value>)
    // <simple-duration-constraint> ::= (at <time-specifier> <simple-duration-constraint>)
    void Parser::parseDurationConstraint(std::vector<Duration>& duration, const std::vector<Variable>& parameters)
    {
        syn->openPar();
        Token* token = syn->readSymbol(
            6, Symbol::AND, Symbol::CLOSE_PAR, Symbol::LESS_EQ, Symbol::GREATER_EQ, Symbol::EQUAL, Symbol::AT);
        if(syn->isSym(token, Symbol::CLOSE_PAR))
            return;
        if(syn->isSym(token, Symbol::AT))
        {
            token              = syn->readSymbol(2, Symbol::START, Symbol::END);
            TimeSpecifier time = syn->isSym(token, Symbol::START) ? TimeSpecifier::k_at_start : TimeSpecifier::k_at_end;
            unsigned int startSize = duration.size();
            parseDurationConstraint(duration, parameters);
            for(unsigned int i = startSize; i < duration.size(); i++)
                duration[i].time = time;
            syn->closePar();
        }
        else if(syn->isSym(token, Symbol::AND))
        {
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            while(syn->isSym(token, Symbol::OPEN_PAR))
            {
                syn->undoToken();
                parseDurationConstraint(duration, parameters);
                token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            }
        }
        else
        {  // =, <= or >=
            Token* aux = syn->readSymbol(Symbol::VARIABLE);
            if(aux->description.compare("?duration") != 0)
                syn->notifyError("Variable ?duration expected");
            NumericExpression exp = parseNumericExpression(parameters);
            duration.push_back(Duration(token->symbol, exp));
            syn->closePar();
        }
    }

    // <f-exp> ::=:numeric-fluents <number>
    // <f-exp> ::=:numeric-fluents (<binary-op> <f-exp> <f-exp>)
    // <f-exp> ::=:numeric-fluents (<multi-op> <f-exp> <f-exp>+)
    // <f-exp> ::=:numeric-fluents (- <f-exp>)
    // <f-exp> ::=:numeric-fluents <f-head>
    NumericExpression Parser::parseNumericExpression(const std::vector<Variable>& parameters)
    {
        Token* token = syn->readSymbol(5, Symbol::NUMBER, Symbol::OPEN_PAR, Symbol::AT, Symbol::VARIABLE, Symbol::NAME);
        if(syn->isSym(token, Symbol::NUMBER))
            return NumericExpression(token->value);
        if(syn->isSym(token, Symbol::AT) || syn->isSym(token, Symbol::NAME))
        {
            std::vector<Term> fncParams;
            syn->undoToken();
            unsigned int fncIndex = !syn->isSym(token, Symbol::AT) ? task->getFunctionIndex(token->description)
                                                                   : task->getFunctionIndex("at");
            if(fncIndex == MAX_UNSIGNED_INT)
            {  // Constant
                NumericExpression exp;
                exp.type = NET_TERM;
                exp.term = parseTerm(parameters);
                return exp;
            }
            else
            {
                parseFunctionHead(fncParams, parameters);
                return NumericExpression(fncIndex, fncParams);
            }
        }
        if(syn->isSym(token, Symbol::VARIABLE))
        {
            NumericExpression exp;
            exp.type = NET_TERM;
            syn->undoToken();
            exp.term = parseTerm(parameters);
            return exp;
        }
        token = syn->readSymbol(6, Symbol::MINUS, Symbol::PLUS, Symbol::PROD, Symbol::DIV, Symbol::AT, Symbol::NAME);
        if(syn->isSym(token, Symbol::AT) || syn->isSym(token, Symbol::NAME))
        {
            std::vector<Term> fncParams;
            syn->undoToken();
            unsigned int fncIndex = parseFunctionHead(fncParams, parameters);
            syn->closePar();
            return NumericExpression(fncIndex, fncParams);
        }
        else
        {
            std::vector<NumericExpression> operands;
            bool moreOperands;
            do
            {
                Token* aux   = syn->nextToken();
                moreOperands = syn->isSym(aux, Symbol::NUMBER) || syn->isSym(aux, Symbol::OPEN_PAR) ||
                               syn->isSym(aux, Symbol::NAME);
                if(moreOperands)
                {
                    syn->undoToken();
                    NumericExpression op = parseNumericExpression(parameters);
                    operands.push_back(op);
                }
            } while(moreOperands);
            return NumericExpression(token->symbol, operands, syn);
        }
    }

    // <f-head> ::= (<function-symbol> <term>*)
    // <f-head> ::= <function-symbol>
    unsigned int Parser::parseFunctionHead(std::vector<Term>& fncParams, const std::vector<Variable>& parameters)
    {
        Token* token = syn->readSymbol(2, Symbol::NAME, Symbol::OPEN_PAR);
        unsigned int fncIndex;
        if(syn->isSym(token, Symbol::NAME))
        {
            fncIndex = task->getFunctionIndex(token->description);
            if(fncIndex == MAX_UNSIGNED_INT)
                syn->notifyError("Function '" + token->description + "' undefined");
            Function& f = task->functions[fncIndex];
            for(unsigned int i = 0; i < f.parameters.size(); i++)
            {
                Term term = parseTerm(f.parameters[i].types, parameters);
                fncParams.push_back(term);
            }
        }
        else
        {
            fncIndex = parseFunctionHead(fncParams, parameters);
            syn->closePar();
        }
        return fncIndex;
    }

    // <term> ::= <name>
    // <term> ::= <variable>
    Term Parser::parseTerm(const std::vector<unsigned int>& validTypes, const std::vector<Variable>& parameters)
    {
        Term term = parseTerm(parameters);
        if(term.isVariable)
        {
            if(!task->compatibleTypes(parameters[term.index].types, validTypes))
                syn->notifyError("Invalid parameter '" + parameters[term.index].name + "'");
        }
        else
        {
            Object& obj = task->objects[term.index];
            if(!task->compatibleTypes(obj.types, validTypes))
                syn->notifyError("Invalid parameter '" + obj.name + "'");
        }
        return term;
    }

    // <term> ::= <name>
    // <term> ::= <variable>
    Term Parser::parseTerm(const std::vector<Variable>& parameters)
    {
        Token* token = syn->readSymbol(2, Symbol::VARIABLE, Symbol::NAME);
        if(syn->isSym(token, Symbol::VARIABLE))
        {
            unsigned int paramIndex = MAX_UNSIGNED_INT;
            for(unsigned int i = 0; i < parameters.size(); i++)
                if(token->description.compare(parameters[i].name) == 0)
                {
                    paramIndex = i;
                    break;
                }
            if(paramIndex == MAX_UNSIGNED_INT)
                syn->notifyError("Variable '" + token->description + "' undeclared");
            return Term(true, paramIndex);
        }
        else
        {
            unsigned int objIndex = task->getObjectIndex(token->description);
            if(objIndex == MAX_UNSIGNED_INT)
                syn->notifyError("Constant '" + token->description + "' undeclared");
            // Object &obj = task->objects[objIndex];
            return Term(false, objIndex);
        }
    }

    // <da-GD> ::= ()
    // <da-GD> ::= <pref-timed-GD>
    // <da-GD> ::= (and <da-GD>*)
    // <da-GD> ::=:universal-preconditions (forall (<typed-list (variable)>) <da-GD>)
    // <pref-timed-GD> ::= <timed-GD>
    // <pref-timed-GD> ::=:preferences (preference [<pref-name>] <timed-GD>)
    // <timed-GD> ::= (at <time-specifier> <GD>)
    // <timed-GD> ::= (over <interval> <GD>)
    DurativeCondition Parser::parseDurativeCondition(const std::vector<Variable>& parameters)
    {
        syn->openPar();
        Token* token = syn->readSymbol(
            6, Symbol::CLOSE_PAR, Symbol::AND, Symbol::FORALL, Symbol::PREFERENCE, Symbol::AT, Symbol::OVER);
        DurativeCondition condition;
        if(syn->isSym(token, Symbol::CLOSE_PAR))
        {
            condition.type = ConditionType::CT_AND;
        }
        else if(syn->isSym(token, Symbol::AND))
        {
            condition.type = ConditionType::CT_AND;
            do
            {
                condition.conditions.push_back(parseDurativeCondition(parameters));
                token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::OPEN_PAR);
                if(syn->isSym(token, Symbol::OPEN_PAR))
                {
                    syn->undoToken();
                }
            } while(syn->isSym(token, Symbol::OPEN_PAR));
        }
        else if(syn->isSym(token, Symbol::AT))
        {
            condition.type = ConditionType::CT_GOAL;
            token          = syn->readSymbol(2, Symbol::START, Symbol::END);
            condition.goal.time =
                syn->isSym(token, Symbol::START) ? TimeSpecifier::k_at_start : TimeSpecifier::k_at_end;
            parseGoalDescription(condition.goal, parameters);
            syn->closePar();
        }
        else if(syn->isSym(token, Symbol::OVER))
        {
            condition.type = ConditionType::CT_GOAL;
            syn->readSymbol(Symbol::ALL);
            condition.goal.time = TimeSpecifier::k_over_all;
            parseGoalDescription(condition.goal, parameters);
            syn->closePar();
        }
        else if(syn->isSym(token, Symbol::FORALL))
        {
            condition.type = ConditionType::CT_FORALL;
            syn->openPar();
            parseVariableList(condition.parameters);
            syn->closePar();
            std::vector<Variable> mergedParameters;
            mergeVariables(mergedParameters, parameters, condition.parameters);
            condition.conditions.push_back(parseDurativeCondition(mergedParameters));
            syn->closePar();
        }
        else
        {  // preference
            condition.type           = ConditionType::CT_PREFERENCE;
            condition.preferenceName = syn->readName();
            syn->openPar();
            token = syn->readSymbol(2, Symbol::AT, Symbol::OVER);
            if(syn->isSym(token, Symbol::AT))
            {
                token = syn->readSymbol(2, Symbol::START, Symbol::END);
                condition.goal.time =
                    syn->isSym(token, Symbol::START) ? TimeSpecifier::k_at_start : TimeSpecifier::k_at_end;
            }
            else
            {
                condition.goal.time = TimeSpecifier::k_over_all;
            }
            parseGoalDescription(condition.goal, parameters);
            task->addPreference(condition.preferenceName, condition.goal, syn);
            syn->closePar();
            syn->closePar();
        }
        return condition;
    }

    void Parser::mergeVariables(std::vector<Variable>& mergedParameters,
                                const std::vector<Variable>& params1,
                                const std::vector<Variable>& params2)
    {
        for(unsigned int i = 0; i < params1.size(); i++)
        {
            mergedParameters.push_back(params1[i]);
        }
        for(unsigned int i = 0; i < params2.size(); i++)
        {
            for(unsigned int j = 0; j < mergedParameters.size(); j++)
            {
                if(params2[i].name.compare(mergedParameters[j].name) == 0)
                {
                    syn->notifyError("Duplicated parameter '" + params2[i].name + "'");
                }
            }
            mergedParameters.push_back(params2[i]);
        }
    }

    // <GD> ::= <atomic formula(term)>
    // <GD> ::=:negative-preconditions <literal(term)>
    // <GD> ::= (and <GD>*)
    // <GD> ::=:disjunctive-preconditions (or <GD>*)
    // <GD> ::=:disjunctive-preconditions (not <GD>)
    // <GD> ::=:disjunctive-preconditions (imply <GD> <GD>)
    // <GD> ::=:existential-preconditions (exists (<typed list(variable)>) <GD> )
    // <GD> ::=:universal-preconditions (forall (<typed list(variable)>) <GD> )
    // <GD> ::=:numeric-fluents <f-comp>
    // <literal(t)> ::= <atomic formula(t)>
    // <literal(t)> ::= (not <atomic formula(t)>)
    // <atomic formula(t)> ::= (<predicate> t*)
    // <atomic formula(t)> ::=:equality (= t t)
    // <f-comp> ::= (<binary-comp> <f-exp> <f-exp>)
    void Parser::parseGoalDescription(GoalDescription& goal, const std::vector<Variable> parameters)
    {
        syn->openPar();
        Token* token          = syn->readSymbol(12,
                                       Symbol::NOT,
                                       Symbol::AND,
                                       Symbol::OR,
                                       Symbol::IMPLY,
                                       Symbol::EXISTS,
                                       Symbol::FORALL,
                                       Symbol::EQUAL,
                                       Symbol::LESS,
                                       Symbol::GREATER,
                                       Symbol::GREATER_EQ,
                                       Symbol::LESS_EQ,
                                       Symbol::NAME);
        bool readTerms        = false;
        unsigned int numTerms = 0;
        switch(token->symbol)
        {
            case Symbol::NAME:  // <atomic formula(term)>
                syn->undoToken();
                goal.setLiteral(parseLiteral(parameters));
                break;
            case Symbol::NOT:
                goal.type = GD_NOT;
                readTerms = true;
                numTerms  = 1;
                break;
            case Symbol::AND:
                goal.type = GD_AND;
                readTerms = true;
                break;
            case Symbol::OR:
                goal.type = GD_OR;
                readTerms = true;
                break;
            case Symbol::IMPLY:
                goal.type = GD_IMPLY;
                readTerms = true;
                numTerms  = 2;
                break;
            case Symbol::EXISTS:
            case Symbol::FORALL:
                syn->undoToken();
                parseADLGoalDescription(goal, parameters);
                break;
            case Symbol::EQUAL:
            case Symbol::LESS:
            case Symbol::GREATER:
            case Symbol::GREATER_EQ:
            case Symbol::LESS_EQ:
                syn->undoToken();
                parseGoalDescriptionComparison(goal, parameters);
                break;
            default:;
        }
        if(readTerms)
        {
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            while(syn->isSym(token, Symbol::OPEN_PAR))
            {
                syn->undoToken();
                GoalDescription term;
                term.time = TimeSpecifier::k_none;
                parseGoalDescription(term, parameters);
                goal.terms.push_back(term);
                token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            }
            syn->undoToken();
            if(numTerms > 0 && goal.terms.size() != numTerms)
                syn->notifyError("Invalid number of conditions");
        }
        syn->closePar();
    }

    // <literal(t)> ::= <atomic formula(t)>
    // <literal(t)> ::= (not <atomic formula(t)>)
    Literal Parser::parseLiteral(const std::vector<Variable> parameters)
    {
        Literal literal;
        std::string name      = syn->readName();
        unsigned int fncIndex = task->getFunctionIndex(name);
        if(fncIndex == MAX_UNSIGNED_INT)
            syn->notifyError("Predicate '" + name + "' undefined");
        literal.fncIndex = fncIndex;
        Function& f      = task->functions[fncIndex];
        for(unsigned int i = 0; i < f.parameters.size(); i++)
        {
            Term term = parseTerm(f.parameters[i].types, parameters);
            literal.params.push_back(term);
        }
        return literal;
    }

    // <GD> ::=:existential-preconditions exists (<typed list(variable)>) <GD>
    // <GD> ::=:universal-preconditions forall (<typed list(variable)>) <GD>
    void Parser::parseADLGoalDescription(GoalDescription& goal, const std::vector<Variable> parameters)
    {
        Token* token = syn->readSymbol(2, Symbol::EXISTS, Symbol::FORALL);
        goal.type    = syn->isSym(token, Symbol::EXISTS) ? GD_EXISTS : GD_FORALL;
        syn->openPar();
        parseVariableList(goal.parameters);
        syn->closePar();
        std::vector<Variable> mergedParameters;
        mergeVariables(mergedParameters, parameters, goal.parameters);
        GoalDescription term;
        term.time = TimeSpecifier::k_none;
        parseGoalDescription(term, mergedParameters);
        goal.terms.push_back(term);
    }

    // <atomic formula(t)> ::=:equality = t t
    // <f-comp> ::= <binary-comp> <f-exp> <f-exp>
    void Parser::parseGoalDescriptionComparison(GoalDescription& goal, const std::vector<Variable> parameters)
    {
        Token* token =
            syn->readSymbol(5, Symbol::EQUAL, Symbol::LESS, Symbol::GREATER, Symbol::GREATER_EQ, Symbol::LESS_EQ);
        switch(token->symbol)
        {
            case Symbol::EQUAL:
                token = syn->nextToken();
                if(syn->isSym(token, Symbol::VARIABLE))
                {  // Equality
                    syn->undoToken();
                    goal.type = GD_EQUALITY;
                    goal.eqTerms.push_back(parseTerm(parameters));
                    goal.eqTerms.push_back(parseTerm(parameters));
                    return;
                }
                else
                {
                    goal.comparator = Comparator::k_eq;
                    syn->undoToken();
                }
                break;
            case Symbol::LESS:
                goal.comparator = Comparator::k_less;
                break;
            case Symbol::GREATER:
                goal.comparator = Comparator::k_greater;
                break;
            case Symbol::GREATER_EQ:
                goal.comparator = Comparator::k_greater_eq;
                break;
            case Symbol::LESS_EQ:
                goal.comparator = Comparator::k_less_eq;
                break;
            default:;
        }
        goal.type = GD_F_CMP;
        goal.exp.push_back(parseNumericExpression(parameters));
        goal.exp.push_back(parseNumericExpression(parameters));
    }

    // <da-effect> ::= (and <da-effect>*)
    // <da-effect> ::= <timed-effect>
    // <da-effect> ::=:conditional-effects (forall (<typed list (variable)>) <da-effect>)
    // <da-effect> ::=:conditional-effects (when <da-GD> <timed-effect>)
    DurativeEffect Parser::parseDurativeEffect(const std::vector<Variable>& parameters)
    {
        syn->openPar();
        Token* token = syn->readSymbol(
            6, Symbol::AND, Symbol::FORALL, Symbol::WHEN, Symbol::AT, Symbol::INCREASE, Symbol::DECREASE);
        DurativeEffect effect;
        std::vector<Variable> mergedParameters;
        switch(token->symbol)
        {
            case Symbol::AND:
                effect.type = DET_AND;
                token       = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                while(syn->isSym(token, Symbol::OPEN_PAR))
                {
                    syn->undoToken();
                    effect.terms.push_back(parseDurativeEffect(parameters));
                    token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                }
                syn->undoToken();
                break;
            case Symbol::AT:
                effect.type = DET_TIMED_EFFECT;
                token       = syn->readSymbol(2, Symbol::START, Symbol::END);
                effect.timedEffect.time =
                    syn->isSym(token, Symbol::END) ? TimeSpecifier::k_at_end : TimeSpecifier::k_at_start;
                parseTimedEffect(effect.timedEffect, parameters);
                break;
            case Symbol::FORALL:
                effect.type = DET_FORALL;
                syn->openPar();
                parseVariableList(effect.parameters);
                syn->closePar();
                mergeVariables(mergedParameters, parameters, effect.parameters);
                effect.terms.push_back(parseDurativeEffect(mergedParameters));
                break;
            case Symbol::WHEN:
                effect.type      = DET_WHEN;
                effect.condition = parseDurativeCondition(parameters);
                token            = syn->readSymbol(2, Symbol::START, Symbol::END);
                effect.timedEffect.time =
                    syn->isSym(token, Symbol::END) ? TimeSpecifier::k_at_end : TimeSpecifier::k_at_start;
                parseTimedEffect(effect.timedEffect, parameters);
                break;
            default:
                effect.type = DET_ASSIGNMENT;
                syn->undoToken();
                effect.assignment = parseAssignmentContinuousEffect(parameters);
        }
        syn->closePar();
        return effect;
    }

    // <timed-effect> ::=:continuous-effects + :numeric-fluents (<assign-op-t> <f-head> <f-exp-t>)
    // <assign-op-t> ::= increase
    // <assign-op-t> ::= decrease
    AssignmentContinuousEffect Parser::parseAssignmentContinuousEffect(const std::vector<Variable>& parameters)
    {
        Token* token = syn->readSymbol(2, Symbol::INCREASE, Symbol::DECREASE);
        AssignmentContinuousEffect effect;
        effect.type = syn->isSym(token, Symbol::INCREASE) ? Assignment::AS_INCREASE : Assignment::AS_DECREASE;
        syn->openPar();
        effect.fluent = parseLiteral(parameters);
        syn->closePar();
        effect.contEff = parseContinuousEffect(parameters);
        return effect;
    }

    // <f-exp-t> ::= (* <f-exp> #t)
    // <f-exp-t> ::= (* #t <f-exp>)
    // <f-exp-t> ::= #t
    ContinuousEffect Parser::parseContinuousEffect(const std::vector<Variable>& parameters)
    {
        ContinuousEffect eff;
        Token* token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::SHARP_T);
        if(syn->isSym(token, Symbol::SHARP_T))
            eff.product = false;
        else
        {
            eff.product = true;
            syn->readSymbol(Symbol::PROD);
            token = syn->nextToken();
            if(syn->isSym(token, Symbol::SHARP_T))
            {
                eff.numExp = parseNumericExpression(parameters);
            }
            else
            {
                syn->undoToken();
                eff.numExp = parseNumericExpression(parameters);
                syn->readSymbol(Symbol::SHARP_T);
            }
            syn->closePar();
        }
        return eff;
    }

    // <timed-effect> ::= <cond-effect>
    // <timed-effect> ::=:numeric-fluents <f-assign-da>
    // <timed-effect> ::=:continuous-effects + :numeric-fluents (<assign-op-t> <f-head> <f-exp-t>)
    // <f-assign-da> ::= (<assign-op> <f-head> <f-exp-da>)
    // <cond-effect> ::= (and <p-effect>*)
    // <cond-effect> ::= <p-effect>
    // <p-effect> ::= (not <atomic formula(term)>)
    // <p-effect> ::= <atomic formula(term)>
    // <p-effect> ::=:numeric-fluents (<assign-op> <f-head> <f-exp>)
    // <p-effect> ::=:object-fluents (assign <function-term> <term>)
    // <p-effect> ::=:object-fluents (assign <function-term> undefined)
    void Parser::parseTimedEffect(TimedEffect& timedEffect, const std::vector<Variable>& parameters)
    {
        syn->openPar();
        Token* token = syn->readSymbol(8,
                                       Symbol::AND,
                                       Symbol::NOT,
                                       Symbol::INCREASE,
                                       Symbol::DECREASE,
                                       Symbol::ASSIGN,
                                       Symbol::SCALE_UP,
                                       Symbol::SCALE_DOWN,
                                       Symbol::NAME);
        switch(token->symbol)
        {
            case Symbol::AND:
                timedEffect.type = TE_AND;
                token            = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                while(syn->isSym(token, Symbol::OPEN_PAR))
                {
                    syn->undoToken();
                    TimedEffect aux;
                    aux.time = TimeSpecifier::k_none;
                    parseTimedEffect(aux, parameters);
                    timedEffect.terms.push_back(aux);
                    token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                }
                syn->undoToken();
                break;
            case Symbol::NOT:
            {
                timedEffect.type = TE_NOT;
                TimedEffect aux;
                aux.time = TimeSpecifier::k_none;
                parseTimedEffect(aux, parameters);
                timedEffect.terms.push_back(aux);
            }
            break;
            case Symbol::NAME:
                timedEffect.type = TE_LITERAL;
                syn->undoToken();
                timedEffect.literal = parseLiteral(parameters);
                break;
            default:
                timedEffect.type = TE_ASSIGNMENT;
                syn->undoToken();
                timedEffect.assignment = parseFluentAssignment(parameters);
        }
        syn->closePar();
    }

    // <f-assign-da> ::= <assign-op> <f-head> <f-exp-da>
    // <timed-effect> ::=:continuous-effects + :numeric-fluents <assign-op-t> <f-head> <f-exp-t>
    // <p-effect> ::=:numeric-fluents <assign-op> <f-head> <f-exp>
    // <p-effect> ::=:object-fluents assign <function-term> <term>
    // <p-effect> ::=:object-fluents assign <function-term> undefined
    FluentAssignment Parser::parseFluentAssignment(const std::vector<Variable>& parameters)
    {
        Token* token = syn->readSymbol(
            5, Symbol::INCREASE, Symbol::DECREASE, Symbol::ASSIGN, Symbol::SCALE_UP, Symbol::SCALE_DOWN);
        FluentAssignment assignment;
        switch(token->symbol)
        {
            case Symbol::INCREASE:
                assignment.type = Assignment::AS_INCREASE;
                break;
            case Symbol::DECREASE:
                assignment.type = Assignment::AS_DECREASE;
                break;
            case Symbol::ASSIGN:
                assignment.type = Assignment::AS_ASSIGN;
                break;
            case Symbol::SCALE_UP:
                assignment.type = Assignment::AS_SCALE_UP;
                break;
            case Symbol::SCALE_DOWN:
                assignment.type = Assignment::AS_SCALE_DOWN;
                break;
            default:;
        }
        std::vector<Term> fncParams;
        assignment.fluent.fncIndex = parseFunctionHead(fncParams, parameters);
        for(unsigned int i = 0; i < fncParams.size(); i++)
            assignment.fluent.params.push_back(fncParams[i]);
        assignment.exp = parseEffectExpression(parameters, syn->isSym(token, Symbol::ASSIGN));
        return assignment;
    }

    // <f-exp-da> | <f-exp-t> | <f-exp> | <term> (if isAssign) | undefined (if isAssign)
    EffectExpression Parser::parseEffectExpression(const std::vector<Variable>& parameters, bool isAssign)
    {
        Token* token =
            syn->readSymbol(5, Symbol::NUMBER, Symbol::OPEN_PAR, Symbol::VARIABLE, Symbol::SHARP_T, Symbol::NAME);
        EffectExpression exp;
        switch(token->symbol)
        {
            case Symbol::NUMBER:
                exp.type  = EE_NUMBER;
                exp.value = token->value;
                break;
            case Symbol::VARIABLE:
            case Symbol::NAME:
                if(token->description.compare("?duration") == 0)
                    exp.type = EE_DURATION;
                else if(!isAssign)
                    syn->notifyError("Invalid assignment to 'undefined'");
                else if(token->description.compare("undefined") == 0)
                    exp.type = EE_UNDEFINED;
                else
                {
                    exp.type = EE_TERM;
                    syn->undoToken();
                    exp.term = parseTerm(parameters);
                }
                break;
            case Symbol::SHARP_T:
                exp.type = EE_SHARP_T;
                break;
            case Symbol::OPEN_PAR:
                parseEffectOperation(exp, parameters, isAssign);
                syn->closePar();
                break;
            default:;
        }
        return exp;
    }

    // <f-exp-da> ::= <binary-op> <f-exp-da> <f-exp-da>
    // <f-exp-da> ::= <multi-op> <f-exp-da> <f-exp-da>+
    // <f-exp-da> ::= - <f-exp-da>
    // <f-exp-t> ::= * <f-exp> #t
    // <f-exp-t> ::= * #t <f-exp>
    void Parser::parseEffectOperation(EffectExpression& exp, const std::vector<Variable>& parameters, bool isAssign)
    {
        Token* token = syn->readSymbol(5, Symbol::MINUS, Symbol::PLUS, Symbol::PROD, Symbol::DIV, Symbol::NAME);
        if(syn->isSym(token, Symbol::NAME))
        {
            exp.type = EE_FLUENT;
            syn->undoToken();
            exp.fluent = parseLiteral(parameters);
        }
        else
        {
            exp.type = EE_OPERATION;
            switch(token->symbol)
            {
                case Symbol::MINUS:
                    exp.operation = OT_SUB;
                    break;
                case Symbol::PLUS:
                    exp.operation = OT_SUM;
                    break;
                case Symbol::PROD:
                    exp.operation = OT_MUL;
                    break;
                default:
                    exp.operation = OT_DIV;
            }
            bool moreOperands;
            do
            {
                Token* aux   = syn->nextToken();
                moreOperands = syn->isSym(aux, Symbol::NUMBER) || syn->isSym(aux, Symbol::OPEN_PAR) ||
                               syn->isSym(aux, Symbol::NAME) || syn->isSym(aux, Symbol::VARIABLE) ||
                               syn->isSym(aux, Symbol::SHARP_T);
                syn->undoToken();
                if(moreOperands)
                {
                    exp.operands.push_back(parseEffectExpression(parameters, isAssign));
                }
            } while(moreOperands);
        }
    }

    // <action-def> ::= (:action <action-symbol>
    //                  :parameters (<typed list (variable)>)
    //                  <action-def body>)
    // <action-def body> ::= [:precondition <emptyOr (pre-GD)>]
    //                       [:effect <emptyOr (effect)>]
    void Parser::parseAction()
    {
        std::string name = syn->readName();
        syn->readColon();
        syn->readSymbol(Symbol::PARAMETERS);
        syn->openPar();
        std::vector<Variable> parameters;
        parseVariableList(parameters);
        syn->closePar();
        Precondition precondition;
        Effect effect;
        Token* token = syn->readSymbol(2, Symbol::COLON, Symbol::CLOSE_PAR);
        if(syn->isSym(token, Symbol::COLON))
        {
            token = syn->readSymbol(2, Symbol::PRECONDITION, Symbol::EFFECT);
            if(syn->isSym(token, Symbol::PRECONDITION))
            {
                precondition = parsePrecondition(parameters);
                token        = syn->readSymbol(2, Symbol::COLON, Symbol::CLOSE_PAR);
                if(syn->isSym(token, Symbol::COLON))
                {
                    syn->readSymbol(Symbol::EFFECT);
                    effect = parseEffect(parameters);
                    syn->closePar();
                }
            }
            else
            {                                // No precondition
                precondition.type = PT_AND;  // AND without terms
                effect            = parseEffect(parameters);
                syn->closePar();
            }
        }
        task->addAction(name, parameters, precondition, effect, syn);
    }

    // <pre-GD> ::= <pref-GD>
    // <pre-GD> ::= (and <pre-GD>*)
    // <pre-GD> ::=:universal-preconditions (forall (<typed list(variable)>) <pre-GD>)
    // <pref-GD> ::=:preferences (preference [<pref-name>] <GD>)
    // <pref-GD> ::= <GD>
    Precondition Parser::parsePrecondition(const std::vector<Variable>& parameters)
    {
        Precondition prec;
        syn->openPar();
        Token* token          = syn->readSymbol(13,
                                       Symbol::NOT,
                                       Symbol::AND,
                                       Symbol::OR,
                                       Symbol::IMPLY,
                                       Symbol::EXISTS,
                                       Symbol::FORALL,
                                       Symbol::EQUAL,
                                       Symbol::LESS,
                                       Symbol::GREATER,
                                       Symbol::GREATER_EQ,
                                       Symbol::LESS_EQ,
                                       Symbol::PREFERENCE,
                                       Symbol::NAME);
        bool readTerms        = false;
        unsigned int numTerms = 0;
        switch(token->symbol)
        {
            case Symbol::NAME:  // <atomic formula(term)>
                syn->undoToken();
                prec.type    = PT_LITERAL;
                prec.literal = parseLiteral(parameters);
                break;
            case Symbol::NOT:
                prec.type = PT_NOT;
                readTerms = true;
                numTerms  = 1;
                break;
            case Symbol::AND:
                prec.type = PT_AND;
                readTerms = true;
                break;
            case Symbol::OR:
                prec.type = PT_OR;
                readTerms = true;
                break;
            case Symbol::IMPLY:
                prec.type = PT_IMPLY;
                readTerms = true;
                numTerms  = 2;
                break;
            case Symbol::EXISTS:
            case Symbol::FORALL:
            {
                prec.type = syn->isSym(token, Symbol::EXISTS) ? PT_EXISTS : PT_FORALL;
                syn->openPar();
                parseVariableList(prec.parameters);
                syn->closePar();
                std::vector<Variable> mergedParameters;
                mergeVariables(mergedParameters, parameters, prec.parameters);
                prec.terms.push_back(parsePrecondition(mergedParameters));
            }
            break;
            case Symbol::EQUAL:
            case Symbol::LESS:
            case Symbol::GREATER:
            case Symbol::GREATER_EQ:
            case Symbol::LESS_EQ:
                syn->undoToken();
                prec.goal.time = TimeSpecifier::k_none;
                parseGoalDescriptionComparison(prec.goal, parameters);
                if(prec.goal.type == GD_EQUALITY)
                    prec.type = PT_EQUALITY;
                else
                    prec.type = PT_F_CMP;
                break;
            case Symbol::PREFERENCE:
                prec.type           = PT_PREFERENCE;
                prec.preferenceName = syn->readName();
                prec.goal.time      = TimeSpecifier::k_none;
                parseGoalDescription(prec.goal, parameters);
                task->addPreference(prec.preferenceName, prec.goal, syn);
                break;
            default:;
        }
        if(readTerms)
        {
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            while(syn->isSym(token, Symbol::OPEN_PAR))
            {
                syn->undoToken();
                prec.terms.push_back(parsePrecondition(parameters));
                token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
            }
            syn->undoToken();
            if(numTerms > 0 && prec.terms.size() != numTerms)
                syn->notifyError("Invalid number of conditions");
        }
        syn->closePar();
        return prec;
    }

    // <effect> ::= (and <c-effect>*)
    // <effect> ::= <c-effect>
    // <c-effect> ::=:conditional-effects (forall (<typed list (variable)>) <effect>)
    // <c-effect> ::=:conditional-effects (when <GD> <cond-effect>)
    // <c-effect> ::= <p-effect>
    // <p-effect> ::= (not <atomic formula(term)>)
    // <p-effect> ::= <atomic formula(term)>
    // <p-effect> ::=:numeric-fluents (<assign-op> <f-head> <f-exp>)
    // <p-effect> ::=:object-fluents (assign <function-term> <term>)
    // <p-effect> ::=:object-fluents (assign <function-term> undefined)
    Effect Parser::parseEffect(const std::vector<Variable>& parameters)
    {
        Effect eff;
        syn->openPar();
        Token* token = syn->readSymbol(10,
                                       Symbol::AND,
                                       Symbol::NOT,
                                       Symbol::INCREASE,
                                       Symbol::DECREASE,
                                       Symbol::ASSIGN,
                                       Symbol::SCALE_UP,
                                       Symbol::SCALE_DOWN,
                                       Symbol::FORALL,
                                       Symbol::WHEN,
                                       Symbol::NAME);
        switch(token->symbol)
        {
            case Symbol::AND:
                eff.type = ET_AND;
                token    = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                while(syn->isSym(token, Symbol::OPEN_PAR))
                {
                    syn->undoToken();
                    eff.terms.push_back(parseEffect(parameters));
                    token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                }
                syn->undoToken();
                break;
            case Symbol::NOT:
                eff.type = ET_NOT;
                eff.terms.push_back(parseEffect(parameters));
                break;
            case Symbol::NAME:
                eff.type = ET_LITERAL;
                syn->undoToken();
                eff.literal = parseLiteral(parameters);
                break;
            case Symbol::WHEN:
                eff.type      = ET_WHEN;
                eff.goal.time = TimeSpecifier::k_none;
                parseGoalDescription(eff.goal, parameters);
                eff.terms.push_back(parseEffect(parameters));
                break;
            case Symbol::FORALL:
            {
                eff.type = ET_FORALL;
                syn->openPar();
                parseVariableList(eff.parameters);
                syn->closePar();
                std::vector<Variable> mergedParameters;
                mergeVariables(mergedParameters, parameters, eff.parameters);
                eff.terms.push_back(parseEffect(mergedParameters));
            }
            break;
            default:
                eff.type = ET_ASSIGNMENT;
                syn->undoToken();
                eff.assignment = parseFluentAssignment(parameters);
        }
        syn->closePar();
        return eff;
    }

    // <object declaration> ::= (:objects <typed list (name)>)
    void Parser::parseObjects()
    {
        Token* token = syn->readSymbol(2, Symbol::NAME, Symbol::CLOSE_PAR);
        std::vector<std::string> objNames;
        std::vector<unsigned int> types;
        while(!syn->isSym(token, Symbol::CLOSE_PAR))
        {
            while(syn->isSym(token, Symbol::NAME))
            {
                objNames.push_back(token->description);
                token = syn->readSymbol(3, Symbol::NAME, Symbol::MINUS, Symbol::CLOSE_PAR);
            }
            if(syn->isSym(token, Symbol::MINUS))
            {
                parseParentTypes(types, false);
                token = syn->readSymbol(2, Symbol::NAME, Symbol::CLOSE_PAR);
            }
            else
                types.push_back(task->getTypeIndex("#object"));
            for(unsigned int i = 0; i < objNames.size(); i++)
                task->addObject(objNames[i], types, syn);
            objNames.clear();
            types.clear();
        }
    }

    // <init> ::= (:init <init-el>*)
    // <init-el> ::= <literal(name)>
    // <init-el> ::=:timed-initial-literals (at <number> <literal(name)>)
    // <init-el> ::=:numeric-fluents (= <basic-function-term> <number>)
    // <init-el> ::=:object-fluents (= <basic-function-term> <name>)
    // <basic-function-term> ::= <function-symbol>
    // <basic-function-term> ::= (<function-symbol> <name>*)
    void Parser::parseInit()
    {
        Token* token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        float time;
        while(syn->isSym(token, Symbol::OPEN_PAR))
        {
            token = syn->readSymbol(4, Symbol::EQUAL, Symbol::AT, Symbol::OVER, Symbol::NAME);
            Fact f;
            if(syn->isSym(token, Symbol::EQUAL))
            {
                f = parseFact();
                syn->closePar();
            }
            else
            {
                bool til = false;
                if(syn->isSym(token, Symbol::AT))
                {
                    Token* aux = syn->nextToken();
                    if(syn->isSym(aux, Symbol::NUMBER))
                    {
                        til = true;
                    }
                    syn->undoToken();
                }
                if(!til)
                {
                    syn->undoToken();
                    f = parseFact();
                }
                else
                {
                    time = syn->readSymbol(Symbol::NUMBER)->value;
                    syn->openPar();
                    token = syn->nextToken();
                    if(syn->isSym(token, Symbol::NOT))
                    {
                        syn->openPar();
                        f = parseFact();
                        syn->closePar();
                        f.value = f.value == task->CONSTANT_TRUE ? task->CONSTANT_FALSE : task->CONSTANT_TRUE;
                    }
                    else
                    {
                        syn->undoToken();
                        f = parseFact();
                    }
                    f.time = time;
                    syn->closePar();
                }
                syn->closePar();
            }
            task->init.push_back(f);
            token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
        }
    }

    // <literal(name)> | <basic-function-term> <number> | <basic-function-term> <name>
    Fact Parser::parseFact()
    {
        Fact fact;
        fact.time           = 0;
        fact.valueIsNumeric = false;
        bool closePar       = false;
        Token* token        = syn->readSymbol(4, Symbol::OPEN_PAR, Symbol::AT, Symbol::OVER, Symbol::NAME);
        if(syn->isSym(token, Symbol::OPEN_PAR))
        {
            closePar = true;
            token    = syn->readSymbol(3, Symbol::AT, Symbol::OVER, Symbol::NAME);
        }
        fact.function = task->getFunctionIndex(token->description);
        if(fact.function == MAX_UNSIGNED_INT)
            syn->notifyError("Function '" + token->description + "' undefined");
        Function& function     = task->functions[fact.function];
        unsigned int numParams = function.parameters.size();
        for(unsigned int i = 0; i < numParams; i++)
        {
            std::string param     = syn->readName();
            unsigned int objIndex = task->getObjectIndex(param);
            if(objIndex == MAX_UNSIGNED_INT)
                syn->notifyError("Object '" + param + "' undefined");
            Object& obj = task->objects[objIndex];
            if(!task->compatibleTypes(obj.types, function.parameters[i].types))
                syn->notifyError("Invalid parameter '" + obj.name + "'");
            fact.parameters.push_back(objIndex);
        }
        if(closePar)
        {  // Fluent
            syn->closePar();
            token = syn->readSymbol(2, Symbol::NAME, Symbol::NUMBER);
            if(syn->isSym(token, Symbol::NAME))
            {
                unsigned int objIndex = task->getObjectIndex(token->description);
                if(objIndex == MAX_UNSIGNED_INT)
                    syn->notifyError("Object '" + token->description + "' undefined");
                Object& obj = task->objects[objIndex];
                if(!task->compatibleTypes(obj.types, function.valueTypes))
                    syn->notifyError("Invalid value '" + obj.name + "'");
                fact.value = objIndex;
            }
            else
            {
                if(!task->isNumericFunction(fact.function))
                    syn->notifyError("'" + function.name + "' is not a numeric function");
                fact.valueIsNumeric = true;
                fact.numericValue   = token->value;
            }
        }
        else
        {  // boolean fact/literal
            if(!task->isBooleanFunction(fact.function))
                syn->notifyError("'" + function.name + "' is not a predicate");
            fact.value = task->CONSTANT_TRUE;
        }
        return fact;
    }

    // <goal> ::= (:goal <pre-GD>)
    void Parser::parseGoal()
    {
        std::vector<Variable> parameters;
        task->goal = parsePrecondition(parameters);
        syn->closePar();
    }

    // <metric-spec> ::=:numeric-fluents (:metric <optimization> <metric-f-exp>)
    // <optimization> ::= minimize
    // <optimization> ::= maximize
    void Parser::parseMetric()
    {
        Token* token     = syn->readSymbol(2, Symbol::MINIMIZE, Symbol::MAXIMIZE);
        task->metricType = syn->isSym(token, Symbol::MINIMIZE) ? MT_MINIMIZE : MT_MAXIMIZE;
        task->metric     = parseMetricExpression();
        syn->closePar();
    }

    // <metric-f-exp> ::= (<binary-op> <metric-f-exp> <metric-f-exp>)
    // <metric-f-exp> ::= (<multi-op> <metric-f-exp> <metric-f-exp>+)
    // <metric-f-exp> ::= (- <metric-f-exp>)
    // <metric-f-exp> ::= <number>
    // <metric-f-exp> ::= (<function-symbol> <name>*)
    // <metric-f-exp> ::= <function-symbol>
    // <metric-f-exp> ::= total-time
    // <metric-f-exp> ::=:preferences (is-violated <pref-name>)
    Metric Parser::parseMetricExpression()
    {
        Metric metric;
        Token* token = syn->readSymbol(9,
                                       Symbol::OPEN_PAR,
                                       Symbol::MINUS,
                                       Symbol::DIV,
                                       Symbol::PROD,
                                       Symbol::PLUS,
                                       Symbol::NUMBER,
                                       Symbol::TOTAL_TIME,
                                       Symbol::IS_VIOLATED,
                                       Symbol::NAME);
        switch(token->symbol)
        {
            case Symbol::OPEN_PAR:
                metric = parseMetricExpression();
                syn->closePar();
                break;
            case Symbol::MINUS:
            case Symbol::DIV:
            case Symbol::PROD:
            case Symbol::PLUS:
            {
                unsigned int minTerms = 2, maxTerms = 2;
                if(syn->isSym(token, Symbol::PLUS))
                {
                    maxTerms    = MAX_UNSIGNED_INT;
                    metric.type = MT_PLUS;
                }
                else if(syn->isSym(token, Symbol::MINUS))
                {
                    minTerms    = 1;
                    metric.type = MT_MINUS;
                }
                else if(syn->isSym(token, Symbol::PROD))
                {
                    maxTerms    = MAX_UNSIGNED_INT;
                    metric.type = MT_PROD;
                }
                else
                    metric.type = MT_DIV;
                token = syn->nextToken();
                while(!syn->isSym(token, Symbol::CLOSE_PAR))
                {
                    syn->undoToken();
                    metric.terms.push_back(parseMetricExpression());
                    token = syn->nextToken();
                }
                syn->undoToken();
                if(metric.terms.size() < minTerms || metric.terms.size() > maxTerms)
                    syn->notifyError("Invalid number of operands");
            }
            break;
            case Symbol::NUMBER:
                metric.type  = MT_NUMBER;
                metric.value = token->value;
                break;
            case Symbol::NAME:
                metric.type = MT_FLUENT;
                syn->undoToken();
                metric.function = parseFluent(metric.parameters);
                break;
            case Symbol::TOTAL_TIME:
                metric.type = MT_TOTAL_TIME;
                break;
            case Symbol::IS_VIOLATED:
                metric.type           = MT_IS_VIOLATED;
                metric.preferenceName = syn->readName();
                if(task->getPreferenceIndex(metric.preferenceName) == MAX_UNSIGNED_INT)
                    syn->notifyError("Preference '" + metric.preferenceName + "' undefined");
                break;
            default:;
        }
        return metric;
    }

    // <function-symbol> <name>*
    unsigned int Parser::parseFluent(std::vector<unsigned int>& parameters)
    {
        std::string functionName   = syn->readName();
        unsigned int functionIndex = task->getFunctionIndex(functionName);
        if(functionIndex == MAX_UNSIGNED_INT || !task->isNumericFunction(functionIndex))
            syn->notifyError("Numeric Function '" + functionName + "' undefined");
        Function& function = task->functions[functionIndex];
        for(unsigned int i = 0; i < function.parameters.size(); i++)
        {
            std::string objectName   = syn->readName();
            unsigned int objectIndex = task->getObjectIndex(objectName);
            if(objectIndex == MAX_UNSIGNED_INT)
                syn->notifyError("Object '" + objectName + "' undefined");
            if(!task->compatibleTypes(task->objects[objectIndex].types, function.parameters[i].types))
                syn->notifyError("Invalid parameter '" + objectName + "'");
            parameters.push_back(objectIndex);
        }
        return functionIndex;
    }

    // <constraints> ::=:constraints (:constraints <con-GD>)
    // <constraints> ::=:constraints (:constraints <pref-con-GD>)
    void Parser::parseConstraints()
    {
        std::vector<Variable> parameters;
        task->constraints.push_back(parseConstraint(parameters));
    }

    // <pref-con-GD> ::= (and <pref-con-GD>*)
    // <pref-con-GD> ::=:universal-preconditions (forall (<typed list (variable)>) <pref-con-GD>)
    // <pref-con-GD> ::=:preferences (preference [<pref-name>] <con-GD>)
    // <pref-con-GD> ::= <con-GD>
    // <con-GD> ::= (and <con-GD>*)
    // <con-GD> ::= (forall (<typed list (variable)>) <con-GD>)
    // <con-GD> ::= (at end <GD>)
    // <con-GD> ::= (always <GD>)
    // <con-GD> ::= (sometime <GD>)
    // <con-GD> ::= (within <number> <GD>)
    // <con-GD> ::= (at-most-once <GD>)
    // <con-GD> ::= (sometime-after <GD> <GD>)
    // <con-GD> ::= (sometime-before <GD> <GD>)
    // <con-GD> ::= (always-within <number> <GD> <GD>)
    // <con-GD> ::= (hold-during <number> <number> <GD>)
    // <con-GD> ::= (hold-after <number> <GD>)
    Constraint Parser::parseConstraint(const std::vector<Variable>& parameters)
    {
        syn->openPar();
        Token* token = syn->readSymbol(13,
                                       Symbol::AND,
                                       Symbol::FORALL,
                                       Symbol::PREFERENCE,
                                       Symbol::AT,
                                       Symbol::ALWAYS,
                                       Symbol::SOMETIME,
                                       Symbol::WITHIN,
                                       Symbol::AT_MOST_ONCE,
                                       Symbol::SOMETIME_AFTER,
                                       Symbol::SOMETIME_BEFORE,
                                       Symbol::ALWAYS_WITHIN,
                                       Symbol::HOLD_DURING,
                                       Symbol::HOLD_AFTER);
        Constraint c;
        switch(token->symbol)
        {
            case Symbol::AND:
                c.type = RT_AND;
                token  = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                while(syn->isSym(token, Symbol::OPEN_PAR))
                {
                    syn->undoToken();
                    c.terms.push_back(parseConstraint(parameters));
                    token = syn->readSymbol(2, Symbol::OPEN_PAR, Symbol::CLOSE_PAR);
                }
                break;
            case Symbol::FORALL:
            {
                c.type = RT_FORALL;
                syn->openPar();
                parseVariableList(c.parameters);
                syn->closePar();
                std::vector<Variable> mergedParameters;
                mergeVariables(mergedParameters, parameters, c.parameters);
                c.terms.push_back(parseConstraint(mergedParameters));
            }
            break;
            case Symbol::PREFERENCE:
                c.type           = RT_PREFERENCE;
                c.preferenceName = syn->readName();
                c.terms.push_back(parseConstraint(parameters));
                task->addPreference(c, syn);
                break;
            case Symbol::AT:
                c.type = RT_AT_END;
                syn->readSymbol(Symbol::END);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::ALWAYS:
                c.type = RT_ALWAYS;
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::SOMETIME:
                c.type = RT_SOMETIME;
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::WITHIN:
                c.type = RT_WITHIN;
                token  = syn->readSymbol(Symbol::NUMBER);
                c.time.push_back(token->value);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::AT_MOST_ONCE:
                c.type = RT_AT_MOST_ONCE;
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::SOMETIME_AFTER:
                c.type = RT_SOMETIME_AFTER;
                parseConstraintGoal(c, parameters);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::SOMETIME_BEFORE:
                c.type = RT_SOMETIME_BEFORE;
                parseConstraintGoal(c, parameters);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::ALWAYS_WITHIN:
                c.type = RT_ALWAYS_WITHIN;
                token  = syn->readSymbol(Symbol::NUMBER);
                c.time.push_back(token->value);
                parseConstraintGoal(c, parameters);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::HOLD_DURING:
                c.type = RT_HOLD_DURING;
                token  = syn->readSymbol(Symbol::NUMBER);
                c.time.push_back(token->value);
                token = syn->readSymbol(Symbol::NUMBER);
                c.time.push_back(token->value);
                parseConstraintGoal(c, parameters);
                break;
            case Symbol::HOLD_AFTER:
                c.type = RT_HOLD_AFTER;
                token  = syn->readSymbol(Symbol::NUMBER);
                c.time.push_back(token->value);
                parseConstraintGoal(c, parameters);
                break;
            default:;
        }
        syn->closePar();
        return c;
    }

    // Adds a <GD> to the given constraint
    void Parser::parseConstraintGoal(Constraint& constraint, const std::vector<Variable>& parameters)
    {
        GoalDescription g;
        g.time = TimeSpecifier::k_none;
        parseGoalDescription(g, parameters);
        constraint.goal.push_back(g);
    }

    // <derived-def> ::= (:derived <atomic formula skeleton> <GD>)
    void Parser::parseDerivedPredicates()
    {
        DerivedPredicate d;
        d.function = parsePredicate();
        parseGoalDescription(d.goal, d.function.parameters);
        syn->closePar();
        task->derivedPredicates.push_back(d);
    }

    // <length-spec> ::= (:length [(:serial <integer>)] [(:parallel <integer>)])
    void Parser::parseLength()
    {
        Token* token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::OPEN_PAR);
        while(syn->isSym(token, Symbol::OPEN_PAR))
        {
            syn->readColon();
            std::string key = syn->readName();
            if(key.compare("serial") == 0)
            {
                token              = syn->readSymbol(Symbol::NUMBER);
                task->serialLength = (int)token->value;
            }
            else if(key.compare("parallel") == 0)
            {
                token                = syn->readSymbol(Symbol::NUMBER);
                task->parallelLength = (int)token->value;
            }
            else
                syn->notifyError("Unexpected keyword '" + key + "'");
            syn->closePar();
            token = syn->readSymbol(2, Symbol::CLOSE_PAR, Symbol::OPEN_PAR);
        }
    }
}  // namespace grstaps
