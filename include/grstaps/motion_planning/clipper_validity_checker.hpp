//
// Created by andrew on 1/16/21.
//

#ifndef GRSTAPS_CLIPPER_VALIDITY_CHECKER_HPP
#define GRSTAPS_CLIPPER_VALIDITY_CHECKER_HPP

#include <clipper/clipper.hpp>
#include <ompl/base/StateValidityChecker.h>

namespace grstaps
{
    /**
     * Wrapper for using Clipper for collision checking in OMPL
     */
    class ClipperValidityChecker : public ompl::base::StateValidityChecker
    {
       public:
        /**
         * Constructor
         */
        ClipperValidityChecker(const ClipperLib2::Paths& internals,
                        const ompl::base::SpaceInformationPtr& space_information);

        /**
         * \return Whether \p state is valid meaning there are no collisions
         */
        virtual bool isValid(const ompl::base::State* state) const override;

       private:
        ClipperLib2::Paths m_internals;
    };
}

#endif  // GRSTAPS_CLIPPER_VALIDITY_CHECKER_HPP
