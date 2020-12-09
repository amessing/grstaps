/*
 * Copyright (C) 2020 Andrew Messing
 *
 * GRSTAPS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * GRSTAPS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "grstaps/solver.hpp"

// local
#include "grstaps/Connections/taskAllocationToScheduling.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Scheduling/TAScheduleTime.h"
#include "grstaps/Search/AStarSearch.h"
#include "grstaps/Search/UniformCostSearch.h"
#include "grstaps/Task_Allocation/AllocationDistance.h"
#include "grstaps/Task_Allocation/AllocationExpander.h"
#include "grstaps/Task_Allocation/AllocationIsGoal.h"
#include "grstaps/Task_Allocation/AllocationResultsPackager.h"
#include "grstaps/Task_Allocation/TAGoalDist.h"
#include "grstaps/Task_Allocation/TaskAllocation.h"
#include "grstaps/Task_Allocation/checkAllocatable.h"
#include "grstaps/logger.hpp"
#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/problem.hpp"
#include "grstaps/solution.hpp"
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/task_planner.hpp"
#include "grstaps/Task_Allocation/checkAllocatable.h"

using std::shared_ptr;

namespace grstaps
{
    std::shared_ptr<Solution> Solver::solve(Problem& problem)
    {
        // Initialize everything
        const nlohmann::json& config = problem.config();

        // Task planner
        TaskPlanner task_planner(problem.task());
        unsigned int tplan_nodes_expanded = 0;
        unsigned int tplan_nodes_visited  = 0;
        unsigned int tplan_nodes_pruned   = 0;
        Plan* base;

        // Motion Planning
        auto motion_planners = setupMotionPlanners(problem);


        // Task Allocation
        taskAllocationToScheduling taToSched(motion_planners, &problem.startingLocations(),problem.longestPath);
        bool usingSpecies = false;
        unsigned int talloc_nodes_expanded = 0;
        unsigned int talloc_nodes_visited  = 0;

        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        auto heuristic = std::make_shared<const TAGoalDist>();
        auto path_cost = std::make_shared<const TAScheduleTime>();

        auto isGoal    = std::make_shared<const AllocationIsGoal>();
        auto expander = std::make_shared<const AllocationExpander>(heuristic, path_cost);
        SearchResultPackager<TaskAllocation>* package            = new AllocationResultsPackager();

        auto numSpec     = std::make_shared<std::vector<int>>(problem.robotTraits().size(), 1);
        auto robotTraits = &problem.robotTraits();

        Timer planTimer;
        planTimer.start();

        while(!task_planner.emptySearchSpace())
        {
            base = task_planner.poll();
            ++tplan_nodes_expanded;
            Logger::debug("Expanding plan: {}", base->id);
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            unsigned int num_children     = successors.size();

            std::vector<std::tuple<Plan*, TaskAllocation>> potential_successors;

            // openmp line
            for(unsigned int i = 0; i < num_children; ++i)
            {
                auto orderingCon       = std::make_shared<std::vector<std::vector<int>>>();
                auto durations         = std::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = std::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = std::make_shared<std::vector<std::vector<float>>>();
                auto  actionLocations  = std::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                Plan* plan = successors[i];
                setupTaskAllocationParameters(plan,
                                              problem,
                                              orderingCon,
                                              durations,
                                              noncumTraitCutoff,
                                              goalDistribution,
                                              actionLocations);

                taToSched.setActionLocations(actionLocations);

                Timer taTime;
                taTime.start();
                if(isAllocatable(goalDistribution, robotTraits, noncumTraitCutoff, numSpec))
                {
                    TaskAllocation ta(usingSpecies,
                                      goalDistribution,
                                      robotTraits,
                                      noncumTraitCutoff,
                                      taToSched,
                                      durations,
                                      orderingCon,
                                      numSpec,
                                      problem.speedIndex,
                                      problem.mpIndex);

                    auto root = std::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
                    root->setData(ta);
                    Graph<TaskAllocation> allocationGraph;
                    allocationGraph.addNode(root);

                    AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);
                    graphAllocateAndSchedule.search(isGoal, expander, package);
                    talloc_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                    talloc_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                    taTime.recordSplit(Timer::SplitType::e_ta);
                    taTime.stop();
                }

                if(package->foundGoal)
                {
                    successors[i]->gc = package->finalNode->getData().taToScheduling.sched.getMakeSpan();
                    potential_successors.push_back({successors[i], package->finalNode->getData()});
                }
            }

            std::sort(potential_successors.begin(),
                      potential_successors.end(),
                      [](std::tuple<Plan*, TaskAllocation> lhs, std::tuple<Plan*, TaskAllocation> rhs) {
                          return std::get<0>(lhs)->h > std::get<0>(rhs)->h;
                      });
            Logger::debug("TA filtered {} out of {}", num_children - potential_successors.size(), num_children);
            Logger::debug("Found");
            for(unsigned int i = 0; i < potential_successors.size(); ++i)
            {
                auto* potential_plan = std::get<0>(potential_successors[i]);
                if(potential_plan->isSolution())
                {
                    planTimer.recordSplit(Timer::SplitType::e_tp);
                    planTimer.stop();
                    delete package;
                    auto potential_ta = std::get<1>(potential_successors[i]);


                    nlohmann::json metrics = {
                        {"makespan", potential_ta.getScheduleTime()},
                        {"num_actions", (*potential_ta.params->actionDurations).size()},
                        {"num_tp_nodes_expanded", tplan_nodes_expanded},
                        {"num_tp_nodes_visited", tplan_nodes_visited},
                        {"num_tp_nodes_pruned", tplan_nodes_pruned},
                        {"num_ta_nodes_expanded", talloc_nodes_expanded},
                        {"num_ta_nodes_visited", talloc_nodes_visited},
                        {"timers", planTimer}
                    };

                    auto m_solution =
                        std::make_shared<Solution>(std::shared_ptr<Plan>(potential_plan),
                                                   std::make_shared<TaskAllocation>(potential_ta),
                                                       metrics);

                    return m_solution;
                }

            std::vector<Plan*> valid_successors;
            for(auto it = potential_successors.begin(), end = potential_successors.end(); it != end; ++it)
            {
                valid_successors.push_back(std::move(std::get<0>(*it)));
            }

            tplan_nodes_pruned += potential_successors.size() - valid_successors.size();
            tplan_nodes_visited += valid_successors.size();
            task_planner.update(base, valid_successors);
            }
        }

        planTimer.recordSplit(Timer::SplitType::e_tp);
        planTimer.stop();

        delete package;
        return nullptr;
    }

    std::shared_ptr<Solution> Solver::solveSequentially(Problem& problem){
        // Initialize everything
        const nlohmann::json& config = problem.config();

        // Task planner
        TaskPlanner task_planner(problem.task());
        unsigned int tplan_nodes_expanded = 0;
        unsigned int tplan_nodes_visited  = 0;
        Plan* base;

        // Motion Planning
        auto motion_planners = setupMotionPlanners(problem);


        // Task Allocation
        taskAllocationToScheduling taToSched(motion_planners, &problem.startingLocations());
        bool usingSpecies = false;
        unsigned int talloc_nodes_expanded = 0;
        unsigned int talloc_nodes_visited  = 0;

        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        auto heur = std::make_shared<const AllocationDistance>();
        auto cos       = std::make_shared<const TAScheduleTime>();

        auto isGoal    = std::make_shared<const AllocationIsGoal>();
        auto expander = std::make_shared<const AllocationExpander>(heur, cos);
        SearchResultPackager<TaskAllocation>* package            = new AllocationResultsPackager();

        auto numSpec     = std::make_shared<std::vector<int>>(problem.robotTraits().size(), 1);
        auto robotTraits = &problem.robotTraits();

        Timer planTime;
        planTime.start();
        float longestMP = problem.longestPath;

        while(!task_planner.emptySearchSpace())
        {
            base = task_planner.poll();
            ++tplan_nodes_expanded;
            Logger::debug("Expanding plan: {}", base->id);
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            for(Plan* plan : successors)
            {
                if(plan->isSolution())
                {
                    auto orderingCon       = std::make_shared<std::vector<std::vector<int>>>();
                    auto durations         = std::make_shared<std::vector<float>>();
                    auto noncumTraitCutoff = std::make_shared<std::vector<std::vector<float>>>();
                    auto goalDistribution  = std::make_shared<std::vector<std::vector<float>>>();
                    auto  actionLocations = std::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                    setupTaskAllocationParameters(plan,
                                                  problem,
                                                  orderingCon,
                                                  durations,
                                                  noncumTraitCutoff,
                                                  goalDistribution,
                                                  actionLocations);


                    if(isAllocatable(goalDistribution, robotTraits, noncumTraitCutoff, numSpec))
                    {
                        taToSched.setActionLocations(actionLocations);
                        Timer taTime;
                        taTime.start();

                        TaskAllocation ta(usingSpecies,
                                          goalDistribution,
                                          robotTraits,
                                          noncumTraitCutoff,
                                          taToSched,
                                          durations,
                                          orderingCon,
                                          numSpec,
                                          problem.speedIndex,
                                          problem.mpIndex);

                        auto root = std::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
                        root->setData(ta);
                        Graph<TaskAllocation> allocationGraph;
                        allocationGraph.addNode(root);

                        AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);

                    while(!graphAllocateAndSchedule.empty())
                    {
                        graphAllocateAndSchedule.search(isGoal, expander, package);
                        talloc_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                        talloc_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                        taTime.recordSplit(Timer::SplitType::e_ta);
                        taTime.stop();

                            if(package->foundGoal)
                            {
                                // if a schedule cannot be found then continue allocating
                                if(package->finalNode->getData().getScheduleTime() < 0.0)
                                {
                                    continue;
                                }

                            planTime.recordSplit(Timer::SplitType::e_tp);
                            planTime.stop();
                            delete package;

                                // todo: finish adding metrics
                                nlohmann::json metrics = {
                                    {"makespan", package->finalNode->getData().getScheduleTime()},
                                    {"num_actions", (*package->finalNode->getData().params->actionDurations).size()},
                                    {"num_tp_nodes_expanded", tplan_nodes_expanded},
                                    {"num_tp_nodes_visited", tplan_nodes_visited},
                                    {"num_ta_nodes_expanded", talloc_nodes_expanded},
                                    {"num_ta_nodes_visited", talloc_nodes_visited},
                                    {"timers", planTime}};
                                auto m_solution = std::make_shared<Solution>(
                                    std::shared_ptr<Plan>(plan),
                                    std::make_shared<TaskAllocation>(package->finalNode->getData()),
                                    metrics);
                                return m_solution;
                            }
                        }
                        taTime.restart();
                    }
                }
            }

            // if no solution is found update the task planner and continue search
            tplan_nodes_visited += successors.size();
            task_planner.update(base, successors);
        }
        planTime.recordSplit(Timer::SplitType::e_tp);
        planTime.stop();

        delete package;
        return nullptr;
    }

    void Solver::writeSolution(const std::string& folder, std::shared_ptr<Solution> solution)
    {
        if(!std::experimental::filesystem::exists(folder))
        {
            std::experimental::filesystem::create_directories(folder);
        }

        std::string filepath = fmt::format("{}/output.json", folder);
        solution->write(filepath);
    }

    void Solver::planSubcomponents(const Plan* base, std::vector<const Plan*>& plan_subcomponents)
    {
        if(base == nullptr)
        {
            plan_subcomponents.clear();
        }
        else
        {
            planSubcomponents(base->parentPlan, plan_subcomponents);
            plan_subcomponents.push_back(base);
        }
    }
    std::shared_ptr<std::vector<std::shared_ptr<MotionPlanner>>> Solver::setupMotionPlanners(const Problem& problem)
    {
        const std::vector<std::vector<b2PolygonShape>>& obstacles =  problem.obstacles();
        const nlohmann::json& config = problem.config();

        auto motion_planners = std::make_shared<std::vector<std::shared_ptr<MotionPlanner>>>();
        motion_planners->reserve(obstacles.size());

        const float boundary_min      = config["mp_boundary_min"];
        const float boundary_max      = config["mp_boundary_max"];
        const float query_time        = config["mp_query_time"];
        const float connection_range  = config["mp_connection_range"];

        for(int i = 0; i < obstacles.size(); ++i)
        {
            auto motion_planner = std::make_shared<MotionPlanner>();
            motion_planner->setMap(obstacles[i], boundary_min, boundary_max);
            motion_planner->setLocations(problem.locations());
            motion_planner->setQueryTime(query_time);
            motion_planner->setConnectionRange(connection_range);
            motion_planners->push_back(motion_planner);
        }
        return motion_planners;
    }
    void Solver::setupTaskAllocationParameters(
        const Plan* plan,
        const Problem& problem,
        std::shared_ptr<std::vector<std::vector<int>>> ordering_constraints,
        std::shared_ptr<std::vector<float>> durations,
        std::shared_ptr<std::vector<std::vector<float>>> noncum_trait_cutoff,
        std::shared_ptr<std::vector<std::vector<float>>> goal_distribution,
        std::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> action_locations)
    {
        // Fill in vectors for TA and Scheduling
        std::vector<const Plan*> plan_subcomponents;
        planSubcomponents(plan, plan_subcomponents);
        std::set<std::pair<uint16_t, uint16_t>> order_constraints;
        for(unsigned int j = 0; j < plan_subcomponents.size(); ++j)
        {
            const Plan* subcomponent = plan_subcomponents[j];
            // Ignore the fictitious action
            // TODO: ignore TILs?
            if(subcomponent->action->name != "<goal>" && subcomponent->action->name != "#initial")
            {
                for(unsigned int k = 0; k < subcomponent->orderings.size(); ++k)
                {
                    // uint16_t
                    TTimePoint fp = firstPoint(subcomponent->orderings[k]);
                    TTimePoint sp = secondPoint(subcomponent->orderings[k]);
                    // Time points are based on start and end snap actions
                    // Also remove the initial action
                    order_constraints.insert({fp / 2 - 1, sp / 2 - 1});
                }

                durations->push_back(subcomponent->action->duration[0].exp.value);

                noncum_trait_cutoff->push_back(
                    problem.actionNonCumRequirements[problem.actionToRequirements.at(subcomponent->action->name)]);
                goal_distribution->push_back(
                    problem.actionRequirements[problem.actionToRequirements.at(subcomponent->action->name)]);
                action_locations->push_back(problem.actionLocation(subcomponent->action->name));
            }
        }

        for(auto oc: order_constraints)
        {
            ordering_constraints->push_back({oc.first, oc.second});
        }
    }
}  // namespace grstaps