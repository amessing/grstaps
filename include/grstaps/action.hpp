// LICENSE

#ifndef GRSTAPS_ACTION_HPP
#define GRSTAPS_ACTION_HPP

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Base Action class
     */
    class Action : public Noncopyable
    {
        public:
            /**
             * Default Constructor
             */
            Action();

            /**
             * \returns The identifier for this action
             */
            unsigned int id() const;

            /**
             * \returns Preconditions
             */

            /**
             * \returns Effects
             */

            /**
             * \returns minimum and maximum duration
             */

            /**
             * \returns function for calculating duration
             */

            /**
             * \returns The required traits for this action
             */

        protected:
            unsigned int m_id; //!< The id of this specific action
            // condition (equation) - pointer/object/noncopyable
            // duration min, max (equation?) - pointer/object/noncopyable
            // effect (equation) - pointer/object/noncopyable
            // traits/trait mapping
                
            static unsigned int s_action_id; //!< The id for the next action that is created
    };
}

#endif // GRSTAPS_ACTION_HPP
