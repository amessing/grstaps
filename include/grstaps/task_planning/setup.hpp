#ifndef GRSTAPS_SETUP_HPP
#define GRSTAPS_SETUP_HPP

namespace grstaps
{
    class GroundedTask;
    class ParsedTask;
    class PlannerParameters;
    class PreprocessedTask;
    class SASTask;

    class Setup
    {
       public:
        static SASTask* doPreprocess(PlannerParameters* parameters);

       private:
        static ParsedTask* parseStage(PlannerParameters* parameters);
        static PreprocessedTask* preprocessStage(ParsedTask* parsedTask, PlannerParameters* parameters);
        static GroundedTask* groundingStage(PreprocessedTask* prepTask, PlannerParameters* parameters);
        static SASTask* sasTranslationStage(GroundedTask* gTask, PlannerParameters* parameters);
    };
}  // namespace grstaps
#endif  // __SETUP_H__
