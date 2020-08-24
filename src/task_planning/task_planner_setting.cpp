#include <iostream>

#include "grstaps/task_planning/hff.hpp"
#include "grstaps/task_planning/linearizer.hpp"
#include "grstaps/task_planning/task_planner_concurrent.hpp"
#include "grstaps/task_planning/task_planner_deadends.hpp"
#include "grstaps/task_planning/task_planner_reversible.hpp"
#include "grstaps/task_planning/task_planner_setting.hpp"

#define toSeconds(t) (float)(((int)(1000 * (clock() - t) / (float)CLOCKS_PER_SEC)) / 1000.0)

namespace grstaps
{
    TaskPlannerSetting::TaskPlannerSetting(SASTask* sTask, bool generateTrace, float timeout)
    {
        m_initial_time         = clock();
        this->m_timeout       = timeout;
        this->m_task          = sTask;
        this->m_generate_trace = generateTrace;
        createInitialPlan();
        m_force_at_end_conditions = checkForceAtEndConditions();
        m_filter_repeated_states = checkRepeatedStates();
        m_initial_state         = new TState(m_task);
        m_task->tilActions     = !m_til_actions.empty();
        checkPlannerType();
    }

    void TaskPlannerSetting::checkPlannerType()
    {
        // cout << ";Open end-cond.: " << (forceAtEndConditions ? 'N' : 'Y');
        // cout << "   Memo: " << (filterRepeatedStates ? 'Y' : 'N');
        // cout << "   Mutex: " << (task->hasPermanentMutexAction() ? 'Y' : 'N') << endl;
        float remainingTime = m_timeout - toSeconds(m_initial_time);
        if(!m_filter_repeated_states || !m_force_at_end_conditions)
        {
            m_task->domainType = DOMAIN_CONCURRENT;
            // cout << ";Concurrent domain" << endl;
            m_planner = new TaskPlannerConcurrent(m_task,
                                                m_initial_plan,
                                                m_initial_state,
                                                m_force_at_end_conditions,
                                                m_filter_repeated_states,
                                                m_generate_trace,
                                                &m_til_actions,
                                                nullptr,
                                                remainingTime);
        }
        else if(m_task->hasPermanentMutexAction())
        {
            m_task->domainType = DOMAIN_DEAD_ENDS;
            // cout << ";Non-reversible domain (possible dead-ends)" << endl;
            m_planner = new TaskPlannerDeadends(m_task,
                                              m_initial_plan,
                                              m_initial_state,
                                              m_force_at_end_conditions,
                                              m_filter_repeated_states,
                                              m_generate_trace,
                                              &m_til_actions,
                                              nullptr,
                                              remainingTime);
        }
        else
        {
            m_task->domainType = DOMAIN_REVERSIBLE;
            // cout << ";Reversible domain" << endl;
            m_planner = new TaskPlannerReversible(m_task,
                                                m_initial_plan,
                                                m_initial_state,
                                                m_force_at_end_conditions,
                                                m_filter_repeated_states,
                                                m_generate_trace,
                                                &m_til_actions,
                                                nullptr,
                                                remainingTime);
        }
    }

    Plan* TaskPlannerSetting::plan()
    {
        return m_planner->plan();
    }

    Plan* TaskPlannerSetting::improveSolution(uint16_t bestG, float bestGC, bool first)
    {
        return m_planner->improveSolution(bestG, bestGC, first);
    }

    unsigned int TaskPlannerSetting::getExpandedNodes()
    {
        return m_planner->getExpandedNodes();
    }

    std::string TaskPlannerSetting::planToPDDL(Plan* p)
    {
        return m_planner->planToPDDL(p);
    }

    // Creates the initial empty plan that only contains the initial and the TIL fictitious actions
    void TaskPlannerSetting::createInitialPlan()
    {
        SASAction* initialAction = createInitialAction();
        m_initial_plan              = new Plan(initialAction, nullptr, 0);
        m_initial_plan              = createTILactions(m_initial_plan);
    }

