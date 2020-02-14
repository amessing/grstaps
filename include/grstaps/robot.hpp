// LICENSE

#ifndef GRSTAPS_ROBOT_HPP
#define GRSTAPS_ROBOT_HPP

// global
#include <string>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Base class for a robot/controllable agent
     */
    class Robot : public Noncopyable
    {
        public:
            /**
             * Constructor
             *
             * \param species_name The name of the species of this robot
             */
            Robot(const std::string& species_name);

            /**
             * \returns The identifier for this robot
             */
            unsigned int id() const;

            /**
             * \returns The traits of this robot
             */

        protected:
            unsigned int m_id; //!< The identifier for this specific robot
            std::string m_species_name; //!< The name of the species of this robot
            // Traits (Vector/Map/Class)
            
            static unsigned int s_robot_id; //!< The identifier for the next robot to be created
    };
}

#endif // GRSTAPS_ROBOT_HPP
