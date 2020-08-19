/********************************************************/
/* Stores the graph of mutex literals and makes         */
/* partitions to divide the graph into different        */
/* subsets of mutually exclusive literals.              */
/********************************************************/

#include <iostream>

#include "grstaps/task_planning/mutex_graph.hpp"

namespace grstaps
{
    /********************************************************/
    /* CLASS: MutexGraph                                    */
    /********************************************************/

    MutexGraph::MutexGraph()
        : numVertex(0)
    {}

    // Adds a new vertex to the graph
    void MutexGraph::addVertex(unsigned int varIndex)
    {
        vertexIndex[varIndex] = numVertex++;
        variableIndex.push_back(varIndex);
        adjacent.emplace_back();
    }

    // Adds v2 as an adjacent of v1
    void MutexGraph::addAdjacent(unsigned int v1, unsigned int v2)
    {
        unsigned int vertex1 = vertexIndex[v1], vertex2 = vertexIndex[v2];
        if(!isAdjacent(vertex1, vertex2))
        {
            adjacent[vertex1].push_back(vertex2);
            adjacent[vertex2].push_back(vertex1);
        }
    }

    // Splits the graph in mutually exclusive connected components
    void MutexGraph::split()
    {
        bool* visited = new bool[numVertex]{false};
        for(unsigned int v = 0; v < numVertex; v++)
        {
            if(!visited[v])
            {
                mutexComponents.emplace_back();
                std::vector<unsigned int>* component = &(mutexComponents.back());
                computeMutexComponent(v, component);
                for(unsigned int i = 0; i < component->size(); i++)
                    visited[(*component)[i]] = true;
            }
        }
        delete[] visited;
        /*
        unsigned int i = 0;
        while (i < mutexComponents.size()) {
            if (isMutuallyExclusive(mutexComponents[i])) i++;
            else {
                processNonMutexComponent(mutexComponents[i]);
                mutexComponents.erase(mutexComponents.begin() + i);
            }
        }
        i = 0;
        while (i < mutexComponents.size()) {
            if (mutexComponents[i].size() == 1 && mutexComponents[i][0] >= numVertex)
               mutexComponents.erase(mutexComponents.begin() + i);
            else i++;
        }*/
    }

    void MutexGraph::computeMutexComponent(unsigned int origin, std::vector<unsigned int>* component)
    {
        bool* visited = new bool[numVertex]{false};
        std::vector<unsigned int> newVertex;
        newVertex.push_back(origin);
        component->push_back(origin);
        visited[origin] = true;
        unsigned int v, a;
        while(newVertex.size() > 0)
        {
            v = newVertex.back();
            newVertex.pop_back();
            for(unsigned int i = 0; i < adjacent[v].size(); i++)
            {
                a = adjacent[v][i];
                if(!visited[a])
                {
                    visited[a]   = true;
                    bool isMutex = true;
                    for(unsigned int j = 0; j < component->size(); j++)
                        if(!isAdjacent(a, (*component)[j]))
                        {
                            isMutex = false;
                            break;
                        }
                    if(isMutex)
                    {
                        newVertex.push_back(a);
                        component->push_back(a);
                    }
                }
            }
        }
        delete[] visited;
    }

    // Recursive DFS. The result is stored in the component vector
    void MutexGraph::depthFirstSearch(unsigned int origin, std::vector<unsigned int>& component, bool* visited)
    {
        component.push_back(origin);
        visited[origin] = true;
        for(unsigned int i = 0; i < adjacent[origin].size(); i++)
        {
            unsigned int a = adjacent[origin][i];
            if(!visited[a])
                depthFirstSearch(a, component, visited);
        }
    }

    // Checks if all vertex in the component are mutually exclusive
    bool MutexGraph::isMutuallyExclusive(const std::vector<unsigned int>& component)
    {
        for(unsigned int i = 0; i < component.size(); i++)
        {
            for(unsigned int j = i + 1; j < component.size(); j++)
                if(!isAdjacent(component[i], component[j]))
                    return false;
        }
        return true;
    }

