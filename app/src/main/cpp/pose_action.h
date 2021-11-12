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

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "Detector_Kpts.h"

struct action_helper {
  bool mark = false;
  int action_count = 0;
  int latency = 0;
};

int get_action_count(int recid);
float get_xyratio(std::vector<float> &kpts_sframe, int index_x, int index_y);
bool get_xyhigher(std::vector<float> &kpts_sframe, int index_x, int index_y);
int check_lateral_raise(std::vector<float> &kpts_sframe, int recid);
int check_stand_press(std::vector<float> &kpts_sframe, int recid);
int check_deep_down(std::vector<float> &kpts_sframe, int recid);
int check_deep_down2(std::vector<float> &kpts_sframe, float h, int recid);
int single_action_check(std::vector<float> &results_kpts, float h, int actionid, int recid=0);
void double_action_check(std::vector<RESULT_KEYPOINT> &results_kpts, std::vector<RESULT> &results, int actionid);
