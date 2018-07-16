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
#include <boost/tokenizer.hpp>
#include "playableitem.h"
#include <glog/logging.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sqlite3.h"
#include <regex>
#include <boost/weak_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mpv/client.h>
#ifdef USE_RE2
#include <re2/re2.h>
#else
#include <sys/types.h>
#include <regex.h>
#endif
#include "playableitem.pb.h"
#include "protostore.h"

bool PlayableItem::fetch(const std::string& filename) {
  boost::mutex::scoped_lock lock(mutex_);
  canonical_.Clear();
  canonical_.set_filename(filename);
  bool result = Load(&canonical_);
  LOG(INFO) << canonical_.DebugString();

  if (canonical_.has_filename() && !canonical_.has_duration()) {
    canonical_.set_duration(CalculateDuration());
  }
  
  return result;
}

PlayableItem::PlayableItem(sqlite3 *db) :
  automation::ThreadSafeProto<automation::PlayableItem>(db) {
}

void PlayableItem::IncrementPlaycount() {
  // There is a slight race here, because if we are being incremented in two
  // threads then it's possible we were both created with original state,
  // and we'll only count this once.  But triggering this would require, say,
  // playback through the API at the same time as through the main thread,
  // which would maybe result in a neat double-tracking effect but probably only
  // really be one logical playback as far as anyone is concerned, so this doesn't
  // really bother me.  If it bothers you, however, we could just do a SQL UPDATE
  // here instead.
  boost::mutex::scoped_lock lock(mutex_);
  canonical_.set_playcount(canonical_.playcount() + 1);
}

#ifdef USE_RE2
bool PlayableItem::matches(const RE2& re) {
  CHECK(re.ok());
  boost::mutex::scoped_lock lock(mutex_);
  return RE2::PartialMatch(canonical_.description(), re) || RE2::PartialMatch(canonical_.filename(), re);
}
#else
bool PlayableItem::matches(const regex_t& re) {
  boost::mutex::scoped_lock lock(mutex_);
  return !regexec(&re, canonical_.description().c_str(), 0, NULL, 0) ||
         !regexec(&re, canonical_.filename().c_str(), 0, NULL, 0);
}
#endif

int PlayableItem::CalculateDuration() {
  std::string filename;
  
  if (!canonical_.has_filename()) {
    LOG(INFO) << "Asked about duration but provided no filename";
    return -1;
  }
  filename = canonical_.filename();

  struct stat statobj;
  if (stat(filename.c_str(), &statobj) || !statobj.st_size || !S_ISREG(statobj.st_mode)) {
    LOG(INFO) << "Asked about duration of an invalid file " << filename;
    return -1;
  }

  auto handle = mpv_create();
  CHECK(mpv_initialize(handle) == 0);
  int error = mpv_set_property_string(handle, "ao", "pcm"); //:file=/dev/null");
  CHECK(mpv_observe_property(handle, 1, "duration", MPV_FORMAT_DOUBLE) == 0);
  CHECK(error == 0) << mpv_error_string(error);

  const char *args[3];
  args[0] = "loadfile";
  args[1] = filename.c_str();
  args[2] = '\0';

  mpv_command(handle, args);
  double max = 0;
  while (handle) {
    mpv_event *event = mpv_wait_event(handle, 0.25);
    if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
      mpv_event_property *prop = (mpv_event_property*)event->data;
      if (strcmp(prop->name, "duration") == 0) {
        max = *(double*)prop->data;
      }
    }
    if (event->event_id == MPV_EVENT_END_FILE) {
      mpv_detach_destroy(handle);
      return max;
    }
  }
}
