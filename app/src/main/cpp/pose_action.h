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

int get_action_count();
float get_xyratio(std::vector<float> &kpts_sframe, int index_x, int index_y);
bool get_xyhigher(std::vector<float> &kpts_sframe, int index_x, int index_y);
int check_lateral_raise(std::vector<float> &kpts_sframe);
int check_stand_press(std::vector<float> &kpts_sframe);
int check_deep_down(std::vector<float> &kpts_sframe);
int check_deep_down2(float w, float h);
