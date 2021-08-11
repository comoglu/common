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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/eventparameters_package.h>
#include <seiscomp/datamodel/config_package.h>
#include <seiscomp/datamodel/qualitycontrol_package.h>
#include <seiscomp/datamodel/inventory_package.h>
#include <seiscomp/datamodel/routing_package.h>
#include <seiscomp/datamodel/journaling_package.h>
#include <seiscomp/datamodel/arclinklog_package.h>
#include <seiscomp/datamodel/dataavailability_package.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>

namespace Seiscomp {
namespace DataModel {

#define _T(name) _db->convertColumnName(name)

DatabaseQuery::DatabaseQuery(Seiscomp::IO::DatabaseInterface* dbDriver)
: DatabaseReader(dbDriver) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseQuery::~DatabaseQuery() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station* DatabaseQuery::getStation(const std::string& network_code,
                                   const std::string& station_code,
                                   Seiscomp::Core::Time time) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PStation." + _T("publicID") + ",Station.* from Network,Station,PublicObject as PStation where Station._parent_oid=Network._oid and Station._oid=PStation._oid and Network." + _T("start") + "<='";
	query += toString(time);
	query += "' and (Network." + _T("end") + ">='";
	query += toString(time);
	query += "' or Network." + _T("end") + " is null) and Station." + _T("start") + "<='";
	query += toString(time);
	query += "' and (Station." + _T("end") + ">='";
	query += toString(time);
	query += "' or Station." + _T("end") + " is null) and Network." + _T("code") + "='";
	query += toString(network_code);
	query += "' and Station." + _T("code") + "='";
	query += toString(station_code);
	query += "'";

