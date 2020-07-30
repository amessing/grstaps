#include "grstaps/task_planning/task_planner.hpp"

#include "grstaps/task_planning/state.hpp"
#include "grstaps/task_planning/successors.hpp"

namespace grstaps
{
#define toSeconds(t) (float)(((int)(1000 * (clock() - t) / (float)CLOCKS_PER_SEC)) / 1000.0)

    TaskPlanner::TaskPlanner(SASTask* task, float timeout)
        : m_task(task)
        , m_successors(new Successors)
        , m_initial_h(FLOAT_INFINITY)
        , m_timeout(timeout)
        , m_start_time(clock())
    {
        SASAction* initial_action = createInitialAction();
        m_initial_plan            = new Plan(initial_action, nullptr, 0);
        m_initial_state           = new TState(m_task);
        m_successors->initialize(m_initial_state, m_task, false, true, nullptr);
        m_successors->evaluate(m_initial_plan);
        m_quality_selector.initialize(FLOAT_INFINITY, MAX_UINT16, m_successors);
        addFrontierNodes(m_initial_plan);
    }

    SASAction* TaskPlanner::createInitialAction()
    {
        std::vector<unsigned int> varList;
        for(unsigned int i = 0; i < m_task->variables.size(); i++)
        {  // Non-numeric effects
            SASVariable& var = m_task->variables[i];
            for(unsigned int j = 0; j < var.value.size(); j++)
            {
                if(var.time[j] == 0)
                {  // Initial state effect
                    varList.push_back(i);
                    break;
                }
            }
        }
        for(unsigned int i = 0; i < m_task->numVariables.size(); i++)
        {  // Numeric effects
            NumericVariable& var = m_task->numVariables[i];
            for(unsigned int j = 0; j < var.value.size(); j++)
            {
                if(var.time[j] == 0)
                {  // Initial state effect
                    varList.push_back(i + m_task->variables.size());
                    break;
                }
            }
        }
        return createFictitiousAction(EPSILON, varList, 0, "#initial", false);
    }

    SASAction* TaskPlanner::createFictitiousAction(float actionDuration,
                                                   std::vector<unsigned int>& varList,
                                                   float timePoint,
                                                   std::string name,
                                                   bool isTIL)
    {
        SASAction* a = new SASAction();
        a->index     = MAX_UNSIGNED_INT;
        a->name      = name;
        a->isTIL     = isTIL;
        SASDuration duration;
        duration.time      = 'N';
        duration.comp      = '=';
        duration.exp.type  = 'N';  // Number (epsilon duration)
        duration.exp.value = actionDuration;
        a->duration.push_back(duration);
        for(unsigned int i = 0; i < varList.size(); i++)
        {
            unsigned int varIndex = varList[i];
            if(varIndex < m_task->variables.size())
            {  //	Non-numeric effect
                SASVariable& var = m_task->variables[varIndex];
                for(unsigned int j = 0; j < var.value.size(); j++)
                {
                    if(var.time[j] == timePoint)
                    {
                        a->endEff.emplace_back(varIndex, var.value[j]);
                        break;
                    }
                }
            }
            else
            {  // Numeric effect
                varIndex -= m_task->variables.size();
                NumericVariable& var = m_task->numVariables[varIndex];
                for(unsigned int j = 0; j < var.value.size(); j++)
                {
                    if(var.time[j] == timePoint)
                    {
                        SASNumericEffect eff;
                        eff.op        = '=';
                        eff.var       = varIndex;
                        eff.exp.type  = 'N';  // Number
                        eff.exp.value = var.value[j];
                        a->endNumEff.push_back(eff);
                        break;
                    }
                }
            }
        }
        return a;
    }

    void TaskPlanner::addFrontierNodes(Plan* p)
    {
        if(!m_quality_selector.improves(p))
        {
            return;
        }
        if(!p->expanded())
        {
            m_quality_selector.add(p);
        }
        else
        {
            for(unsigned int i = 0; i < p->childPlans->size(); i++)
            {
                addFrontierNodes(p->childPlans->at(i));
            }
        }
    }

    Plan* TaskPlanner::bestPlan()
    {
        return m_quality_selector.poll();
    }

    std::vector<Plan*> TaskPlanner::getNextSuccessors(Plan* base)
    {
        if(base == nullptr)
        {
            return std::vector<Plan*>();
        }
        if(base->expanded())  // Should only happen if concurrent?
        {
            // Cause exception bc the base pointer needs to change
            throw "expanded";
            unsigned int numChildren = base->childPlans->size();
            for(unsigned int i = 0; i < numChildren; i++)
            {
                m_quality_selector.add(base->childPlans->at(i));
            }
            // run again
            return getNextSuccessors(base);
        }
        else
        {
            std::vector<Plan*> successors;
            m_successors->computeSuccessors(base, &successors);
            ++m_expanded_nodes;
            return successors;
        }
    }

    void TaskPlanner::update(Plan* base, std::vector<Plan*>& successors)
    {
        base->addChildren(successors);
        for(Plan* p: successors)
        {
            m_quality_selector.add(p);
        }
    }

    bool TaskPlanner::emptySearchSpace()
    {
        return m_quality_selector.size() == 0;
    }
}  // namespace grstaps
