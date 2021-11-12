//   Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "pose_action.h"
/*
keypoint indexes:
0: 'nose',
1: 'left_eye',
2: 'right_eye',
3: 'left_ear',
4: 'right_ear',
5: 'left_shoulder',
6: 'right_shoulder',
7: 'left_elbow',
8: 'right_elbow',
9: 'left_wrist',
10: 'right_wrist',
11: 'left_hip',
12: 'right_hip',
13: 'left_knee',
14: 'right_knee',
15: 'left_ankle',
16: 'right_ankle'
*/
struct {
  bool mark = false;
  int action_count = 0;
  int latency = 0;
} action_record;

int get_action_count() {
  return action_record.action_count;
}
float get_xyratio(std::vector<float> &kpts_sframe, int index_x, int index_y) {
  float xdiff = std::abs(kpts_sframe[index_x*3 + 1] - kpts_sframe[index_y*3 + 1]);
  float ydiff = std::abs(kpts_sframe[index_x*3 + 2] - kpts_sframe[index_y*3 + 2]);
  return xdiff/ydiff;
}

bool get_xyhigher(std::vector<float> &kpts_sframe, int index_x, int index_y) {
  float ydiff = kpts_sframe[index_x*3 + 2] - kpts_sframe[index_y*3 + 2];
  return ydiff > 0;
}

int check_lateral_raise(std::vector<float> &kpts_sframe) {
  std::vector<int> kpts = {5, 9, 6, 10};
  float down_thres=0.5;
  float up_thres=3.;
  if(kpts_sframe.empty()) {
    return action_record.action_count;
  }
  float xy_ratio;
  if (kpts_sframe[kpts[0]*3] + kpts_sframe[kpts[1]*3] >= kpts_sframe[kpts[2]*3] + kpts_sframe[kpts[3]*3]) {
    xy_ratio = get_xyratio(kpts_sframe, kpts[0], kpts[1]);
  }
  else {
    xy_ratio = get_xyratio(kpts_sframe, kpts[2], kpts[3]);
  }
  if (xy_ratio < down_thres && action_record.mark) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.mark = false;
      action_record.latency = 0;
    }
  }
  else if(xy_ratio > up_thres && !(action_record.mark)) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.action_count += 1;
      action_record.mark = true;
      action_record.latency = 0;
    }
  }
  return action_record.action_count;
}

int check_stand_press(std::vector<float> &kpts_sframe) {
  std::vector<int> kpts = {7, 5, 8, 6};
  if(kpts_sframe.empty()) {
    return action_record.action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]*3] + kpts_sframe[kpts[1]*3] >= kpts_sframe[kpts[2]*3] + kpts_sframe[kpts[3]*3]) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  if (xy_ratio && action_record.mark) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.mark = false;
      action_record.latency = 0;
    }
  }
  else if(!xy_ratio && !(action_record.mark)) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.action_count += 1;
      action_record.mark = true;
      action_record.latency = 0;
    }
  }
  return action_record.action_count;
}

int check_deep_down(std::vector<float> &kpts_sframe) {
  std::vector<int> kpts = {9, 13, 10, 14};
//  std::vector<int> kpts = {11, 13, 12, 14};
  if(kpts_sframe.empty()) {
    return action_record.action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]*3] + kpts_sframe[kpts[1]*3] >= kpts_sframe[kpts[2]*3] + kpts_sframe[kpts[3]*3]) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  if (xy_ratio && action_record.mark) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.mark = false;
      action_record.latency = 0;
    }
  }
  else if(!xy_ratio && !(action_record.mark)) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.action_count += 1;
      action_record.mark = true;
      action_record.latency = 0;
    }
  }
  return action_record.action_count;
}

int check_deep_down2(float w, float h) {
  float down_thres = 2.5;
  float up_thres = 3;
  if (h/w > up_thres && action_record.mark) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.action_count += 1;
      action_record.mark = false;
      action_record.latency = 0;
    }
  }
  else if(h/w < down_thres && !(action_record.mark)) {
    action_record.latency += 1;
    if (action_record.latency == 4) {
      action_record.mark = true;
      action_record.latency = 0;
    }
  }
  return action_record.action_count;
}
