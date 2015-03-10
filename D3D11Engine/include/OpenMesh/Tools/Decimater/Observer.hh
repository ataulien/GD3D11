/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2015 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision: 1199 $                                                         *
 *   $Date: 2015-01-16 08:47:33 +0100 (Fr, 16 Jan 2015) $                   *
 *                                                                           *
\*===========================================================================*/

/** \file Observer.hh
 *
 * This file contains an observer class which is used to monitor the progress
 * of an decimater.
 *
 */

//=============================================================================
//
//  CLASS Observer
//
//=============================================================================

#pragma once

//== INCLUDES =================================================================

#include <cstddef>
#include <OpenMesh/Core/System/config.h>

//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================

/** \brief Observer class
 *
 * Observers can be used to monitor the progress of the decimation and to
 * abort it in between.
 */
class OPENMESHDLLEXPORT Observer
{
public:

  /** Create an observer
   *
   * @param _notificationInterval Interval of decimation steps between notifications.
   */
  Observer(size_t _notificationInterval);
  
  /// Destructor
  virtual ~Observer();
  
  /// Get the interval between notification steps
  size_t get_interval() const;

  /// Set the interval between notification steps
  void set_interval(size_t _notificationInterval);
  
  /** \brief callback
   *
   * This function has to be overloaded. It will be called regularly during
   * the decimation process and will return the current step.
   *
   * @param _step Current step of the decimater
   */
  virtual void notify(size_t _step) = 0;

  /** \brief Abort callback
   *
   * After each notification, this function is called by the decimater. If the
   * function returns true, the decimater will stop at a consistent state. Otherwise
   * it will continue.
   *
   * @return abort Yes or No
   */
  virtual bool abort() const;
  
private:
  size_t notificationInterval_;
};


//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
