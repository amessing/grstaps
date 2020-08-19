#ifndef GRSTAPS_PARSER_HPP
#define GRSTAPS_PARSER_HPP

#include "grstaps/task_planning/parsed_task.hpp"
#include "grstaps/task_planning/syntax_analyzer.hpp"

namespace grstaps
{
    class Parser
    {
       private:
        SyntaxAnalyzer *syn;
        ParsedTask *task;
        void parseRequirements();
        void parseTypes();
        void parseParentTypes(std::vector<unsigned int> &types, bool allowNumber);
        void parseConstants();
        void parsePredicates();
        Function parsePredicate();
        void parseVariableList(std::vector<Variable> &parameters);
        void parseFunctions();
        void parseDurativeAction();
        void parseDurationConstraint(std::vector<Duration> &duration, const std::vector<Variable> &parameters);
        NumericExpression parseNumericExpression(const std::vector<Variable> &parameters);
        unsigned int parseFunctionHead(std::vector<Term> &fncParams, const std::vector<Variable> &parameters);
        Term parseTerm(const std::vector<unsigned int> &validTypes, const std::vector<Variable> &parameters);
        Term parseTerm(const std::vector<Variable> &parameters);
        DurativeCondition parseDurativeCondition(const std::vector<Variable> &parameters);
        void parseGoalDescription(GoalDescription &goal, const std::vector<Variable> parameters);
        Literal parseLiteral(const std::vector<Variable> parameters);
        void mergeVariables(std::vector<Variable> &mergedParameters,
                            const std::vector<Variable> &params1,
                            const std::vector<Variable> &params2);
        void parseADLGoalDescription(GoalDescription &goal, const std::vector<Variable> parameters);
        void parseGoalDescriptionComparison(GoalDescription &goal, const std::vector<Variable> parameters);
        DurativeEffect parseDurativeEffect(const std::vector<Variable> &parameters);
        void parseTimedEffect(TimedEffect &timedEffect, const std::vector<Variable> &parameters);
        ContinuousEffect parseContinuousEffect(const std::vector<Variable> &parameters);
        AssignmentContinuousEffect parseAssignmentContinuousEffect(const std::vector<Variable> &parameters);
        FluentAssignment parseFluentAssignment(const std::vector<Variable> &parameters);
        EffectExpression parseEffectExpression(const std::vector<Variable> &parameters, bool isAssign);
        void parseEffectOperation(EffectExpression &exp, const std::vector<Variable> &parameters, bool isAssign);
        void parseAction();
        void parseConstraints();
        Constraint parseConstraint(const std::vector<Variable> &parameters);
        void parseConstraintGoal(Constraint &constraint, const std::vector<Variable> &parameters);
        void parseDerivedPredicates();
        Precondition parsePrecondition(const std::vector<Variable> &parameters);
        Effect parseEffect(const std::vector<Variable> &parameters);
        void parseObjects();
        void parseInit();
        Fact parseFact();
        void parseGoal();
        void parseMetric();
        Metric parseMetricExpression();
        unsigned int parseFluent(std::vector<unsigned int> &parameters);
        void parseLength();

       public:
        Parser();
        ~Parser();
        ParsedTask *parseDomain(char *domainFileName);
        ParsedTask *parseProblem(char *problemFileName);
    };
}  // namespace grstaps
#endif  // GRSTAPS_PARSER_HPP
