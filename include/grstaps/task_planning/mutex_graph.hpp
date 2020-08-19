#ifndef GRSTAPS_MUTEX_GRAPH_HPP
#define GRSTAPS_MUTEX_GRAPH_HPP

#include <vector>

#include <unordered_map>

namespace grstaps
{
    class MutexGraph
    {
       private:
        unsigned int numVertex;
        std::vector<std::vector<unsigned int> > mutexComponents;     // Set of mutually exclusive connected components
        std::vector<unsigned int> variableIndex;                     // Graph vertex -> real variable index
        std::unordered_map<unsigned int, unsigned int> vertexIndex;  // Real variable index -> graph vertex
        std::vector<std::vector<unsigned int> > adjacent;            // Adjacent vertex for each vertex
        void depthFirstSearch(unsigned int origin, std::vector<unsigned int> &component, bool *visited);
        bool isMutuallyExclusive(const std::vector<unsigned int> &component);
        inline bool isAdjacent(unsigned int i, unsigned int j)
        {
            for(unsigned int a = 0; a < adjacent[i].size(); a++)
                if(adjacent[i][a] == j)
                    return true;
            return false;
        }
        void processNonMutexComponent(const std::vector<unsigned int> &component);
        unsigned int highestDegreeVertex(const std::vector<unsigned int> &component);
        void computeMutexSubcomponent(unsigned int v1, unsigned int v2, std::vector<unsigned int> &subcomponent);
        void removeLinks(std::vector<unsigned int> &subcomponent);
        inline void removeLink(unsigned int i, unsigned int j)
        {
            for(unsigned int a = 0; a < adjacent[i].size(); a++)
                if(adjacent[i][a] == j)
                {
                    adjacent[i].erase(adjacent[i].begin() + a);
                    break;
                }
            for(unsigned int a = 0; a < adjacent[j].size(); a++)
                if(adjacent[j][a] == i)
                {
                    adjacent[j].erase(adjacent[j].begin() + a);
                    break;
                }
        }
        void computeMutexComponent(unsigned int origin, std::vector<unsigned int> *component);

       public:
        MutexGraph();
        void addVertex(unsigned int varIndex);
        void fixNumVertex();
        void addAdjacent(unsigned int v1, unsigned int v2);
        void split();
        unsigned int numVariables();
        void getVariable(unsigned int index, std::vector<unsigned int> &values, unsigned int noneOfThose);
    };
}  // namespace grstaps
#endif  // GRSTAPS_MUTEX_GRAPH_HPP
