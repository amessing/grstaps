/*
 * Copyright (C)2020 Glen Neville
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

#ifndef GRSTAPS_CHECK_ALLOCATABLE_H
#define GRSTAPS_CHECK_ALLOCATABLE_H

// Global
#include <vector>

// External
#include <boost/shared_ptr.hpp>

namespace grstaps
{
    bool isAllocatable(const std::vector<std::vector<float>>& goalDistribution,
                       const std::vector<std::vector<float>>& speciesDistribution,
                       const std::vector<std::vector<float>>& nonCumTraitCutoff,
                       boost::shared_ptr<std::vector<int>> numSpec);
}
#endif  // GRSTAPS_CHECK_ALLOCATABLE_H
