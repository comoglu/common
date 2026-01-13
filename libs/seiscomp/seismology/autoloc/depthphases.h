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

#ifndef SEISCOMP_SEISMOLOGY_AUTOLOC_DEPTHPHASES_H
#define SEISCOMP_SEISMOLOGY_AUTOLOC_DEPTHPHASES_H

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/seismology/ttt.h>

#include <string>
#include <vector>
#include <functional>


namespace Seiscomp {
namespace Seismology {
namespace Autoloc {


/**
 * @brief Configuration for depth phase analysis
 */
struct SC_SYSTEM_CORE_API DepthPhaseConfig {
	//! List of depth phases to consider
	std::vector<std::string> phases{"pP", "sP", "pwP"};

	//! Minimum source depth (km) for depth phase analysis to be meaningful
	double minDepth{15.0};

	//! Maximum source depth (km) for depth phase analysis
	double maxDepth{700.0};

	//! Minimum epicentral distance (deg) for reliable depth phases
	double minDistance{30.0};

	//! Maximum epicentral distance (deg) for depth phases
	double maxDistance{90.0};

	//! Maximum allowed residual (s) for depth phase association
	double maxResidual{3.0};

	//! Minimum number of depth phases required for depth determination
	int minPhaseCount{3};

	//! Weight factor for depth phases in location (relative to P)
	double weight{1.5};

	//! Time window before P pick to search for depth phase (s)
	double searchWindowBefore{5.0};

	//! Time window after theoretical pP time to search (s)
	double searchWindowAfter{10.0};
};


/**
 * @brief Represents a single depth phase observation
 */
struct SC_SYSTEM_CORE_API DepthPhaseObservation {
	std::string phase;           //!< Phase code (pP, sP, etc.)
	std::string referencePhase;  //!< Reference phase (usually P)
	std::string stationCode;     //!< Station code
	std::string networkCode;     //!< Network code
	double observedTime;         //!< Observed arrival time (epoch)
	double theoreticalTime;      //!< Theoretical arrival time (epoch)
	double residual;             //!< observed - theoretical (s)
	double timeDifferenceObs;    //!< Observed pP-P or sP-P time (s)
	double timeDifferenceTheo;   //!< Theoretical pP-P or sP-P time (s)
	double distance;             //!< Epicentral distance (deg)
	double weight;               //!< Weight assigned to this observation
	bool isValid;                //!< Whether observation passes quality checks
};


/**
 * @brief Result of depth phase analysis
 */
struct SC_SYSTEM_CORE_API DepthPhaseResult {
	bool success{false};                  //!< Whether analysis succeeded
	double depth{0.0};                    //!< Estimated depth (km)
	double depthUncertainty{0.0};         //!< Depth uncertainty (km)
	double depthLowerBound{0.0};          //!< Lower bound of depth estimate (km)
	double depthUpperBound{0.0};          //!< Upper bound of depth estimate (km)
	int observationCount{0};              //!< Number of depth phase observations used
	double meanResidual{0.0};             //!< Mean residual of depth phases (s)
	double rmsResidual{0.0};              //!< RMS residual of depth phases (s)
	std::string method;                   //!< Method used ("pP-P", "sP-P", "combined")
	std::vector<DepthPhaseObservation> observations;  //!< Individual observations
};


/**
 * @brief Depth phase analyzer for constraining earthquake depths
 *
 * This class provides methods to:
 * 1. Identify depth phases in existing picks
 * 2. Calculate theoretical depth phase times
 * 3. Invert for depth using depth phase - direct phase time differences
 *
 * The pP-P (or sP-P) time difference is primarily sensitive to source depth
 * and relatively insensitive to epicentral distance, making it a powerful
 * tool for depth determination when direct depth resolution is poor.
 *
 * Usage:
 * @code
 * DepthPhaseAnalyzer analyzer;
 * analyzer.setConfig(config);
 * analyzer.setTravelTimeTable(ttt);
 *
 * // With existing picks
 * DepthPhaseResult result = analyzer.analyze(
 *     originLat, originLon, originDepth, originTime,
 *     arrivals
 * );
 *
 * // Invert for depth given observations
 * double newDepth = analyzer.invertForDepth(
 *     originLat, originLon, depthPhaseObservations
 * );
 * @endcode
 */
class SC_SYSTEM_CORE_API DepthPhaseAnalyzer : public Core::BaseObject {
	DECLARE_SC_CLASS(DepthPhaseAnalyzer)

	public:
		DepthPhaseAnalyzer();
		~DepthPhaseAnalyzer() override;

