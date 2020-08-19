#ifndef GRSTAP_PREPROCESS_HPP
#define GRSTAP_PREPROCESS_HPP

#include "grstaps/task_planning/parsed_task.hpp"
#include "grstaps/task_planning/preprocessed_task.hpp"

namespace grstaps
{
    struct FeatureList
    {
        int universalQuantifierPrec;
        int existentialQuantifierPrec;
        int implicationPrec;
        int disjunctionPrec;
        int universalQuantifierEff;
        int existentialQuantifierEff;
        int implicationEff;
        int disjunctionEff;
        int conditionalEff;
    };

    class Preprocess
    {
       private:
        ParsedTask *task;
        PreprocessedTask *prepTask;
        void preprocessOperators();
        void checkPreconditionFeatures(Precondition &prec, FeatureList *features);
        void checkPreconditionFeatures(DurativeCondition &prec, FeatureList *features);
        void checkGoalFeatures(GoalDescription &goal, FeatureList *features);
        void checkGoalFeatures(DurativeCondition &goal, FeatureList *features);
        void checkEffectFeatures(Effect &eff, FeatureList *features);
        void checkEffectFeatures(DurativeEffect &eff, FeatureList *features);
        void preprocessAction(Action a, FeatureList *features, bool isGoal);
        void preprocessAction(DurativeAction a, FeatureList *features, bool isGoal);
        void removeQuantifiers(Precondition &prec, unsigned int numParameters);
        void removeQuantifiers(DurativeCondition &prec, unsigned int numParameters);
        void removeQuantifiers(Effect &eff, unsigned int numParameters);
        void removeQuantifiers(DurativeEffect &eff, unsigned int numParameters);
        void removeQuantifiers(GoalDescription &goal, unsigned int numParameters);
        void replaceQuantifierParameter(Precondition &prec,
                                        Precondition term,
                                        unsigned int paramNumber,
                                        unsigned int numParameters);
        void replaceQuantifierParameter(DurativeCondition &prec,
                                        DurativeCondition term,
                                        unsigned int paramNumber,
                                        unsigned int numParameters);
        void replaceQuantifierParameter(Effect &eff, Effect term, unsigned int paramNumber, unsigned int numParameters);
        void replaceQuantifierParameter(DurativeEffect &eff,
                                        DurativeEffect term,
                                        unsigned int paramNumber,
                                        unsigned int numParameters);
        void replaceQuantifierParameter(GoalDescription &goal,
                                        GoalDescription term,
                                        unsigned int paramNumber,
                                        unsigned int numParameters);
        void replaceParameter(Precondition &term, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(DurativeCondition &term, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(Literal &literal, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(GoalDescription &goal, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(Term &term, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(NumericExpression &exp, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(FluentAssignment &fa, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(EffectExpression &exp, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(Effect &eff, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(DurativeEffect &eff, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(TimedEffect &eff, unsigned int paramToReplace, unsigned int objectIndex);
        void replaceParameter(AssignmentContinuousEffect &eff, unsigned int paramToReplace, unsigned int objectIndex);
        void removeImplications(Precondition &prec);
        void removeImplications(DurativeCondition &prec);
        void removeImplications(Effect &eff);
        void removeImplications(DurativeEffect &eff);
        void removeImplications(GoalDescription &goal);
        void preconditionOptimization(Precondition *prec, Precondition *parent, unsigned int termNumber, Action &a);
        void preconditionOptimization(DurativeCondition *prec,
                                      DurativeCondition *parent,
                                      unsigned int termNumber,
                                      DurativeAction &a);
        void preconditionOptimization(DurativeCondition *prec,
                                      DurativeCondition *parent,
                                      unsigned int termNumber,
                                      DurativeEffect *parentEff);
        void negationOptimization(Precondition *prec, Precondition *parent, unsigned int termNumber, Action &a);
        Comparator negateComparator(Comparator c);
        void effectOptimization(Effect *eff, Effect *parent, unsigned int termNumber, Action &a);
        void effectOptimization(DurativeEffect *eff,
                                DurativeEffect *parent,
                                unsigned int termNumber,
                                DurativeAction &a);
        void effectOptimization(TimedEffect *eff,
                                TimedEffect *parent,
                                unsigned int termNumber,
                                DurativeEffect *parentEff);
        void negationOptimization(Effect *eff, Effect *parent, unsigned int termNumber, Action &a);
        void negationOptimization(TimedEffect *eff,
                                  TimedEffect *parent,
                                  unsigned int termNumber,
                                  DurativeEffect *parentEff);
        void goalOptimization(GoalDescription *goal,
                              Precondition *parentPrec,
                              Effect *parentEff,
                              GoalDescription *parent,
                              unsigned int termNumber);
        void goalOptimization(GoalDescription *goal,
                              DurativeCondition *parentPrec,
                              GoalDescription *parent,
                              unsigned int termNumber);
        void negationOptimization(GoalDescription *goal,
                                  Precondition *parentPrec,
                                  Effect *parentEff,
                                  GoalDescription *parent,
                                  unsigned int termNumber);
        void negationOptimization(GoalDescription *goal,
                                  DurativeCondition *parentPrec,
                                  GoalDescription *parent,
                                  unsigned int termNumber);
        void removeConditionalEffects(Action a, std::vector<Action> &aList);
        void removeConditionalEffects(DurativeAction a, std::vector<DurativeAction> &aList);
        void ignoreConditionalEffect(Action a, std::vector<Action> &aList);
        void ignoreConditionalEffect(DurativeAction a, std::vector<DurativeAction> &aList);
        void considerConditionalEffect(Action a, std::vector<Action> &aList);
        void considerConditionalEffect(DurativeAction a, std::vector<DurativeAction> &aList);
        bool removeConditionalEffect(Effect *eff, Effect *parent, int numTerm, Action &a);
        bool removeConditionalEffect(DurativeEffect *eff, DurativeEffect *parent, int numTerm, DurativeAction &a);
        bool manageConditionalEffect(Effect *eff, Effect *parent, int numTerm, Action &a);
        bool manageConditionalEffect(DurativeEffect *eff, int numTerm, DurativeAction &a);
        bool existingConditionalEffects(Effect &eff);
        bool existingConditionalEffects(DurativeEffect &eff);
        void buildOperators(Action &a, bool isGoal);
        void buildOperators(DurativeAction &a, bool isGoal);
        std::vector<Operator> buildOperatorPrecondition(Precondition &prec, Action &a, Operator *op);
        std::vector<Operator> buildOperatorPrecondition(DurativeCondition &prec, DurativeAction &a, Operator *op);
        std::vector<Operator> buildOperatorPrecondition(GoalDescription &goal, Action &a, Operator *op);
        std::vector<Operator> buildOperatorPrecondition(GoalDescription &goal, DurativeAction &a, Operator *op);
        std::vector<Operator> buildOperatorPreconditionAnd(Precondition &prec,
                                                           Action &a,
                                                           Operator *op,
                                                           unsigned int numTerm);
        std::vector<Operator> buildOperatorPreconditionAnd(DurativeCondition &prec,
                                                           DurativeAction &a,
                                                           Operator *op,
                                                           unsigned int numTerm);
        std::vector<Operator> buildOperatorPreconditionAnd(GoalDescription &goal,
                                                           Action &a,
                                                           Operator *op,
                                                           unsigned int numTerm);
        std::vector<Operator> buildOperatorPreconditionAnd(GoalDescription &goal,
                                                           DurativeAction &a,
                                                           Operator *op,
                                                           unsigned int numTerm);
        bool checkValidOperator(Operator &op, unsigned int numParameters);
        bool setParameterValues(unsigned int paramValues[],
                                unsigned int equivalences[],
                                const std::vector<OpEquality> &equality);
        bool checkEqualities(unsigned int paramValues[],
                             unsigned int equivalences[],
                             std::vector<OpEquality> &equality,
                             unsigned int numParameters);
        bool checkPreconditions(unsigned int paramValues[], std::vector<OpFluent> &precs);
        void terminateBuildingOperator(Operator &op, Action &a);
        void terminateBuildingOperator(Operator &op, DurativeAction &a);
        void buildOperatorEffect(Operator &op, Effect &effect);
        void buildOperatorEffect(Operator &op, DurativeEffect &effect);
        void buildOperatorEffect(Operator &op, TimedEffect &effect, TimeSpecifier time);
        void buildOperatorEffect(Operator &op, AssignmentContinuousEffect &effect, TimeSpecifier time);
        void buildOperatorEffect(Operator &op, FluentAssignment &effect, TimeSpecifier time);

       public:
        Preprocess();
        ~Preprocess();
        PreprocessedTask *preprocessTask(ParsedTask *parsedTask);
    };
}  // namespace grstaps
#endif