    // Makes partitions to the component to get mutex subcomponents
    void MutexGraph::processNonMutexComponent(const std::vector<unsigned int>& component)
    {
        std::vector<std::vector<unsigned int> > subcomponents;
        do
        {
            unsigned int v = highestDegreeVertex(component);
            unsigned int a = adjacent[v][0];
            for(unsigned int i = 1; i < adjacent[v].size(); i++)
                if(adjacent[adjacent[v][i]].size() < adjacent[a].size())
                    a = adjacent[v][i];
            subcomponents.clear();
            subcomponents.emplace_back();
            computeMutexSubcomponent(v, a, subcomponents[0]);
            removeLinks(subcomponents[0]);
            subcomponents.clear();
            bool* visited = new bool[adjacent.size()]{false};
            for(unsigned int i = 0; i < component.size(); i++)
            {
                v = component[i];
                if(!visited[v])
                {
                    subcomponents.emplace_back();
                    depthFirstSearch(v, subcomponents.back(), visited);
                }
            }
            delete[] visited;
        } while(subcomponents.size() == 1);
        for(unsigned int i = 0; i < subcomponents.size(); i++)
        {
            if(isMutuallyExclusive(subcomponents[i]))
                mutexComponents.push_back(subcomponents[i]);
            else
                processNonMutexComponent(subcomponents[i]);
        }
    }

    // Returns the vertex in the component with the highest degree
    unsigned int MutexGraph::highestDegreeVertex(const std::vector<unsigned int>& component)
    {
        unsigned int v = 0;
        while(v < component.size() && component[v] >= numVertex)
            v++;  // Skip <None of those> fictitious values
        for(unsigned int i = 1; i < component.size(); i++)
            if(component[v] < numVertex && adjacent[component[i]].size() > adjacent[component[v]].size())
                v = i;
        return component[v];
    }

    // Computes a mutually exclusive subcomponent which contains vertex v1 and v2
    void MutexGraph::computeMutexSubcomponent(unsigned int v1, unsigned int v2, std::vector<unsigned int>& subcomponent)
    {
        bool* visited = new bool[adjacent.size()]{false};
        subcomponent.push_back(v1);
        subcomponent.push_back(v2);
        std::vector<unsigned int> newVertex;
        newVertex.push_back(v1);
        newVertex.push_back(v2);
        visited[v1] = true;
        visited[v2] = true;
        unsigned int v, a;
        while(newVertex.size() > 0)
        {
            v = newVertex.back();
            newVertex.pop_back();
            for(unsigned int i = 0; i < adjacent[v].size(); i++)
            {
                a = adjacent[v][i];
                if(!visited[a])
                {
                    visited[a]   = true;
                    bool isMutex = true;
                    for(unsigned int j = 0; j < subcomponent.size(); j++)
                        if(!isAdjacent(a, subcomponent[j]))
                        {
                            isMutex = false;
                            break;
                        }
                    if(isMutex)
                    {
                        newVertex.push_back(a);
                        subcomponent.push_back(a);
                    }
                }
            }
        }
        delete[] visited;
    }

    // Removes the links of the first element in the subcomponent with the other vertex
    void MutexGraph::removeLinks(std::vector<unsigned int>& subcomponent)
    {
        unsigned int v = subcomponent[0], fictValue = 0;
        for(unsigned int i = 1; i < subcomponent.size(); i++)
        {
            if(subcomponent[i] >= numVertex)
                fictValue = subcomponent[i];
            removeLink(v, subcomponent[i]);
        }
        if(fictValue == 0)
        {  // Add the <None of those> fictitious value
            fictValue = adjacent.size();
            adjacent.emplace_back();
        }
        for(unsigned int i = 1; i < subcomponent.size(); i++)
        {
            if(!isAdjacent(fictValue, subcomponent[i]))
            {
                adjacent[fictValue].push_back(subcomponent[i]);
                adjacent[subcomponent[i]].push_back(fictValue);
            }
        }
    }

    unsigned int MutexGraph::numVariables()
    {
        return mutexComponents.size();
    }

    void MutexGraph::getVariable(unsigned int index, std::vector<unsigned int>& values, unsigned int noneOfThose)
    {
        values.clear();
        bool hasFictValue = false;
        for(unsigned int i = 0; i < mutexComponents[index].size(); i++)
        {
            if(mutexComponents[index][i] >= numVertex)
                hasFictValue = true;
            else
                values.push_back(variableIndex[mutexComponents[index][i]]);
        }
        if(hasFictValue)
            values.push_back(noneOfThose);
    }
}  // namespace grstaps
