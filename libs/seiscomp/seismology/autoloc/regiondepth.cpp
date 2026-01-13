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

#define SEISCOMP_COMPONENT RegionDepth

#include "regiondepth.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/geo/coordinate.h>


namespace Seiscomp {
namespace Seismology {
namespace Autoloc {


IMPLEMENT_SC_CLASS(RegionDepthLookup, "RegionDepthLookup");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RegionDepthLookup::RegionDepthLookup() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RegionDepthLookup::~RegionDepthLookup() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RegionDepthLookup::setConfig(const RegionDepthConfig &config) {
	_config = config;
	_initialized = false;
	_regions.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RegionDepthConfig &RegionDepthLookup::config() const {
	return _config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RegionDepthLookup::init() {
	_regions.clear();
	_initialized = false;

	if ( !_config.enabled ) {
		SEISCOMP_DEBUG("Region depth constraints disabled");
		return false;
	}

	if ( _config.regions.empty() ) {
		SEISCOMP_WARNING("Region depth enabled but no regions configured");
		return false;
	}

	const Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();
	const std::vector<Geo::GeoFeature*> &features = featureSet.features();

	SEISCOMP_DEBUG("Loading depth regions from GeoFeatureSet (%zu features available)",
	               features.size());

	for ( const std::string &regionName : _config.regions ) {
		bool found = false;

		for ( const Geo::GeoFeature *feature : features ) {
			if ( feature->name() == regionName ) {
				_regions.push_back(feature);
				found = true;

				// Log what constraints this region provides
				double defaultDepth, maxDepth;
				bool hasDefault = parseDepthAttribute(feature, "defaultDepth", defaultDepth);
				bool hasMax = parseDepthAttribute(feature, "maxDepth", maxDepth);

				SEISCOMP_INFO("Loaded depth region '%s' (defaultDepth=%s, maxDepth=%s)",
				              regionName.c_str(),
				              hasDefault ? Core::toString(defaultDepth).c_str() : "not set",
				              hasMax ? Core::toString(maxDepth).c_str() : "not set");
				break;
			}
		}

		if ( !found ) {
			SEISCOMP_WARNING("Depth region '%s' not found in GeoFeatureSet",
			                 regionName.c_str());
		}
	}

	_initialized = !_regions.empty();

	if ( _initialized ) {
		SEISCOMP_INFO("Region depth lookup initialized with %zu regions",
		              _regions.size());
	}
	else {
		SEISCOMP_WARNING("No depth regions loaded - using global defaults");
	}

	return _initialized;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RegionDepthLookup::isInitialized() const {
	return _initialized;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RegionDepthConstraints RegionDepthLookup::getConstraints(double lat, double lon) const {
	RegionDepthConstraints result;
	result.defaultDepth = _config.globalDefaultDepth;
	result.maxDepth = _config.globalMaxDepth;
	result.matched = false;

	if ( !_config.enabled || _regions.empty() ) {
		return result;
	}

	Geo::GeoCoordinate location(lat, lon);

	for ( const Geo::GeoFeature *region : _regions ) {
		if ( region->contains(location) ) {
			result.regionName = region->name();
			result.matched = true;

			// Get defaultDepth from region attributes
			double depth;
			if ( parseDepthAttribute(region, "defaultDepth", depth) ) {
				result.defaultDepth = depth;
				result.hasDefaultDepth = true;
			}

			// Get maxDepth from region attributes
			if ( parseDepthAttribute(region, "maxDepth", depth) ) {
				result.maxDepth = depth;
				result.hasMaxDepth = true;
			}

			SEISCOMP_DEBUG("Location %.2f/%.2f matched region '%s' "
			               "(defaultDepth=%.1f km, maxDepth=%.1f km)",
			               lat, lon, result.regionName.c_str(),
			               result.defaultDepth, result.maxDepth);

			return result;  // First match wins
		}
	}

	SEISCOMP_DEBUG("Location %.2f/%.2f matched no region, using global defaults "
	               "(defaultDepth=%.1f km, maxDepth=%.1f km)",
	               lat, lon, result.defaultDepth, result.maxDepth);

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RegionDepthLookup::getDefaultDepth(double lat, double lon) const {
	return getConstraints(lat, lon).defaultDepth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RegionDepthLookup::getMaxDepth(double lat, double lon) const {
	return getConstraints(lat, lon).maxDepth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t RegionDepthLookup::regionCount() const {
	return _regions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<std::string> RegionDepthLookup::regionNames() const {
	std::vector<std::string> names;
	names.reserve(_regions.size());

	for ( const Geo::GeoFeature *region : _regions ) {
		names.push_back(region->name());
	}

	return names;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RegionDepthLookup::parseDepthAttribute(const Geo::GeoFeature *feature,
                                            const std::string &attrName,
                                            double &value) const {
	const Geo::GeoFeature::Attributes &attrs = feature->attributes();
	auto it = attrs.find(attrName);

	if ( it == attrs.end() ) {
		return false;
	}

	if ( !Core::fromString(value, it->second) ) {
		SEISCOMP_WARNING("Failed to parse %s='%s' for region '%s'",
		                 attrName.c_str(), it->second.c_str(),
		                 feature->name().c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Autoloc
} // namespace Seismology
} // namespace Seiscomp
