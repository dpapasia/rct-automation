/*
 *   Copyright 2012-2014 Google, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */


#include <iostream>
#include "http.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <pion/http/request.hpp>

using HTTPRequestPtr = pion::http::request_ptr;

using namespace std;

void WebCommand::handle_command(HTTPRequestPtr http_request, const pion::tcp::connection_ptr& tcp_conn) {
  pion::http::response_writer_ptr
    writer(pion::http::response_writer::create(tcp_conn,
                                      *http_request));
  pion::http::response& r = writer->get_response();
  VLOG(60) << "Resource: " << http_request->get_original_resource();
  VLOG(60) << "Query string: " << http_request->get_query_string();

  r.set_status_code(HTTPTypes::RESPONSE_CODE_OK);
  r.set_status_message(HTTPTypes::RESPONSE_MESSAGE_OK);
  params_ = http_request->get_queries();
  request_ = http_request;
  writer_ = writer;
  SSL *cert = tcp_conn->get_ssl_socket().impl()->ssl;

  if (cert) {
    X509 *info = SSL_get_peer_certificate(cert);
    if (info) {
      char buf[512];
      X509_NAME_oneline(X509_get_subject_name(info), buf, sizeof buf);
      remote_user_ = buf;
    }
  }

  LOG(INFO) << "API command " << request_->get_resource() << " from " << tcp_conn->get_remote_ip() << " " << remote_user_ << " running now...";
  this->handle_command(http_request, writer, remote_user_);

  writer->send();
}

void WebCommand::ReturnMessage(const ::google::protobuf::Message& value) {
  std::string format;
  if (params_.count("format")) {
    format = params_.equal_range("format").first->second;
  } else {
    format = "pb";
  }
  // TODO throw up some headers for the json users to know more about what they have
  if (format == "debugpb") {
    writer_ << value.DebugString();  
  } else if (format == "json") {
    writer_->get_response().set_content_type("application/json");
    std::string output;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    google::protobuf::util::MessageToJsonString(value, &output, options);
    writer_ << output;
    return;
  } else {
    writer_->get_response().set_content_type("application/x-protobuf; desc=\"/pb/"+value.GetTypeName()+".desc\"; messageType=\""+value.GetTypeName()+"\";);");
    writer_ << value.SerializeAsString();
    VLOG(5) << "Sent proto of size " << value.SerializeAsString().size() << " on wire";
  }
} 

template <>
int64_t ParseString(const std::string &arg) {
  return std::stoll(arg);
}

template <>
double ParseString(const std::string &arg) {
  return std::stod(arg);
}

