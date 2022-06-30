// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#include "Utils.h"
#include "paddle_api.h"
#include "Detector.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

struct RESULT_KEYPOINT {
  std::string class_name;
  int num_joints = -1;
  std::vector<float> keypoints;
};

class Detector_KeyPoint {
public:
  explicit Detector_KeyPoint(
      const std::string &modelDir, const std::string &labelPath,
      const int cpuThreadNum, const std::string &cpuPowerMode, int inputWidth,
      int inputHeight, const std::vector<float> &inputMean,
      const std::vector<float> &inputStd, float scoreThreshold);

  void Predict(const cv::Mat &rgbImage, std::vector<RESULT> *results,
               std::vector<RESULT_KEYPOINT> *results_kpts,
               double *preprocessTime, double *predictTime,
               double *postprocessTime, bool single);

  void CropImg(const cv::Mat &img, std::vector<cv::Mat> &crop_img,
               std::vector<RESULT> &results,
               std::vector<std::vector<float>> &center_bs,
               std::vector<std::vector<float>> &scale_bs,
               float expandratio = 0.2);
  void FindMaxRect(std::vector<RESULT> *results, std::vector<RESULT> &rect_buff, bool single);
  float get_threshold() { return scoreThreshold_; };

private:
  std::vector<cv::Scalar> GenerateColorMap(int numOfClasses);
  void Preprocess(std::vector<cv::Mat> &bs_images);
  void Postprocess(std::vector<RESULT_KEYPOINT> *results,
                   std::vector<std::vector<float>> &center_bs,
                   std::vector<std::vector<float>> &scale_bs);

private:
  int inputWidth_;
  int inputHeight_;
  bool use_dark = true;
  std::vector<float> inputMean_;
  std::vector<float> inputStd_;
  float scoreThreshold_;
  std::vector<cv::Scalar> colorMap_;
  std::shared_ptr<paddle::lite_api::PaddlePredictor> predictor_keypoint_;
};


class PoseSmooth {
public:
  explicit PoseSmooth(const int width,
                      const int height,
                      std::string filter_type = "OneEuro",
                      float alpha = 0.5,
                      float fc_d = 0.1,
                      float fc_min = 0.1,
                      float beta = 0.1,
                      float thres_mult = 0.3)
      : width(width),
        height(height),
        alpha(alpha),
        fc_d(fc_d),
        fc_min(fc_min),
        beta(beta),
        filter_type(filter_type),
        thres_mult(thres_mult){};

  // Run predictor
  RESULT_KEYPOINT smooth_process(RESULT_KEYPOINT* result);
  void PointSmooth(RESULT_KEYPOINT* result,
                   RESULT_KEYPOINT* keypoint_smoothed,
                   std::vector<float> thresholds,
                   int index);
  float OneEuroFilter(float x_cur, float x_pre, int loc);
  float smoothing_factor(float te, float fc);
  float ExpSmoothing(float x_cur, float x_pre, int loc = 0);

private:
  int width = 0;
  int height = 0;
  float alpha = 0.;
  float fc_d = 1.;
  float fc_min = 0.;
  float beta = 1.;
  float thres_mult = 1.;
  std::string filter_type = "OneEuro";
  std::vector<float> thresholds = {0.005,
                                   0.005,
                                   0.005,
                                   0.005,
                                   0.005,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01,
                                   0.01};
  RESULT_KEYPOINT x_prev_hat;
  RESULT_KEYPOINT dx_prev_hat;
};