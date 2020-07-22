#ifndef GRSTAPS_LANDMARK_HEURISTIC_HPP
#define GRSTAPS_LANDMARK_HEURISTIC_HPP

#include <string>
#include <vector>

#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class LandmarkNode;

    class SASAction;

    class SASTask;

    class TState;

    class LandmarkCheck
    {            // Landmarks reachability for heuristic purposes
    private:
        std::vector<TVariable> vars;
        std::vector<TValue> values;
        std::vector<LandmarkCheck*> prev;
        std::vector<LandmarkCheck*> next;
        bool checked;
        bool single;

    public:
        LandmarkCheck(LandmarkNode* n);

        void addNext(LandmarkCheck* n);

        void addPrev(LandmarkCheck* n);

        void removeSuccessor(LandmarkCheck* n);

        void removePredecessor(LandmarkCheck* n);

        bool isGoal(SASTask* task);

        bool goOn(TState* s);

        bool isInitialState(TState* state);

        inline void uncheck()
        {
            checked = false;
        }

        inline void check()
        {
            checked = true;
        }

        inline bool isChecked() const
        {
            return checked;
        }

        inline bool isSingle() const
        {
            return single;
        }

        inline unsigned int numPrev() const
        {
            return prev.size();
        }

        inline unsigned int numNext() const
        {
            return next.size();
        }

        inline TVariable getVar() const
        {
            return vars[0];
        }

        inline TValue getValue() const
        {
            return values[0];
        }

        inline LandmarkCheck* getNext(unsigned int i) const
        {
            return next[i];
        }

        inline LandmarkCheck* getPrev(unsigned int i) const
        {
            return prev[i];
        }

        std::string toString(SASTask* task, bool showNext);
    };

    class LandmarkHeuristic
    {        // Landmarks heuristic
    private:
        SASTask* task;
        std::vector<LandmarkCheck*> nodes;
        std::vector<LandmarkCheck*> rootNodes;

        void addRootNode(LandmarkCheck* n, TState* state, std::vector<LandmarkCheck*>* toDelete);

        bool hasRootPredecessor(LandmarkCheck* n);

    public:
        LandmarkHeuristic();

        ~LandmarkHeuristic();

        void initialize(SASTask* task, std::vector<SASAction*>* tilActions);

        void initialize(TState* state, SASTask* task, std::vector<SASAction*>* tilActions);

        void uncheckNodes();

        uint16_t evaluate();

        void copyRootNodes(std::vector<LandmarkCheck*>* v);

        std::string toString(SASTask* task);

        inline unsigned int getNumNodes() const
        {
            return nodes.size();
        }

        inline uint16_t countUncheckedNodes() const
        {
            uint16_t n = 0;
            for(unsigned int i = 0; i < nodes.size(); i++)
            {
                if(!nodes[i]->isChecked())
                {
                    n++;
                }
            }
            return n;
        }

        int getNumInformativeNodes();
    };
}

#endif //GRSTAPS_LANDMARK_HEURISTIC_HPP
