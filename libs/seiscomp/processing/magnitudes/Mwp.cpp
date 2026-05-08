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


#include <seiscomp/processing/magnitudes/Mwp.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm*s";

// PREM (Dziewonski & Anderson 1981) depth layers.
// Each row: { max_depth_km, alpha_m_s, rho_kg_m3 }
struct PremLayer { double depthKm; double alpha; double rho; };
static const PremLayer PREM[] = {
	{  15,  6400, 2800},   // continental crust
	{  35,  6800, 2900},   // lower crust
	{  80,  8050, 3380},   // lithospheric mantle
	{ 220,  8100, 3380},   // upper mantle (LVZ)
	{ 400,  8905, 3540},   // upper transition zone
	{ 600,  9990, 3820},   // lower transition zone
	{ 660, 10266, 3993},   // 660-km discontinuity
	{ 771, 10752, 4381},   // lower mantle top
	{1000, 11065, 4526},   // lower mantle
	{2000, 12254, 5074},   // deep lower mantle
	{2891, 13716, 5566},   // CMB
};
static const int NPREM = sizeof(PREM) / sizeof(PREM[0]);

void premAtDepth(double depthKm, double &alpha, double &rho) {
	for ( int i = 0; i < NPREM; ++i ) {
		if ( depthKm <= PREM[i].depthKm ) {
			alpha = PREM[i].alpha;
			rho   = PREM[i].rho;
			return;
		}
	}
	alpha = PREM[NPREM-1].alpha;
	rho   = PREM[NPREM-1].rho;
}

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_Mwp::MagnitudeProcessor_Mwp()
 : MagnitudeProcessor("Mwp") {
	_minimumDistanceDeg = 5.0;
	_maximumDistanceDeg = 105.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_Mwp::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	auto readDouble = [&](const std::string &key, double &val) {
		if ( !cfg ) return;
		cfg->getDouble(val, key);
	};

	readDouble("magnitudes.Mwp.estimateMw.shallowMaxDepth",      _shallowMaxDepth);
	readDouble("magnitudes.Mwp.estimateMw.intermediateMaxDepth", _intermediateMaxDepth);

	readDouble("magnitudes.Mwp.estimateMw.shallow.a",        _mwShallowA);
	readDouble("magnitudes.Mwp.estimateMw.shallow.b",        _mwShallowB);
	readDouble("magnitudes.Mwp.estimateMw.shallow.stdError", _mwShallowStdError);

	readDouble("magnitudes.Mwp.estimateMw.intermediate.a",        _mwIntermediateA);
	readDouble("magnitudes.Mwp.estimateMw.intermediate.b",        _mwIntermediateB);
	readDouble("magnitudes.Mwp.estimateMw.intermediate.stdError", _mwIntermediateStdError);

	readDouble("magnitudes.Mwp.estimateMw.deep.a",        _mwDeepA);
	readDouble("magnitudes.Mwp.estimateMw.deep.b",        _mwDeepB);
	readDouble("magnitudes.Mwp.estimateMw.deep.stdError", _mwDeepStdError);

	SEISCOMP_DEBUG("Mwp estimateMw depth classes: "
	               "shallow(0-%.0f km) a=%.4f b=%.4f sigma=%.4f | "
	               "intermediate(%.0f-%.0f km) a=%.4f b=%.4f sigma=%.4f | "
	               "deep(>%.0f km) a=%.4f b=%.4f sigma=%.4f",
	               _shallowMaxDepth,
	               _mwShallowA, _mwShallowB, _mwShallowStdError,
	               _shallowMaxDepth, _intermediateMaxDepth,
	               _mwIntermediateA, _mwIntermediateB, _mwIntermediateStdError,
	               _intermediateMaxDepth,
	               _mwDeepA, _mwDeepB, _mwDeepStdError);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *, const Locale *,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	_lastDepth = depth;

	double alpha, rho;
	premAtDepth(depth, alpha, rho);

	if ( Magnitudes::compute_Mwp(amplitude*1.E-9, delta, value,
	                              0.0, 1.0, alpha, rho) ) {
		return OK;
	}
	else {
		return DistanceOutOfRange;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::estimateMw(
	const Config::Config *,
	double magnitude,
	double &estimation,
	double &stdError)
{
	double a, b, sigma;
	if ( _lastDepth <= _shallowMaxDepth ) {
		a = _mwShallowA; b = _mwShallowB; sigma = _mwShallowStdError;
	}
	else if ( _lastDepth <= _intermediateMaxDepth ) {
		a = _mwIntermediateA; b = _mwIntermediateB; sigma = _mwIntermediateStdError;
	}
	else {
		a = _mwDeepA; b = _mwDeepB; sigma = _mwDeepStdError;
	}

	estimation = a * magnitude + b;
	stdError = sigma;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
