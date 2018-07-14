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
#ifndef HTTP_H
#define HTTP_H

#include <functional>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <glog/logging.h>
#include <set>
#include "registerable-inl.h"
#include "sqlite3.h"
#include <google/protobuf/util/json_util.h>
#include <pion/http/request.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/tcp/connection.hpp>

using HTTPRequestPtr = pion::http::request_ptr;
using TCPConnectionPtr = pion::tcp::connection_ptr;
using HTTPResponseWriterPtr = pion::http::response_writer_ptr;
using HTTPTypes = pion::http::types;

namespace google { namespace protobuf { class Message; } }

template<class Type> Type ParseString(const std::string &arg);

class WebAPI {
 public:
  static std::string apikey;
  static std::set<std::string> superusers;
  static bool ReadFromDatabase(sqlite3 *db);
  static bool is_superuser(std::string username);
  WebAPI() {
  }
  virtual ~WebAPI();
  typedef std::function<void(HTTPRequestPtr, const TCPConnectionPtr&)> web_callback;
  REGISTER_REGISTRAR(WebAPI, web_callback);
  
 private:
};

class WebCommand : public WebAPI::Registrar {
  virtual void handle_command(HTTPRequestPtr&, HTTPResponseWriterPtr, const std::string&) = 0;
  void handle_command(HTTPRequestPtr, const TCPConnectionPtr& tcp_conn); 
  WebAPI::web_callback get_callback() {
    return [this](HTTPRequestPtr request, const TCPConnectionPtr& conn) {
      handle_command(request, conn);
    };
  }
 protected:
  void ReturnMessage(const google::protobuf::Message&);

  template<class Type>
  Type ArgumentOrDefault(const std::string &arg, Type default_retval) { 
    if (params_.count(arg)) {
      return ParseString<Type>(params_.equal_range(arg).first->second);
    } else {
      return default_retval;
    }
  }
  
  template <class Type>
  Type LoadMessage() {
    const std::string req(request_->get_content(), request_->get_content_length());
    std::string format;
    if (params_.count("format")) {
      format = params_.equal_range("format").first->second;
    } else {
      format = "pb";
    }
    Type input;
    if (format == "json") {
      google::protobuf::util::JsonStringToMessage(req, &input);
    } else if(format == "pb") {
      VLOG(5) << "Loading protobuf of size " << req.size();
      input.ParseFromString(req);
    } else {
      throw std::invalid_argument("Request body is not of valid type.");
    }
    input.DiscardUnknownFields();
    return input;
  }

  pion::ihash_multimap params_;
  HTTPRequestPtr request_;
  HTTPResponseWriterPtr writer_;
  std::string remote_user_; 
};

#endif
