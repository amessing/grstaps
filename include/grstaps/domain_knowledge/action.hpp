// LICENSE

#ifndef GRSTAPS_DOMAIN_KNOWLEDGE_ACTION_HPP
#define GRSTAPS_DOMAIN_KNOWLEDGE_ACTION_HPP

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    namespace domainknowledge
    {
        /**
         * Base Action class
         */
        class Action : public Noncopyable
        {
            public:
                Action() = default;

                // toSasAction() const;
                // toTaAction() const;

            protected:
                uint m_id;
                // condition (equation) - pointer/object/noncopyable
                // duration min, max (equation?) - pointer/object/noncopyable
                // effect (equation) - pointer/object/noncopyable
                // traits/trait mapping
                
                static uint s_action_id;
        };
    }
}

#endif // GRSTAPS_DOMAIN_KNOWLEDGE_ACTION_HPP
