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

std::vector<action_helper> action_recs(2);

void clear_action_count() {
  action_recs[0].action_count = 0;
  action_recs[1].action_count = 0;
  return;
}
float thres_conf = 0.3;

int get_action_count(int recid) {
  return action_recs[recid].action_count;
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

int check_lateral_raise(std::vector<float> &kpts_sframe, int recid) {
  std::vector<int> kpts = {5, 9, 6, 10};
  float down_thres=0.5;
  float up_thres=3.;
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  float xy_ratio;
  if (kpts_sframe[kpts[0]*3] > thres_conf && kpts_sframe[kpts[1]*3] > thres_conf) {
    xy_ratio = get_xyratio(kpts_sframe, kpts[0], kpts[1]);
  }
  else if (kpts_sframe[kpts[2]*3] > thres_conf && kpts_sframe[kpts[3]*3] > thres_conf) {
    xy_ratio = get_xyratio(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }
  if (xy_ratio < down_thres && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(xy_ratio > up_thres && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int check_stand_press(std::vector<float> &kpts_sframe, int recid) {
  std::vector<int> kpts = {7, 5, 8, 6};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]*3] > thres_conf && kpts_sframe[kpts[1]*3] > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else if(kpts_sframe[kpts[2]*3] > thres_conf && kpts_sframe[kpts[3]*3] > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(!xy_ratio && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int check_deep_down(std::vector<float> &kpts_sframe, int recid) {
  std::vector<int> kpts = {9, 13, 10, 14};
//  std::vector<int> kpts = {11, 13, 12, 14};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]*3] > thres_conf && kpts_sframe[kpts[1]*3] > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else if(kpts_sframe[kpts[2]*3] > thres_conf && kpts_sframe[kpts[3]*3] > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(!xy_ratio && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int check_deep_down2(std::vector<float> &kpts_sframe, float h, int recid) {
//  std::vector<int> kpts = {9, 13, 10, 14};
  std::vector<int> kpts = {11, 13, 12, 14};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  float down_thres=0.12;
  float up_thres=0.14;
  float xy_ratio;
  if (kpts_sframe[kpts[0]*3] > thres_conf && kpts_sframe[kpts[1]*3] > thres_conf) {
    int ydiff = std::max(-kpts_sframe[kpts[0]*3 + 2] + kpts_sframe[kpts[1]*3 + 2], 0.f);
    xy_ratio = ydiff / h;
  }
  else if(kpts_sframe[kpts[2]*3] > thres_conf && kpts_sframe[kpts[3]*3] > thres_conf) {
    int ydiff = std::max(-kpts_sframe[kpts[2]*3 + 2] + kpts_sframe[kpts[3]*3 + 2], 0.f);
    xy_ratio = ydiff / h;
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio<down_thres && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(xy_ratio>up_thres && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 2) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int single_action_check(std::vector<float> &results_kpts, float h, int actionid, int recid) {
  if (results_kpts.size()==0 || actionid>3) {
    return action_recs[recid].action_count;
  }
  if (actionid == 1) {
    return check_lateral_raise(results_kpts, recid);
  }
  else if (actionid == 2) {
    return check_stand_press(results_kpts, recid);
  }
  else {
    return check_deep_down2(results_kpts, h, recid);
  }
}

void double_action_check(std::vector<RESULT_KEYPOINT> &results_kpts, std::vector<RESULT> &results, int actionid, int imgw) {
  std::vector<float> left, right;
  if (results_kpts.size() == 0) {
    return;
  }
  float rh = 0;
  float lh = 0;
  float maxconf = -1;
  float maxid = -1;
  for (int i=0; i < results_kpts.size(); i++) {
    if (left.size()>0 || right.size()>0) {
      break;
    }
    for (int j=0; j < results_kpts[i].num_joints; j++) {
      if (results_kpts[i].keypoints[j*3] > 0.8) {
        if (results_kpts[i].keypoints[j*3 + 1] < imgw/2.) {
          left = results_kpts[i].keypoints;
          lh = results_kpts[i].height;
          if (results_kpts.size() > 1) {
            right = results_kpts[1-i].keypoints;
            rh = results_kpts[1-i].height;
          }
        }
        else {
          right = results_kpts[i].keypoints;
          rh = results_kpts[i].height;
          if (results_kpts.size() > 1) {
            left = results_kpts[1-i].keypoints;
            lh = results_kpts[1-i].height;
          }
        }
        break;
      }
      else if (results_kpts[i].keypoints[j*3] > maxconf) {
        maxconf = results_kpts[i].keypoints[j*3];
        maxid = i;
      }
    }
  }
  if (left.empty() && right.empty() && results_kpts[maxid].keypoints[maxconf*3 + 1] < imgw/2.) {
    left = results_kpts[maxid].keypoints;
    lh = results_kpts[maxid].height;
    if (results_kpts.size() > 1) {
      right = results_kpts[1-maxid].keypoints;
      rh = results_kpts[1-maxid].height;
    }
  }
  else if(left.empty() && right.empty()) {
    right = results_kpts[maxid].keypoints;
    rh = results_kpts[1-maxid].height;
    if (results_kpts.size() > 1) {
      left = results_kpts[1-maxid].keypoints;
      lh = results_kpts[1-maxid].height;
    }
  }
  if (!left.empty()) {
    single_action_check(left, lh, actionid, 0);
  }
  if (!right.empty()) {
    single_action_check(right, rh, actionid, 1);
  }
}
