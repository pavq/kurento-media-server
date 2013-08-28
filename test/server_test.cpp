/*
 * server_test.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server_test_base.hpp"
#include <boost/test/unit_test.hpp>

#include "mediaServer_constants.h"

#include <gst/gst.h>

using namespace kurento;

#define GST_CAT_DEFAULT _server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test"

BOOST_FIXTURE_TEST_SUITE ( server_test_suite,  F)

static void
check_version (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  int32_t v;
  int32_t gotVersion;
  mediaServerConstants *c;

  c = new mediaServerConstants();
  v = c->VERSION;
  delete c;

  gotVersion = client->getVersion();
  BOOST_CHECK_EQUAL (gotVersion, v);
}

static void
check_no_handler (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();

  BOOST_CHECK_THROW (client->createMediaPipeline (mediaPipeline, 0), HandlerNotFoundException);
}

static void
check_add_handler_address (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  client->addHandlerAddress (0, "localhost", 2323);
  client->addHandlerAddress (0, "localhost", 3434);
}

static void
check_type (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);
  BOOST_CHECK (mediaPipeline.type.__isset.mediaObject);
  BOOST_CHECK_EQUAL (mediaPipeline.type.mediaObject, MediaObjectType::type::MEDIA_PIPELINE);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::RTP_END_POINT);
  BOOST_CHECK (mo.type.__isset.sdpEndPoint);
  BOOST_CHECK_EQUAL (mo.type.sdpEndPoint, SdpEndPointType::type::RTP_END_POINT);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::WEBRTC_END_POINT);
  BOOST_CHECK (mo.type.__isset.sdpEndPoint);
  BOOST_CHECK_EQUAL (mo.type.sdpEndPoint, SdpEndPointType::type::WEBRTC_END_POINT);

  client->createUriEndPoint (mo, mediaPipeline, UriEndPointType::type::PLAYER_END_POINT, "");
  BOOST_CHECK (mo.type.__isset.uriEndPoint);
  BOOST_CHECK_EQUAL (mo.type.uriEndPoint, UriEndPointType::type::PLAYER_END_POINT);

  client->createUriEndPoint (mo, mediaPipeline, UriEndPointType::type::RECORDER_END_POINT, "");
  BOOST_CHECK (mo.type.__isset.uriEndPoint);
  BOOST_CHECK_EQUAL (mo.type.uriEndPoint, UriEndPointType::type::RECORDER_END_POINT);

  client->createHttpEndPoint (mo, mediaPipeline);
  BOOST_CHECK (mo.type.__isset.endPoint);
  BOOST_CHECK_EQUAL (mo.type.endPoint, EndPointType::type::HTTP_END_POINT);

  client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER);
  BOOST_CHECK (mo.type.__isset.mixerType);
  BOOST_CHECK_EQUAL (mo.type.mixerType, MixerType::type::MAIN_MIXER);

  client->release (mediaPipeline);
}

static void
check_same_token (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);

  client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER);
  BOOST_CHECK_EQUAL (mediaPipeline.token, mo.token);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::RTP_END_POINT);
  BOOST_CHECK_EQUAL (mediaPipeline.token, mo.token);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::WEBRTC_END_POINT);
  BOOST_CHECK_EQUAL (mediaPipeline.token, mo.token);

  client->release (mediaPipeline);
}

static void
check_use_released_media_pipeline (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);
  client->release (mediaPipeline);
  BOOST_CHECK_THROW (client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER), MediaObjectNotFoundException);
}

static void
check_parent (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();
  MediaObjectId parent = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);

  client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER);
  client->getParent (parent, mo);
  BOOST_CHECK_EQUAL (mediaPipeline.id, parent.id);

  client->release (mediaPipeline);
}

static void
check_get_parent_in_released_media_pipeline (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();
  MediaObjectId parent = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);
  client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER);
  client->release (mediaPipeline);

  BOOST_CHECK_THROW (client->getParent (parent, mo), MediaObjectNotFoundException);
}

static void
check_media_pipeline_no_parent (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId parent = MediaObjectId();

  GST_DEBUG ("check_media_pipeline_no_parent test");
  client->createMediaPipeline (mediaPipeline, 0);
  BOOST_CHECK_THROW (client->getParent (parent, mediaPipeline), NoParentException);

  client->release (mediaPipeline);
}

static void
check_uri_end_point (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId uriEp = MediaObjectId();
  std::string uri, out;

  client->createMediaPipeline (mediaPipeline, 0);

  uri = "/player_end_point/uri";
  client->createUriEndPoint (uriEp, mediaPipeline, UriEndPointType::type::PLAYER_END_POINT, uri);
  client->getUri (out, uriEp);
  BOOST_CHECK_EQUAL (uri, out);
  client->start (uriEp);
  client->pause (uriEp);
  client->stop (uriEp);

  uri = "/recorder_end_point/uri";
  client->createUriEndPoint (uriEp, mediaPipeline, UriEndPointType::type::RECORDER_END_POINT, uri);
  client->getUri (out, uriEp);
  BOOST_CHECK_EQUAL (uri, out);
  client->start (uriEp);
  client->pause (uriEp);
  client->stop (uriEp);

  client->release (mediaPipeline);
}

static void
check_http_end_point (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId httpEp = MediaObjectId();
  std::string out;

  client->createMediaPipeline (mediaPipeline, 0);

  client->createHttpEndPoint (httpEp, mediaPipeline);
  client->getUrl (out, httpEp);

  client->release (mediaPipeline);
}

static void
check_zbar_filter (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId zbarFilter = MediaObjectId();
  std::string out;

  client->createMediaPipeline (mediaPipeline, 0);
  client->createFilter (zbarFilter, mediaPipeline, FilterType::type::ZBAR_FILTER);
  client->release (mediaPipeline);
}

static void
client_side (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  check_version (client);
  check_no_handler (client);
  check_add_handler_address (client);
  check_use_released_media_pipeline (client);
  check_type (client);
  check_same_token (client);
  check_parent (client);
  check_get_parent_in_released_media_pipeline (client);
  check_media_pipeline_no_parent (client);
  check_uri_end_point (client);
  check_http_end_point (client);
  check_zbar_filter (client);
}

BOOST_AUTO_TEST_CASE ( server_test )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);
  client_side (client);
}

BOOST_AUTO_TEST_SUITE_END()
