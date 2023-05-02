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


#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/processing/amplitudes/Ms20.h>
#include <seiscomp/processing/amplitudes/iaspei.h>
#include <seiscomp/math/filter/seismometers.h>

#include <limits>


using namespace Seiscomp::Math;


namespace {

bool measure_period(int n, const double *f, int i0, double offset,
                    double *per, double *std) {

	//  Measures the period of an approximately sinusoidal signal f about
	//  the sample with index i0. It does so by measuring the zero
	//  crossings of the signal as well as the position of its extrema.
	//
	//  To be improved e.g. by using splines

	// TODO offset!
	int ip1, ip2, in1, in2;

	double f0 = f[i0];

	// find zero crossings

	// first previous
	for (ip1=i0;   ip1>=0 && (f[ip1]-offset)*f0 >= 0;  ip1--);
	// second previous
	for (ip2=ip1;  ip2>=0 && (f[ip2]-offset)*f0 <  0;  ip2--);

	// first next
	for (in1=i0;   in1<n  && (f[in1]-offset)*f0 >= 0;  in1++);
	// second next
	for (in2=in1;  in2<n  && (f[in2]-offset)*f0 <  0;  in2++);

	double wt = 0, pp = 0;

	// for computing the standard deviation, we need:
	double m[5];
	int nm=0;

	if (ip2>=0) {
		wt += 0.5;
		pp += 0.5*(ip1 -ip2);
		m[nm++] = ip1 -ip2;

		int imax = find_absmax(n, f, ip2, ip1, offset);
		wt += 1;
		pp += i0 -imax;
		m[nm++] = i0 -imax;
	}
	if (ip1>=0 && in1<n) {
		wt += 1;
		pp += in1 -ip1;
		m[nm++] = in1 -ip1;
	}
	if (in2<n) {
		wt += 0.5;
		pp += 0.5*(in2 -in1);
		m[nm++] = in2 -in1;

		int imax = find_absmax(n, f, in1, in2, offset);
		wt += 1;
		pp += imax - i0;
		m[nm++] = imax - i0;

	}

	// compute standard deviation of period
	if (nm>=3) {
		double avg = 0, sum = 0;
		for(int i=0; i<nm; i++) avg += m[i];
		avg /= nm;
		for(int i=0; i<nm; i++) sum += (m[i]-avg)*(m[i]-avg);
		*std = 2*sqrt(sum/(nm-1));
	}
	else    *std = 0;


	if (wt < 0.9) return false;

	*per = 2*pp/wt;

	return true;
}

}


namespace Seiscomp {
namespace Processing {


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_Ms_20, "Ms_20");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Ms_20::AmplitudeProcessor_Ms_20()
: AmplitudeProcessor("Ms_20") {
	setSignalEnd(3600.);
	setMinSNR(0);
	setMinDist(20);
	setMaxDist(160);
	setMaxDepth(100);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Ms_20::AmplitudeProcessor_Ms_20(const Core::Time& trigger, double duration)
: AmplitudeProcessor(trigger, "Ms_20") {
	setSignalEnd(3600.);
	setMinSNR(0);
	setMinDist(20);
	setMaxDist(160);
	setMaxDepth(100);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Ms_20::AmplitudeProcessor_Ms_20::initFilter(double fsamp) {
	AmplitudeProcessor::setFilter(
		new Filtering::IIR::WWSSN_LP_Filter<double>(Velocity)
	);
	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Ms_20::computeAmplitude(
	const DoubleArray &data,
	size_t i1, size_t i2,
	size_t si1, size_t si2, double offset,
	AmplitudeIndex *dt,
	AmplitudeValue *amplitude,
	double *period, double *snr) {

	if (data.size() == 0)
		return false;

	const size_t n = data.size();
	const double *f = static_cast<const double*>(data.data());

	double amax, pmax;
	size_t imax;

	if ( _config.iaspeiAmplitudes ) {
		// In addition to WWSSN-LP seismograph simulation, the
		// IASPEI Magnitude Working Group recommends to explicitly
		// limit Ms_20 measurements to signals with a dominant period
		// (after WWSSN-LP filtering) between 18 and 22 seconds.
		double fsamp = _stream.fsamp;
		size_t p18s = size_t(fsamp*18);
		size_t p22s = size_t(fsamp*22);
		IASPEI::AmplitudePeriodMeasurement m {
			IASPEI::measureAmplitudePeriod(
				n, f, offset, si1, si2, p18s, p22s) };
		if ( ! m.success )
			return false;

		amax = (m.ap2p2 + m.ap2p1)/2;
		imax = (m.ip2p2 + m.ip2p1)/2;
		pmax = (m.ip2p2 - m.ip2p1)*2;
		// We don't determine the standard error of the period.
	}
	else {
		// Low-level signal amplitude computation. This is magnitude specific.
		//
		// Input:
		//      f           double array of length n
		//      i1,i2       indices defining the measurement window,
		//                  0 <= i1 < i2 <= n
		//      offset      this is subtracted from the samples in f before
		//                  computation
		//
		// Output:
		//      dt          Point at which the measurement was mad/completed. May
		//                  be the peak time or end of integration.
		//      amplitude   amplitude. This may be a peak amplitude as well as a
		//                  sum or integral.
		//      period      dominant period of the signal. Optional. If the period
		//                  is not computed, set it to -1.

		imax = find_absmax(data.size(), (const double*)data.data(), si1, si2, offset);
		amax = fabs(data[imax] - offset);
		pmax = -1;
		double pstd =  0; // standard error of period
		if ( !measure_period(data.size(), static_cast<const double*>(data.data()), imax, offset, &pmax, &pstd) )
			pmax = -1;
	}

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;
	*period = pmax;
	amplitude->value = amax;

	if ( _usedComponent <= SecondHorizontal ) {
		if ( _streamConfig[_usedComponent].gain != 0.0 )
			amplitude->value /= _streamConfig[_usedComponent].gain;
		else {
			setStatus(MissingGain, 0.0);
			return false;
		}
	}
	else
		return false;

	// Convert meters to nanometers
	// (see IASPEI Magitude Working Group Recommendations)
	amplitude->value *= 1E9;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Ms_20::finalizeAmplitude(DataModel::Amplitude *amplitude) const {
	if ( amplitude == NULL )
		return;

	try {
		DataModel::TimeQuantity time(amplitude->timeWindow().reference());
		amplitude->setScalingTime(time);
	}
	catch ( ... ) {
	}

	try {
		DataModel::RealQuantity A = amplitude->amplitude();
		double f = 1. / amplitude->period().value();
		double c = 1. / IASPEI::wwssnlpAmplitudeResponse(f);
		A.setValue(c*A.value());
		amplitude->setAmplitude(A);
	}
	catch ( ... ) {
	}

	if (_config.iaspeiAmplitudes) {
		amplitude->setMethodID("IASPEI Ms(20) amplitude");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor_Ms_20::timeWindowLength(double distance_deg) const {
	// Minimal S/SW group velocity.
	//
	// This is very approximate and may need refinement. Usually the Lg
	// group velocity is around 3.2-3.6 km/s. By setting v_min to 3 km/s,
	// we are probably on the safe side. We add 30 s to coount for rupture
	// duration, which may, however, nit be sufficient.
	double v_min = 3.5;

	double distance_km = distance_deg*111.195; 
	double windowLength = distance_km/v_min + 30;  
	return windowLength < _config.signalEnd ? windowLength :_config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Processing
} // namespace Seiscomp
