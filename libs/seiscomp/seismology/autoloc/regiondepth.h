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

#ifndef SEISCOMP_SEISMOLOGY_AUTOLOC_REGIONDEPTH_H
#define SEISCOMP_SEISMOLOGY_AUTOLOC_REGIONDEPTH_H

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/featureset.h>

#include <string>
#include <vector>


namespace Seiscomp {
namespace Seismology {
namespace Autoloc {


/**
 * @brief Configuration for region-based depth constraints
 */
struct SC_SYSTEM_CORE_API RegionDepthConfig {
	//! Whether region-based depth constraints are enabled
	bool enabled{false};

	//! List of region names to use (checked in order, first match wins)
	std::vector<std::string> regions;

	//! Global default depth (km) used when no region matches
	double globalDefaultDepth{10.0};

	//! Global maximum depth (km) used when no region matches
	double globalMaxDepth{700.0};
};


/**
 * @brief Depth constraints for a specific region
 */
struct SC_SYSTEM_CORE_API RegionDepthConstraints {
	std::string regionName;     //!< Name of the matching region
	double defaultDepth{10.0};  //!< Default depth for this region (km)
	double maxDepth{700.0};     //!< Maximum depth for this region (km)
	bool hasDefaultDepth{false}; //!< Whether region defines defaultDepth
	bool hasMaxDepth{false};     //!< Whether region defines maxDepth
	bool matched{false};         //!< Whether a region was matched
};


/**
 * @brief Region-based depth constraint lookup
 *
 * This class provides region-specific depth constraints based on
 * geographic polygons defined in BNA or GeoJSON files. Each polygon
 * can specify:
 * - defaultDepth: The depth to use when depth cannot be resolved
 * - maxDepth: Maximum allowed depth for origins in this region
 *
 * This is useful for applying geologically realistic constraints:
 * - Stable cratons: shallow earthquakes only (max ~35 km)
 * - Subduction zones: deep earthquakes possible (max ~700 km)
 * - Volcanic areas: very shallow (max ~20 km)
 * - Mid-ocean ridges: shallow (max ~15 km)
 *
 * Region files should be placed in $SEISCOMP_ROOT/share/spatial/vector/
 * and use the same format as the evrc plugin for scevent.
 *
 * BNA format example:
 * @code
 * "stable_craton","rank 1","defaultDepth: 10, maxDepth: 35",5
 * -100.0,35.0
 * -95.0,35.0
 * -95.0,40.0
 * -100.0,40.0
 * -100.0,35.0
 * @endcode
 *
 * Usage:
 * @code
 * RegionDepthLookup lookup;
 * lookup.setConfig(config);
 * lookup.init();
 *
 * RegionDepthConstraints constraints = lookup.getConstraints(lat, lon);
 * if (constraints.matched) {
 *     double defaultDepth = constraints.defaultDepth;
 *     double maxDepth = constraints.maxDepth;
 * }
 * @endcode
 */
class SC_SYSTEM_CORE_API RegionDepthLookup : public Core::BaseObject {
	DECLARE_SC_CLASS(RegionDepthLookup)

	public:
		RegionDepthLookup();
		~RegionDepthLookup() override;

	public:
		/**
		 * @brief Set configuration
		 * @param config Configuration parameters
		 */
		void setConfig(const RegionDepthConfig &config);

		/**
		 * @brief Get current configuration
		 * @return Current configuration
		 */
		const RegionDepthConfig &config() const;

		/**
		 * @brief Initialize the lookup by loading region polygons
		 *
		 * This method loads the configured regions from the GeoFeatureSet.
		 * The GeoFeatureSet must be initialized before calling this method
		 * (typically done by SeisComP application framework).
		 *
		 * @return true if at least one region was loaded, false otherwise
		 */
		bool init();

		/**
		 * @brief Check if lookup is initialized and has regions
		 * @return true if initialized with at least one region
		 */
		bool isInitialized() const;

		/**
		 * @brief Get depth constraints for a geographic location
		 *
		 * Checks configured regions in order and returns constraints
		 * from the first matching region. If no region matches,
		 * returns global defaults.
		 *
		 * @param lat Latitude (degrees)
		 * @param lon Longitude (degrees)
		 * @return Depth constraints for the location
		 */
		RegionDepthConstraints getConstraints(double lat, double lon) const;

		/**
		 * @brief Get default depth for a location
		 *
		 * Convenience method that returns just the default depth.
		 *
		 * @param lat Latitude (degrees)
		 * @param lon Longitude (degrees)
		 * @return Default depth in km
		 */
		double getDefaultDepth(double lat, double lon) const;

		/**
		 * @brief Get maximum depth for a location
		 *
		 * Convenience method that returns just the maximum depth.
		 *
		 * @param lat Latitude (degrees)
		 * @param lon Longitude (degrees)
		 * @return Maximum depth in km
		 */
		double getMaxDepth(double lat, double lon) const;

		/**
		 * @brief Get the number of loaded regions
		 * @return Number of regions
		 */
		size_t regionCount() const;

		/**
		 * @brief Get list of loaded region names
		 * @return Vector of region names
		 */
		std::vector<std::string> regionNames() const;

	private:
		/**
		 * @brief Parse depth value from region attributes
		 * @param feature GeoFeature to parse
		 * @param attrName Attribute name ("defaultDepth" or "maxDepth")
		 * @param value Output value
		 * @return true if attribute was found and parsed
		 */
		bool parseDepthAttribute(const Geo::GeoFeature *feature,
		                         const std::string &attrName,
		                         double &value) const;

	private:
		RegionDepthConfig _config;
		std::vector<const Geo::GeoFeature*> _regions;
		bool _initialized{false};
};


DEFINE_SMARTPOINTER(RegionDepthLookup);


} // namespace Autoloc
} // namespace Seismology
} // namespace Seiscomp


#endif // SEISCOMP_SEISMOLOGY_AUTOLOC_REGIONDEPTH_H
