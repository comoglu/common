/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#ifndef SEISCOMP_PROCESSING_PICKER_S_L2_H
#define SEISCOMP_PROCESSING_PICKER_S_L2_H

#include "S_aic.h"

namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API SL2Picker : public SAICPicker {
	public:
		//! C'tor
		SL2Picker();

		//! D'tor
		~SL2Picker();

		bool setup(const Settings &settings) override;

	protected:
		WaveformOperator* createFilterOperator(Filter* compFilter) override;
};


}
}


#endif
