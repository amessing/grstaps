#include "grstaps/task_planning/setup.hpp"
#include <iostream>
#include <time.h>

#include "grstaps/task_planning/grounder.hpp"
#include "grstaps/task_planning/parser.hpp"
#include "grstaps/task_planning/planner_parameters.hpp"
#include "grstaps/task_planning/preprocess.hpp"
#include "grstaps/task_planning/sas_translator.hpp"

#define _TRACE_OFF_
//#define _TIME_ON_
#define toSeconds(t) (float)(((int)(1000 * (clock() - t) / (float)CLOCKS_PER_SEC)) / 1000.0)

namespace grstaps
{
    SASTask* Setup::doPreprocess(PlannerParameters* parameters)
    {
        parameters->total_time = 0;
        SASTask* sTask         = nullptr;
        ParsedTask* parsedTask = parseStage(parameters);
        if(parsedTask != nullptr)
        {
            PreprocessedTask* prepTask = preprocessStage(parsedTask, parameters);
            if(prepTask != nullptr)
            {
                GroundedTask* gTask = groundingStage(prepTask, parameters);
                if(gTask != nullptr)
                {
                    sTask = sasTranslationStage(gTask, parameters);
                    delete gTask;
                }
                delete prepTask;
            }
            delete parsedTask;
        }
        return sTask;
    }

    ParsedTask* Setup::parseStage(PlannerParameters* parameters)
    {
        clock_t t = clock();
        Parser parser;
        ParsedTask* parsedTask = parser.parseDomain(parameters->domainFileName);
        parser.parseProblem(parameters->problemFileName);
#ifdef _TRACE_ON_
        cout << parsedTask->toString() << endl;
#endif
        float time = toSeconds(t);
        parameters->total_time += time;
#ifdef _TIME_ON_
        cout << ";Parsing time: " << time << endl;
#endif
        return parsedTask;
    }

    PreprocessedTask* Setup::preprocessStage(ParsedTask* parsedTask, PlannerParameters* parameters)
    {
        clock_t t = clock();
        Preprocess preprocess;
        PreprocessedTask* prepTask = preprocess.preprocessTask(parsedTask);
        float time                 = toSeconds(t);
        parameters->total_time += time;
#ifdef _TRACE_ON_
        cout << prepTask->toString() << endl;
#endif
#ifdef _TIME_ON_
        cout << ";Preprocessing time: " << time << endl;
#endif
        return prepTask;
    }

    GroundedTask* Setup::groundingStage(PreprocessedTask* prepTask, PlannerParameters* parameters)
    {
        clock_t t = clock();
        Grounder grounder;
        GroundedTask* gTask = grounder.groundTask(prepTask, parameters->keepStaticData);
        float time          = toSeconds(t);
        parameters->total_time += time;
#ifdef _TRACE_ON_
        cout << gTask->toString() << endl;
#endif
#ifdef _TIME_ON_
        cout << ";Grounding time: " << time << endl;
#endif
        if(parameters->generateGroundedDomain)
        {
            std::cout << ";" << gTask->actions.size() << " grounded actions" << std::endl;
            gTask->writePDDLDomain();
            gTask->writePDDLProblem();
        }
        return gTask;
    }

    SASTask* Setup::sasTranslationStage(GroundedTask* gTask, PlannerParameters* parameters)
    {
        clock_t t = clock();
        SASTranslator translator;
        SASTask* sasTask =
            translator.translate(gTask, parameters->noSAS, parameters->generateMutexFile, parameters->keepStaticData);
        float time = toSeconds(t);
        parameters->total_time += time;
#ifdef _TIME_ON_
        cout << ";SAS translation time: " << time << endl;
#endif
#ifdef _TRACE_ON_
        cout << sasTask->toString() << endl;
#endif
        return sasTask;
    }
}  // namespace grstaps
