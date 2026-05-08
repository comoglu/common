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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_H


#include <seiscomp//processing/magnitudeprocessor.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_Mwp : public MagnitudeProcessor {
	public:
		MagnitudeProcessor_Mwp();

	public:
		void setDefaults() override {}
		bool setup(const Settings &settings) override;
		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        const Locale *,
		                        double &value) override;

		Status estimateMw(const Config::Config *config, double magnitude,
		                  double &estimation, double &stdError) override;

	private:
		double _lastDepth{0.0};

		// Depth-class boundaries (km); configurable via global.cfg
		double _shallowMaxDepth{70.0};
		double _intermediateMaxDepth{300.0};

		// Shallow (0–70 km): calibrated from GCMT 2015–2026, N=891
		double _mwShallowA{1.017};
		double _mwShallowB{0.107};
		double _mwShallowStdError{0.204};

		// Intermediate (70–300 km): calibrated from GCMT 2015–2026, N=202
		double _mwIntermediateA{1.143};
		double _mwIntermediateB{-0.954};
		double _mwIntermediateStdError{0.085};

		// Deep (>300 km): calibrated from GCMT 2015–2026, N=104
		double _mwDeepA{1.134};
		double _mwDeepB{-0.907};
		double _mwDeepStdError{0.103};
};


}
}


#endif
