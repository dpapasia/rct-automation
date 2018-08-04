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

#include <deque>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include "dirent.h"
#include <stdlib.h>
#include "playableitem.h"
#include "mplayersession.h"
#include "stdio.h"
#include <string>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <sys/stat.h>
#include <glog/logging.h>
#include "fcntl.h"
#include "playerstate.pb.h"
#include <mpv/client.h>
#include <gflags/gflags.h>

MplayerSession::MplayerSession() :
  mpv_(mpv_create()) {

  CHECK(mpv_observe_property(mpv_, 0, "pause", MPV_FORMAT_FLAG) == 0);
  CHECK(mpv_observe_property(mpv_, 0, "time-pos", MPV_FORMAT_DOUBLE) == 0);
  CHECK(mpv_observe_property(mpv_, 0, "length", MPV_FORMAT_DOUBLE) == 0);
  CHECK(mpv_observe_property(mpv_, 0, "metadata", MPV_FORMAT_STRING) == 0);
  CHECK(mpv_initialize(mpv_) == 0);
}

bool MplayerSession::Play(PlayableItem& item) {
  if  (item.data().has_playableitemid()) {
    item.IncrementPlaycount();
    item.Update();
  }
  return Play(item.data());
}
bool MplayerSession::Play(const automation::PlayableItem& item) {
  boost::mutex::scoped_lock state_lock(state_mutex_);
  state_.mutable_now_playing()->MergeFrom(item);
  state_lock.unlock();


  LOG(INFO) << "requesting playing of " << item.filename();
  CHECK_NOTNULL(mpv_);

  if (item.type() == automation::PlayableItem::WEBSTREAM) {
    char endpos[16];
    char cache[16];
    snprintf(endpos, sizeof endpos, "%ld", item.duration());
    snprintf(cache, sizeof cache, "%d", item.cache());
    mpv_set_property_string(mpv_, "cache", cache);
    mpv_set_property_string(mpv_, "length", endpos);
  } else {
    mpv_set_property_string(mpv_, "cache", "0");
  }

  const char *args[3];
  args[0] = "loadfile";
  args[1] = item.filename().c_str();
  args[2] = '\0';

 
  CHECK(mpv_command(mpv_, args) == 0);
  while (mpv_) {
    mpv_event *event = mpv_wait_event(mpv_, 0.25);
    if (event->event_id == MPV_EVENT_END_FILE) {
      return true;
    }
    if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
      mpv_event_property *prop = (mpv_event_property*)event->data;
      if (prop->data == nullptr) {
        continue;
      }
      state_lock.lock();
      if (strcmp(prop->name, "pause") == 0) {
        state_.set_paused(*(bool *)prop->data);
			}
			if (!strcmp(prop->name, "time-pos")) {
        state_.set_time_pos(*(double *)prop->data);
      }
			if (!strcmp(prop->name, "length")) state_.set_length(*(double *)prop->data);
			if (!strcmp(prop->name, "metadata")) state_.set_metadata((char *)prop->data);
      state_lock.unlock();
		}
	}
  return false;
}

void MplayerSession::Pause() {
  VLOG(5) << "in player pause";

  int desired = 1;

  mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, &desired);
}
void MplayerSession::Unpause() {
  int desired = 0;
  LOG(INFO) << "in unpause";

  mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, &desired);
}

void MplayerSession::PauseToggle() {
  int desired;

  {
    boost::mutex::scoped_lock state_lock(state_mutex_);
    desired = state_.paused() ? 0 : 1;
  }
  mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, &desired);
}

void MplayerSession::Stop() {
  CHECK(mpv_command_string(mpv_, "playlist-next force") == 0);
}

void MplayerSession::MergeState(automation::PlayerState* dest) {
  boost::mutex::scoped_lock state_lock(state_mutex_);
  VLOG(5) << "Merging out our state: " << state_.DebugString();
  dest->MergeFrom(state_);
}

void MplayerSession::SetSpeed(double speed) {
  CHECK(mpv_set_property(mpv_, "speed", MPV_FORMAT_DOUBLE, &speed) == 0);
}
void MplayerSession::Seek(double timepos) {
  CHECK(mpv_set_property(mpv_, "time_pos", MPV_FORMAT_DOUBLE, &timepos) == 0);
}
  