	return Station::Cast(queryObject(Station::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* DatabaseQuery::getEvent(const std::string& originID) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PEvent." + _T("publicID") + ",Event.* from Event,PublicObject as PEvent,OriginReference where OriginReference._parent_oid=Event._oid and Event._oid=PEvent._oid and OriginReference." + _T("originID") + "='";
	query += toString(originID);
	query += "'";

	return Event::Cast(queryObject(Event::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* DatabaseQuery::getEventByPreferredMagnitudeID(const std::string& magnitudeID) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PEvent." + _T("publicID") + ",Event.* from Event,PublicObject as PEvent where Event._oid=PEvent._oid and Event." + _T("preferredMagnitudeID") + "='";
	query += toString(magnitudeID);
	query += "'";

	return Event::Cast(queryObject(Event::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* DatabaseQuery::getEventForFocalMechanism(const std::string& focalMechanismID) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PEvent." + _T("publicID") + ",Event.* from FocalMechanismReference,Event,PublicObject as PEvent where FocalMechanismReference._parent_oid=Event._oid and Event._oid=PEvent._oid and FocalMechanismReference." + _T("focalMechanismID") + "='";
	query += toString(focalMechanismID);
	query += "'";

	return Event::Cast(queryObject(Event::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* DatabaseQuery::getEventByPublicID(const std::string& eventID) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PEvent." + _T("publicID") + ",Event.* from Event,PublicObject as PEvent where Event._oid=PEvent._oid and PEvent." + _T("publicID") + "='";
	query += toString(eventID);
	query += "'";

	return Event::Cast(queryObject(Event::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Amplitude* DatabaseQuery::getAmplitude(const std::string& pickID,
                                       const std::string& type) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select PAmplitude." + _T("publicID") + ",Amplitude.* from Amplitude,PublicObject as PAmplitude where Amplitude._oid=PAmplitude._oid and Amplitude." + _T("pickID") + "='";
	query += toString(pickID);
	query += "' and Amplitude." + _T("type") + "='";
	query += toString(type);
	query += "'";

	return Amplitude::Cast(queryObject(Amplitude::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getAmplitudes(Seiscomp::Core::Time startTime,
                                              Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PAmplitude." + _T("publicID") + ",Amplitude.* from Amplitude,PublicObject as PAmplitude where Amplitude._oid=PAmplitude._oid and Amplitude." + _T("timeWindow_reference") + ">='";
	query += toString(startTime);
	query += "' and Amplitude." + _T("timeWindow_reference") + "<='";
	query += toString(endTime);
	query += "'";

	return getObjectIterator(query, Amplitude::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getAmplitudesForPick(const std::string& pickID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PAmplitude." + _T("publicID") + ",Amplitude.* from Amplitude,PublicObject as PAmplitude where Amplitude._oid=PAmplitude._oid and Amplitude." + _T("pickID") + "='";
	query += toString(pickID);
	query += "'";

	return getObjectIterator(query, Amplitude::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getAmplitudesForOrigin(const std::string& originID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PAmplitude." + _T("publicID") + ",Amplitude.* from Arrival,Amplitude,PublicObject as PAmplitude,Origin,PublicObject as POrigin where Arrival." + _T("pickID") + "=Amplitude." + _T("pickID") + " and Arrival._parent_oid=Origin._oid and Amplitude._oid=PAmplitude._oid and Origin._oid=POrigin._oid and POrigin." + _T("publicID") + "='";
	query += toString(originID);
	query += "'";

	return getObjectIterator(query, Amplitude::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getOriginsForAmplitude(const std::string& amplitudeID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select POrigin." + _T("publicID") + ",Origin.* from Arrival,Origin,PublicObject as POrigin,Amplitude,PublicObject as PAmplitude where Arrival." + _T("pickID") + "=Amplitude." + _T("pickID") + " and Arrival._parent_oid=Origin._oid and Origin._oid=POrigin._oid and Amplitude._oid=PAmplitude._oid and PAmplitude." + _T("publicID") + "='";
	query += toString(amplitudeID);
	query += "'";

	return getObjectIterator(query, Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* DatabaseQuery::getOriginByMagnitude(const std::string& magnitudeID) {
	if ( !validInterface() ) return NULL;

	std::string query;
	query += "select POrigin." + _T("publicID") + ",Origin.* from Origin,PublicObject as POrigin,Magnitude,PublicObject as PMagnitude where Magnitude._parent_oid=Origin._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude." + _T("publicID") + "='";
	query += toString(magnitudeID);
	query += "'";

	return Origin::Cast(queryObject(Origin::TypeInfo(), query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArrivalsForAmplitude(const std::string& amplitudeID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select Arrival.* from Arrival,Amplitude,PublicObject as PAmplitude where Arrival." + _T("pickID") + "=Amplitude." + _T("pickID") + " and Amplitude._oid=PAmplitude._oid and PAmplitude." + _T("publicID") + "='";
	query += toString(amplitudeID);
	query += "'";

	return getObjectIterator(query, Arrival::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getPicks(const std::string& originID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PPick." + _T("publicID") + ",Pick.* from Arrival,Pick,PublicObject as PPick,Origin,PublicObject as POrigin where Arrival." + _T("pickID") + "=PPick." + _T("publicID") + " and Arrival._parent_oid=Origin._oid and Pick._oid=PPick._oid and Origin._oid=POrigin._oid and POrigin." + _T("publicID") + "='";
	query += toString(originID);
	query += "'";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getPicks(Seiscomp::Core::Time startTime,
                                         Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PPick." + _T("publicID") + ",Pick.* from Pick,PublicObject as PPick where Pick._oid=PPick._oid and Pick." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Pick." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "'";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getPicks(Seiscomp::Core::Time startTime,
                                         Seiscomp::Core::Time endTime,
                                         const WaveformStreamID& waveformID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PPick." + _T("publicID") + ",Pick.* from Pick,PublicObject as PPick where Pick._oid=PPick._oid and Pick." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Pick." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "' and (Pick." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and Pick." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and Pick." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and Pick." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and Pick." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "')";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getWaveformQuality(const std::string& type) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select WaveformQuality.* from WaveformQuality where WaveformQuality." + _T("end") + " is null and WaveformQuality." + _T("type") + "='";
	query += toString(type);
	query += "'";

	return getObjectIterator(query, WaveformQuality::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getWaveformQuality(const WaveformStreamID& waveformID,
                                                   const std::string& parameter,
                                                   Seiscomp::Core::Time startTime,
                                                   Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select WaveformQuality.* from WaveformQuality where WaveformQuality." + _T("type") + "='report' and WaveformQuality." + _T("end") + ">'";
	query += toString(startTime);
	query += "' and WaveformQuality." + _T("start") + "<'";
	query += toString(endTime);
	query += "' and (WaveformQuality." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and WaveformQuality." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and WaveformQuality." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and WaveformQuality." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and WaveformQuality." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "') and WaveformQuality." + _T("parameter") + "='";
	query += toString(parameter);
	query += "'";

	return getObjectIterator(query, WaveformQuality::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getWaveformQuality(Seiscomp::Core::Time startTime,
                                                   Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select WaveformQuality.* from WaveformQuality where WaveformQuality." + _T("type") + "='report' and WaveformQuality." + _T("end") + ">'";
	query += toString(startTime);
	query += "' and WaveformQuality." + _T("start") + "<'";
	query += toString(endTime);
	query += "'";

	return getObjectIterator(query, WaveformQuality::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getWaveformQuality(const WaveformStreamID& waveformID,
                                                   const std::string& parameter,
                                                   const std::string& type,
                                                   Seiscomp::Core::Time startTime,
                                                   Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select WaveformQuality.* from WaveformQuality where WaveformQuality." + _T("end") + ">'";
	query += toString(startTime);
	query += "' and WaveformQuality." + _T("start") + "<'";
	query += toString(endTime);
	query += "' and (WaveformQuality." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and WaveformQuality." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and WaveformQuality." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and WaveformQuality." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and WaveformQuality." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "') and WaveformQuality." + _T("parameter") + "='";
	query += toString(parameter);
	query += "' and WaveformQuality." + _T("type") + "='";
	query += toString(type);
	query += "'";

	return getObjectIterator(query, WaveformQuality::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getWaveformQualityDescending(const WaveformStreamID& waveformID,
                                                             const std::string& parameter,
                                                             const std::string& type) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select WaveformQuality.* from WaveformQuality where (WaveformQuality." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and WaveformQuality." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and WaveformQuality." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and WaveformQuality." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and WaveformQuality." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "') and WaveformQuality." + _T("parameter") + "='";
	query += toString(parameter);
	query += "' and WaveformQuality." + _T("type") + "='";
	query += toString(type);
	query += "' order by WaveformQuality._oid desc limit 10";

	return getObjectIterator(query, WaveformQuality::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getOutage(const WaveformStreamID& waveformID,
                                          Seiscomp::Core::Time startTime,
                                          Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select Outage.* from Outage where Outage." + _T("start") + "<'";
	query += toString(endTime);
	query += "' and Outage." + _T("end") + ">'";
	query += toString(startTime);
	query += "' and (Outage." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and Outage." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and Outage." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and Outage." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and Outage." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "')";

	return getObjectIterator(query, Outage::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getQCLog(const WaveformStreamID& waveformID,
                                         Seiscomp::Core::Time startTime,
                                         Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PQCLog." + _T("publicID") + ",QCLog.* from QCLog,PublicObject as PQCLog where QCLog._oid=PQCLog._oid and QCLog." + _T("end") + ">'";
	query += toString(startTime);
	query += "' and QCLog." + _T("start") + "<'";
	query += toString(endTime);
	query += "' and (QCLog." + _T("waveformID_networkCode") + "='";
	query += toString(waveformID.networkCode());
	query += "' and QCLog." + _T("waveformID_stationCode") + "='";
	query += toString(waveformID.stationCode());
	query += "' and QCLog." + _T("waveformID_locationCode") + "='";
	query += toString(waveformID.locationCode());
	query += "' and QCLog." + _T("waveformID_channelCode") + "='";
	query += toString(waveformID.channelCode());
	query += "' and QCLog." + _T("waveformID_resourceURI") + "='";
	query += toString(waveformID.resourceURI());
	query += "')";

	return getObjectIterator(query, QCLog::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getPreferredOrigins(Seiscomp::Core::Time startTime,
                                                    Seiscomp::Core::Time endTime,
                                                    const std::string& referenceOriginID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select POrigin." + _T("publicID") + ",Origin.* from Event,Origin,PublicObject as POrigin where POrigin." + _T("publicID") + "=Event." + _T("preferredOriginID") + " and Origin._oid=POrigin._oid and Origin." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Origin." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "' and POrigin." + _T("publicID") + "!='";
	query += toString(referenceOriginID);
	query += "'";

	return getObjectIterator(query, Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getPreferredMagnitudes(Seiscomp::Core::Time startTime,
                                                       Seiscomp::Core::Time endTime,
                                                       const std::string& referenceMagnitudeID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PMagnitude." + _T("publicID") + ",Magnitude.* from Origin,PublicObject as POrigin,Event,Magnitude,PublicObject as PMagnitude where PMagnitude." + _T("publicID") + "=Event." + _T("preferredMagnitudeID") + " and POrigin." + _T("publicID") + "=Event." + _T("preferredOriginID") + " and Magnitude._parent_oid=Origin._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and Origin." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Origin." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "' and PMagnitude." + _T("publicID") + "!='";
	query += toString(referenceMagnitudeID);
	query += "'";

	return getObjectIterator(query, Magnitude::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEvents(Seiscomp::Core::Time startTime,
                                          Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PEvent." + _T("publicID") + ",Event.* from Event,PublicObject as PEvent,Origin,PublicObject as POrigin where POrigin." + _T("publicID") + "=Event." + _T("preferredOriginID") + " and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Origin." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Origin." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "'";

	return getObjectIterator(query, Event::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getOrigins(const std::string& eventID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select POrigin." + _T("publicID") + ",Origin.* from OriginReference,Event,PublicObject as PEvent,Origin,PublicObject as POrigin where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and PEvent." + _T("publicID") + "='";
	query += toString(eventID);
	query += "' order by Origin." + _T("creationInfo_creationTime") + " asc";

	return getObjectIterator(query, Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getOriginsDescending(const std::string& eventID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select POrigin." + _T("publicID") + ",Origin.* from OriginReference,Event,PublicObject as PEvent,Origin,PublicObject as POrigin where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and PEvent." + _T("publicID") + "='";
	query += toString(eventID);
	query += "' order by Origin." + _T("creationInfo_creationTime") + " desc";

	return getObjectIterator(query, Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getFocalMechanismsDescending(const std::string& eventID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PFocalMechanism." + _T("publicID") + ",FocalMechanism.* from FocalMechanismReference,FocalMechanism,PublicObject as PFocalMechanism,Event,PublicObject as PEvent where FocalMechanismReference." + _T("focalMechanismID") + "=PFocalMechanism." + _T("publicID") + " and FocalMechanismReference._parent_oid=Event._oid and FocalMechanism._oid=PFocalMechanism._oid and Event._oid=PEvent._oid and PEvent." + _T("publicID") + "='";
	query += toString(eventID);
	query += "' order by FocalMechanism." + _T("creationInfo_creationTime") + " desc";

	return getObjectIterator(query, FocalMechanism::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEventPickIDs(const std::string& publicID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(Arrival." + _T("pickID") + ") from Arrival,OriginReference,Event,PublicObject as PEvent,Origin,PublicObject as POrigin where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Arrival._parent_oid=Origin._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and PEvent." + _T("publicID") + "='";
	query += toString(publicID);
	query += "'";

	return getObjectIterator(query, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEventPickIDsByWeight(const std::string& publicID,
                                                        double weight) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(Arrival." + _T("pickID") + ") from Arrival,OriginReference,Event,PublicObject as PEvent,Origin,PublicObject as POrigin where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Arrival._parent_oid=Origin._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and (Arrival." + _T("weight") + ">'";
	query += toString(weight);
	query += "' or Arrival." + _T("weight") + " is null) and PEvent." + _T("publicID") + "='";
	query += toString(publicID);
	query += "'";

	return getObjectIterator(query, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEventPicks(const std::string& eventID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(PPick." + _T("publicID") + "),Pick.* from Event,PublicObject as PEvent,Origin,PublicObject as POrigin,Arrival,Pick,PublicObject as PPick,OriginReference where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and Arrival." + _T("pickID") + "=PPick." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Arrival._parent_oid=Origin._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Pick._oid=PPick._oid and PEvent." + _T("publicID") + "='";
	query += toString(eventID);
	query += "'";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEventPicksByWeight(const std::string& publicID,
                                                      double weight) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(PPick." + _T("publicID") + "),Pick.* from Event,PublicObject as PEvent,Origin,PublicObject as POrigin,Arrival,Pick,PublicObject as PPick,OriginReference where OriginReference." + _T("originID") + "=POrigin." + _T("publicID") + " and Arrival." + _T("pickID") + "=PPick." + _T("publicID") + " and OriginReference._parent_oid=Event._oid and Arrival._parent_oid=Origin._oid and Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Pick._oid=PPick._oid and (Arrival." + _T("weight") + ">'";
	query += toString(weight);
	query += "' or Arrival." + _T("weight") + " is null) and PEvent." + _T("publicID") + "='";
	query += toString(publicID);
	query += "'";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getConfigModule(const std::string& name,
                                                bool enabled) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PConfigModule." + _T("publicID") + ",ConfigModule.* from ConfigModule,PublicObject as PConfigModule where ConfigModule._oid=PConfigModule._oid and ConfigModule." + _T("name") + "='";
	query += toString(name);
	query += "' and ConfigModule." + _T("enabled") + "='";
	query += toString(enabled);
	query += "'";

	return getObjectIterator(query, ConfigModule::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getEquivalentPick(const std::string& stationCode,
                                                  const std::string& networkCode,
                                                  const std::string& locationCode,
                                                  const std::string& channelCode,
                                                  Seiscomp::Core::Time startTime,
                                                  Seiscomp::Core::Time endTime) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PPick." + _T("publicID") + ",Pick.* from Pick,PublicObject as PPick where Pick._oid=PPick._oid and Pick." + _T("time_value") + ">='";
	query += toString(startTime);
	query += "' and Pick." + _T("time_value") + "<='";
	query += toString(endTime);
	query += "' and Pick." + _T("waveformID_stationCode") + "='";
	query += toString(stationCode);
	query += "' and Pick." + _T("waveformID_networkCode") + "='";
	query += toString(networkCode);
	query += "' and Pick." + _T("waveformID_locationCode") + "='";
	query += toString(locationCode);
	query += "' and Pick." + _T("waveformID_channelCode") + "='";
	query += toString(channelCode);
	query += "'";

	return getObjectIterator(query, Pick::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getJournal(const std::string& objectID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select JournalEntry.* from JournalEntry where JournalEntry." + _T("objectID") + "='";
	query += toString(objectID);
	query += "'";

	return getObjectIterator(query, JournalEntry::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getJournalAction(const std::string& objectID,
                                                 const std::string& action) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select JournalEntry.* from JournalEntry where JournalEntry." + _T("objectID") + "='";
	query += toString(objectID);
	query += "' and JournalEntry." + _T("action") + "='";
	query += toString(action);
	query += "'";

	return getObjectIterator(query, JournalEntry::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequestByStreamCode(Seiscomp::Core::Time startTime,
                                                              Seiscomp::Core::Time endTime,
                                                              const std::string& networkCode,
                                                              const std::string& stationCode,
                                                              const std::string& locationCode,
                                                              const std::string& channelCode,
                                                              const std::string& type) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(PArclinkRequest." + _T("publicID") + "),ArclinkRequest.* from ArclinkRequestLine,ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequestLine._parent_oid=ArclinkRequest._oid and ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("created") + ">'";
	query += toString(startTime);
	query += "' and ArclinkRequest." + _T("created") + "<'";
	query += toString(endTime);
	query += "' and ArclinkRequest." + _T("type") + " like '";
	query += toString(type);
	query += "' and ArclinkRequestLine." + _T("streamID_networkCode") + "='";
	query += toString(networkCode);
	query += "' and ArclinkRequestLine." + _T("streamID_stationCode") + "='";
	query += toString(stationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_locationCode") + "='";
	query += toString(locationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_channelCode") + "='";
	query += toString(channelCode);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequestByRequestID(const std::string& requestID) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PArclinkRequest." + _T("publicID") + ",ArclinkRequest.* from ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("requestID") + "='";
	query += toString(requestID);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequestByUserID(const std::string& userID,
                                                          Seiscomp::Core::Time startTime,
                                                          Seiscomp::Core::Time endTime,
                                                          const std::string& type) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PArclinkRequest." + _T("publicID") + ",ArclinkRequest.* from ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("userID") + " like '";
	query += toString(userID);
	query += "' and ArclinkRequest." + _T("created") + ">'";
	query += toString(startTime);
	query += "' and ArclinkRequest." + _T("created") + "<'";
	query += toString(endTime);
	query += "' and ArclinkRequest." + _T("type") + " like '";
	query += toString(type);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequestByTime(Seiscomp::Core::Time startTime,
                                                        Seiscomp::Core::Time endTime,
                                                        const std::string& type) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select PArclinkRequest." + _T("publicID") + ",ArclinkRequest.* from ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("created") + ">'";
	query += toString(startTime);
	query += "' and ArclinkRequest." + _T("created") + "<'";
	query += toString(endTime);
	query += "' and ArclinkRequest." + _T("type") + " like '";
	query += toString(type);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequest(const std::string& userID,
                                                  Seiscomp::Core::Time startTime,
                                                  Seiscomp::Core::Time endTime,
                                                  const std::string& networkCode,
                                                  const std::string& stationCode,
                                                  const std::string& locationCode,
                                                  const std::string& channelCode,
                                                  const std::string& type,
                                                  const std::string& netClass) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(PArclinkRequest." + _T("publicID") + "),ArclinkRequest.* from ArclinkRequestLine,ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequestLine._parent_oid=ArclinkRequest._oid and ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("userID") + " like '";
	query += toString(userID);
	query += "' and ArclinkRequest." + _T("created") + ">'";
	query += toString(startTime);
	query += "' and ArclinkRequest." + _T("created") + "<'";
	query += toString(endTime);
	query += "' and ArclinkRequestLine." + _T("streamID_networkCode") + " like '";
	query += toString(networkCode);
	query += "' and ArclinkRequestLine." + _T("streamID_stationCode") + " like '";
	query += toString(stationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_locationCode") + " like '";
	query += toString(locationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_channelCode") + " like '";
	query += toString(channelCode);
	query += "' and ArclinkRequest." + _T("type") + " like '";
	query += toString(type);
	query += "' and ArclinkRequestLine." + _T("netClass") + " like '";
	query += toString(netClass);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseQuery::getArclinkRequestRestricted(const std::string& userID,
                                                            Seiscomp::Core::Time startTime,
                                                            Seiscomp::Core::Time endTime,
                                                            const std::string& networkCode,
                                                            const std::string& stationCode,
                                                            const std::string& locationCode,
                                                            const std::string& channelCode,
                                                            const std::string& type,
                                                            const std::string& netClass,
                                                            bool restricted) {
	if ( !validInterface() ) return DatabaseIterator();

	std::string query;
	query += "select distinct(PArclinkRequest." + _T("publicID") + "),ArclinkRequest.* from ArclinkRequestLine,ArclinkRequest,PublicObject as PArclinkRequest where ArclinkRequestLine._parent_oid=ArclinkRequest._oid and ArclinkRequest._oid=PArclinkRequest._oid and ArclinkRequest." + _T("userID") + " like '";
	query += toString(userID);
	query += "' and ArclinkRequest." + _T("created") + ">'";
	query += toString(startTime);
	query += "' and ArclinkRequest." + _T("created") + "<'";
	query += toString(endTime);
	query += "' and ArclinkRequestLine." + _T("streamID_networkCode") + " like '";
	query += toString(networkCode);
	query += "' and ArclinkRequestLine." + _T("streamID_stationCode") + " like '";
	query += toString(stationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_locationCode") + " like '";
	query += toString(locationCode);
	query += "' and ArclinkRequestLine." + _T("streamID_channelCode") + " like '";
	query += toString(channelCode);
	query += "' and ArclinkRequest." + _T("type") + " like '";
	query += toString(type);
	query += "' and ArclinkRequestLine." + _T("netClass") + " like '";
	query += toString(netClass);
	query += "' and ArclinkRequestLine." + _T("restricted") + "='";
	query += toString(restricted);
	query += "'";

	return getObjectIterator(query, ArclinkRequest::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
