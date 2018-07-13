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
)
proto_library(
  name = "playlist_proto",
  srcs = ["playlist.proto"],
)
proto_library(
  name = "protostore_proto",
  srcs = ["protostore.proto"],
)
proto_library(
  name = "requirement_proto",
  srcs = ["requirement.proto"],
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
  srcs = ["actions.cc"]
)
cc_library(
  name = "db",
  srcs = ["db.cc"],
  hdrs = ["db.h"],
  deps = ["@com_github_glog_glog//:glog"],
)
cc_library(
  name = "base",
  hdrs = ["base.h"],
)
cc_library(
  name = "automationstate",
  srcs = ["automationstate.cc"],
  hdrs = ["automationstate.h"],
  deps = [":base", ":playlist", ":playableitem"],

)
cc_library(
  name = "http",
  srcs = ["http.cc"],
  hdrs = ["http.h"],
)
cc_library(
  name = "mplayersession",
  srcs = ["mplayersession.cc"],
  hdrs = ["mplayersession.h"],
  deps = [":playableitem"],
)
cc_library(
  name = "playableitem",
  srcs = ["playableitem.cc"],
  hdrs = ["playableitem.h"],
  deps = [":playableitem_cc_proto", ":playlist_cc_proto"]
)
cc_library(
  name = "playlist",
  srcs = ["playlist.cc"],
  hdrs = ["playlist.h"],
)
cc_library(
  name = "protostore",
  hdrs = ["protostore.h"],
  deps = [":protostore_cc_proto"],
)
cc_library(
  name = "requirementengine",
  srcs = ["requirementengine.cc"],
  hdrs = ["requirementengine.h"],
  deps = [":automationstate"],
)

cc_binary(
  name = "acmd",
  srcs = ["acmd-main.cc"],
  deps = [":db", ":base", ":automationstate", ":http", ":mplayersession", ":playableitem", ":playlist", ":requirementengine", ":playlist_cc_proto", ":protostore", "@com_github_gflags_gflags//:gflags"]
)
cc_binary(
  name = "automation",
  srcs = ["automation.cc"],
  deps = [":db", ":base", ":automationstate", ":http", ":mplayersession", ":playableitem", ":playlist", ":requirementengine", ":playlist_cc_proto", ":protostore", "@com_github_gflags_gflags//:gflags"]
)


