# Copyright 2012-2018 Google, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

proto_library(
  name = "playableitem_proto",
  srcs = ["playableitem.proto"],
)
proto_library(
  name = "playerstate_proto",
  srcs = ["playerstate.proto"],
  deps = [":playableitem_proto"],
)
proto_library(
  name = "playlist_proto",
  srcs = ["playlist.proto"],
  deps = [":playableitem_proto"],
)
proto_library(
  name = "protostore_proto",
  srcs = ["protostore.proto"],
)
proto_library(
  name = "requirement_proto",
  srcs = ["requirement.proto"],
  deps = [":playlist_proto", ":playableitem_proto"],
)
proto_library(
  name = "sql_proto",
  srcs = ["sql.proto"],
)
cc_proto_library(
  name = "playableitem_cc_proto",
  deps = [":playableitem_proto"],
)
cc_proto_library(
  name = "playerstate_cc_proto",
  deps = [":playerstate_proto"],
)
cc_proto_library(
  name = "playlist_cc_proto",
  deps = [":playlist_proto"],
)
cc_proto_library(
  name = "protostore_cc_proto",
  deps = [":protostore_proto"],
)
cc_proto_library(
  name = "requirement_cc_proto",
  deps = [":requirement_proto"],
)
cc_proto_library(
  name = "sql_cc_proto",
  deps = [":sql_proto"],
)

cc_library(
  name = "actions",
  srcs = ["actions.cc"],
  deps = [":base", ":db", ":automationstate"],
  alwayslink = 1,
)
cc_library(
  name = "db",
  srcs = ["db.cc"],
  hdrs = ["db.h"],
  deps = ["@com_github_glog_glog//:glog", ":playlist", ":playlist_cc_proto", ":protostore"],
)
cc_library(
  name = "automationstate",
  srcs = ["automationstate.cc"],
  hdrs = ["automationstate.h"],
  deps = [":base", ":playlist", ":mplayersession", ":requirementengine"],

)
cc_library(
  name = "http",
  srcs = ["http.cc"],
  hdrs = ["http.h"],
  deps = [":base"],
)
cc_library(
  name = "base",
  hdrs = ["registerable-inl.h", "base.h"],
  deps = ["@com_github_glog_glog//:glog", "@com_google_protobuf//:protobuf"],
)

cc_library(
  name = "mplayersession",
  srcs = ["mplayersession.cc"],
  hdrs = ["mplayersession.h"],
  deps = [":playerstate_cc_proto", ":playableitem", ":protostore"],
)
cc_library(
  name = "playableitem",
  srcs = ["playableitem.cc"],
  hdrs = ["playableitem.h"],
  deps = [":playableitem_cc_proto", ":protostore", ":base"]
)
cc_library(
  name = "playlist",
  srcs = ["playlist.cc"],
  hdrs = ["playlist.h"],
  deps = [":base", ":playableitem", ":playlist_cc_proto"],
)
cc_library(
  name = "messagestore",
  hdrs = ["messagestore.h"],
  srcs = ["messagestore.cc"],
  deps = [":base"],
)
cc_library(
  name = "protostore",
  hdrs = ["protostore.h"],
  deps = [":protostore_cc_proto", ":messagestore"],
)
cc_library(
  name = "requirementengine",
  srcs = ["requirementengine.cc"],
  hdrs = ["requirementengine.h"],
  deps = [":base", ":protostore", ":playerstate_cc_proto", ":requirement_cc_proto"],
)
cc_library(
  name = "webapi",
  srcs = ["webapi.cc"],
  deps = [":automationstate", ":http", ":db", ":sql_cc_proto"],
)
cc_binary(
  name = "acmd",
  srcs = ["acmd-main.cc"],
  deps = [":db", ":base", ":automationstate", ":http", ":mplayersession", ":playableitem", ":playlist", ":requirementengine", ":playlist_cc_proto", ":protostore", "@com_github_gflags_gflags//:gflags"],
  linkopts = ["-lsqlite3", "-lssl", "-lcrypto", "-lboost_system"],
)
cc_binary(
  name = "automation",
  srcs = ["automation.cc"],
  deps = [":db", ":base", ":automationstate", ":http", ":mplayersession", ":playableitem", ":playlist", ":requirementengine", ":playlist_cc_proto", ":protostore", "@com_github_gflags_gflags//:gflags", ":webapi"],
  linkopts = ["-lsqlite3", "-lssl", "-lcrypto", "-lboost_system", "-lpion", "-llog4cpp", "-lboost_thread"],
)