	public:
		/**
		 * @brief Set the configuration
		 * @param config Configuration parameters
		 */
		void setConfig(const DepthPhaseConfig &config);

		/**
		 * @brief Get current configuration
		 * @return Current configuration
		 */
		const DepthPhaseConfig &config() const;

		/**
		 * @brief Set the travel time table interface to use
		 * @param ttt Travel time table interface
		 * @return true if successful
		 */
		bool setTravelTimeTable(TravelTimeTableInterface *ttt);

		/**
		 * @brief Set the travel time table by name
		 * @param type Travel time table type (e.g., "libtau", "LOCSAT")
		 * @param model Model name (e.g., "iasp91", "ak135")
		 * @return true if successful
		 */
		bool setTravelTimeTable(const std::string &type,
		                        const std::string &model);

		/**
		 * @brief Compute theoretical depth phase times for a given origin
		 * @param lat Source latitude (deg)
		 * @param lon Source longitude (deg)
		 * @param depth Source depth (km)
		 * @param stationLat Station latitude (deg)
		 * @param stationLon Station longitude (deg)
		 * @param stationElev Station elevation (m)
		 * @param phases List of phase codes to compute
		 * @return Travel time list for requested phases
		 */
		TravelTimeList computeDepthPhaseTimes(
			double lat, double lon, double depth,
			double stationLat, double stationLon, double stationElev = 0.0,
			const std::vector<std::string> &phases = {}) const;

		/**
		 * @brief Compute the theoretical time difference between depth phase and P
		 * @param depthPhase Depth phase code (e.g., "pP", "sP")
		 * @param lat Source latitude (deg)
		 * @param lon Source longitude (deg)
		 * @param depth Source depth (km)
		 * @param stationLat Station latitude (deg)
		 * @param stationLon Station longitude (deg)
		 * @param stationElev Station elevation (m)
		 * @return Time difference in seconds (pP-P or sP-P), or -1 if not computable
		 */
		double computeDepthPhaseTimeDifference(
			const std::string &depthPhase,
			double lat, double lon, double depth,
			double stationLat, double stationLon, double stationElev = 0.0) const;

		/**
		 * @brief Analyze arrivals for depth phases and estimate depth
		 *
		 * This method examines existing phase picks to identify depth phases
		 * and uses them to constrain the source depth.
		 *
		 * @param lat Source latitude (deg)
		 * @param lon Source longitude (deg)
		 * @param depth Initial depth estimate (km)
		 * @param originTime Origin time (epoch seconds)
		 * @param arrivals List of arrivals with phase, time, station info
		 * @return Analysis result with depth estimate and observations
		 */
		template <typename ArrivalContainer>
		DepthPhaseResult analyze(
			double lat, double lon, double depth, double originTime,
			const ArrivalContainer &arrivals) const;

		/**
		 * @brief Invert for depth using depth phase observations
		 *
		 * Performs grid search or iterative inversion to find the depth
		 * that best fits the observed depth phase times.
		 *
		 * @param lat Source latitude (deg)
		 * @param lon Source longitude (deg)
		 * @param observations Vector of depth phase observations
		 * @param initialDepth Starting depth for inversion (km)
		 * @return Inverted depth (km), or -1 if inversion failed
		 */
		double invertForDepth(
			double lat, double lon,
			const std::vector<DepthPhaseObservation> &observations,
			double initialDepth = 33.0) const;

		/**
		 * @brief Check if a phase code is a depth phase
		 * @param phase Phase code to check
		 * @return true if it's a depth phase
		 */
		static bool isDepthPhase(const std::string &phase);

		/**
		 * @brief Get the reference (direct) phase for a depth phase
		 * @param depthPhase Depth phase code
		 * @return Reference phase code (e.g., "P" for "pP")
		 */
		static std::string getReferencePhase(const std::string &depthPhase);

	private:
		/**
		 * @brief Perform grid search for optimal depth
		 */
		double gridSearchDepth(
			double lat, double lon,
			const std::vector<DepthPhaseObservation> &observations,
			double minDepth, double maxDepth, double step) const;

		/**
		 * @brief Calculate misfit for a given depth
		 */
		double calculateMisfit(
			double lat, double lon, double depth,
			const std::vector<DepthPhaseObservation> &observations) const;

	private:
		DepthPhaseConfig _config;
		TravelTimeTableInterfacePtr _ttt;
};


DEFINE_SMARTPOINTER(DepthPhaseAnalyzer);


} // namespace Autoloc
} // namespace Seismology
} // namespace Seiscomp


#endif // SEISCOMP_SEISMOLOGY_AUTOLOC_DEPTHPHASES_H
