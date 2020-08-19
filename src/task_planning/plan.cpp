#include "grstaps/task_planning/plan.hpp"
#include <iostream>

// local
#include "grstaps/task_planning/causal_link.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: TOpenCond                                     */
    /********************************************************/

    TOpenCond::TOpenCond(TStep s, uint16_t c)
    {
        step       = s;
        condNumber = c;
    }

    /********************************************************/
    /* CLASS: Plan                                          */
    /********************************************************/

    Plan::Plan(SASAction* action, Plan* parentPlan, uint32_t idPlan)
    {
        this->parentPlan = parentPlan;
        this->action     = action;
        fixedEnd         = -1;
        childPlans       = nullptr;
        id               = idPlan;
        openCond         = nullptr;
        h = hAux                     = FLOAT_INFINITY;
        hLand                        = MAX_UINT16;
        gc                           = 0;
        g                            = parentPlan == nullptr ? 0 : parentPlan->g + 1;
        repeatedState                = false;
        unsatisfiedNumericConditions = false;
        task_allocatable             = false;
    }

    Plan::Plan(SASAction* action, Plan* parentPlan, float fixedEnd, uint32_t idPlan)
    {
        this->parentPlan = parentPlan;
        this->action     = action;
        this->fixedEnd   = fixedEnd;
        childPlans       = nullptr;
        id               = idPlan;
        openCond         = nullptr;
        h = hAux                     = FLOAT_INFINITY;
        hLand                        = MAX_UINT16;
        gc                           = 0;
        g                            = parentPlan == nullptr ? 0 : parentPlan->g + 1;
        repeatedState                = false;
        unsatisfiedNumericConditions = false;
        task_allocatable             = false;
    }

    // Compares this plan with the given one. Returns a negative number if this is better, 0 if both are equally good or
    // a positive number if p is better
    int Plan::compare(Plan* p, int queue)
    {
        float v1 = 0, v2 = 0;
        switch(queue & SEARCH_MASK_PLATEAU)
        {
            case SEARCH_G_HFF:
                v1 = g + h;
                v2 = p->g + p->h;
                break;
            case SEARCH_G_2HFF:
                v1 = g + 2 * h;
                v2 = p->g + 2 * p->h;
                break;
            case SEARCH_HFF:
                v1 = h;
                v2 = p->h;
                break;
            case SEARCH_G_3HFF:
                v1 = g + 3 * h;
                v2 = p->g + 3 * p->h;
                break;
            case SEARCH_G_HLAND_HFF:
                v1 = g + h + hLand;
                v2 = p->g + p->h + p->hLand;
                break;
            case SEARCH_G:
                v1 = g;
                v2 = p->g;
                break;
            case SEARCH_G_HLAND:
                v1 = g + hLand;
                v2 = p->g + p->hLand;
                break;
            case SEARCH_G_3HLAND:
                v1 = g + 3 * hLand;
                v2 = p->g + 3 * p->hLand;
                break;
            case SEARCH_HLAND:
                v1 = hLand;
                v2 = p->hLand;
                break;
            case SEARCH_G_2HAUX:
                v1 = g + 3 * hAux;
                v2 = p->g + 3 * p->hAux;
                break;
            default:
                std::cout << "Error" << std::endl;
                exit(0);
        }
        if(unsatisfiedNumericConditions)
        {
            v1++;
        }
        if(p->unsatisfiedNumericConditions)
        {
            v2++;
        }
        if(v1 == v2)
        {
            // if (useful && !(p->useful)) return -1;
            // if (p->useful && !useful) return 1;
            if(queue < SEARCH_MASK_PLATEAU)
            {
                return ((int)g) - ((int)p->g);
            }
            else
            {
                v1 = gc;
                v2 = p->gc;
            }
        }
        if(v1 < v2)
        {
            return -1;
        }
        if(v1 > v2)
        {
            return 1;
        }
        return 0;
    }

    float Plan::getH(int queue)
    {
        switch(queue & SEARCH_MASK_PLATEAU)
        {
            case SEARCH_G_HFF:
            case SEARCH_G_2HFF:
            case SEARCH_HFF:
            case SEARCH_G_3HFF:
            case SEARCH_G:
            case SEARCH_G_HLAND_HFF:
                return h;
            case SEARCH_G_HLAND:
            case SEARCH_G_3HLAND:
            case SEARCH_HLAND:
                return hLand;
            case SEARCH_G_2HAUX:
                return hAux;
            default:
                return 0;
        }
    }

    /*
    void Plan::checkUsefulPlan() {
        useful = false;
        if (parentPlan == nullptr) return;
        for (unsigned int i = 0; i < parentPlan->usefulActions.size(); i++) {
            if (parentPlan->usefulActions[i] == action) {
                useful = true;
                break;
            }
        }
    }
    */

    // Returns a string representation of this plan
    std::string Plan::toString()
    {
        std::string s = parentPlan != nullptr ? parentPlan->toString() : "";
        if(action != nullptr)
        {
            s += "+ [" + std::to_string(gc) + "] Action: " + action->name + "\n";
        }
        for(unsigned int i = 0; i < causalLinks.size(); i++)
        {
            s += "  * " + std::to_string(causalLinks[i].firstPoint()) + " ---> " +
                 std::to_string(causalLinks[i].secondPoint()) + "\n";
        }
        for(unsigned int i = 0; i < orderings.size(); i++)
        {
            s += "  * " + std::to_string(firstPoint(orderings[i])) + " -> " +
                 std::to_string(secondPoint(orderings[i])) + "\n";
        }
        return s;
    }

    // Destructor
    Plan::~Plan()
    {
        clearChildren();
        if(openCond != nullptr)
        {
            delete openCond;
            openCond = nullptr;
        }
    }

    // Adds the children of a plan
    void Plan::addChildren(std::vector<Plan*>& suc)
    {
        childPlans = new std::vector<Plan*>(suc);
    }

    // Removes the child plans. Call only if all the child plans have been expanded
    void Plan::clearChildren()
    {
        if(childPlans != nullptr)
        {
            delete childPlans;
            childPlans = nullptr;
        }
    }

    void Plan::addOpenCondition(uint16_t condNumber, uint16_t stepNumber)
    {
        if(openCond == nullptr)
        {
            openCond = new std::vector<TOpenCond>();
        }
        openCond->emplace_back(stepNumber, condNumber);
    }
}  // namespace grstaps