    // Creates and returns the initial fictitious action
    SASAction* TaskPlannerSetting::createInitialAction()
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

    // Create a fictitious action with the given duration and with the effects with the modified variables in the given
    // time point
    SASAction* TaskPlannerSetting::createFictitiousAction(float actionDuration,
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

    // Adds the fictitious TIL actions to the initial plan. Returns the resulting plan
    Plan* TaskPlannerSetting::createTILactions(Plan* parentPlan)
    {
        Plan* result = parentPlan;
        std::unordered_map<float, std::vector<unsigned int> > til;  // Time point -> variables modified at that time
        for(unsigned int i = 0; i < m_task->variables.size(); i++)
        {  // Non-numeric effects
            SASVariable& var = m_task->variables[i];
            for(unsigned int j = 0; j < var.value.size(); j++)
            {
                if(var.time[j] > 0)
                {  // Time-initial literal
                    if(til.find(var.time[j]) == til.end())
                    {
                        std::vector<unsigned int> v;
                        v.push_back(i);
                        til[var.time[j]] = v;
                    }
                    else
                    {
                        til[var.time[j]].push_back(i);
                    }
                }
            }
        }
        for(unsigned int i = 0; i < m_task->numVariables.size(); i++)
        {  // Numeric effects
            NumericVariable& var = m_task->numVariables[i];
            for(unsigned int j = 0; j < var.value.size(); j++)
            {
                if(var.time[j] > 0)
                {  // Numeric time-initial literal
                    if(til.find(var.time[j]) == til.end())
                    {
                        std::vector<unsigned int> v;
                        v.push_back(i + m_task->variables.size());
                        til[var.time[j]] = v;
                    }
                    else
                    {
                        til[var.time[j]].push_back(i + m_task->variables.size());
                    }
                }
            }
        }
        for(auto it = til.begin(); it != til.end(); ++it)
        {
            float timePoint = it->first;
            SASAction* a =
                createFictitiousAction(timePoint, it->second, timePoint, "#til" + std::to_string(timePoint), true);
            m_til_actions.push_back(a);
            result = new Plan(a, result, timePoint, 0);
        }
        return result;
    }

    bool TaskPlannerSetting::checkForceAtEndConditions()
    {  // Check if it's required to leave at-end conditions not supported for some actions
        std::vector<std::vector<unsigned short> > varValues;
        varValues.resize(m_task->variables.size());
        for(unsigned int i = 0; i < m_task->variables.size(); i++)
        {
            SASVariable& var = m_task->variables[i];
            for(unsigned int j = 0; j < var.value.size(); j++)
            {
                varValues[i].push_back((unsigned short)var.value[j]);
            }
        }
        RPG rpg(varValues, m_task, true, &m_til_actions);
        for(unsigned int i = 0; i < m_task->goals.size(); i++)
        {
            if(rpg.isExecutable(&(m_task->goals[i])))
                return true;
        }
        return false;
    }

    bool TaskPlannerSetting::checkRepeatedStates()
    {
        TVariable v;
        TValue value;
        for(SASAction& a: m_task->actions)
        {
            for(unsigned int i = 0; i < a.startEff.size(); i++)
            {
                v     = a.startEff[i].var;
                value = a.startEff[i].value;
                for(unsigned int j = 0; j < a.endEff.size(); j++)
                {
                    if(a.endEff[j].var == v && a.endEff[j].value != value)
                    {
                        // If (v = value) is required by any action, the domain is concurrent
                        std::vector<SASAction*>& req = m_task->requirers[v][value];
                        for(unsigned int k = 0; k < req.size(); k++)
                        {
                            if(req[k] != &a)
                            {
                                return false;
                            }
                        }
                        break;
                    }
                }
            }
        }
        return true;
    }
}  // namespace grstaps
