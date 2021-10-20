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
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

struct RESULT {
  std::string class_name;
  cv::Scalar fill_color;
  float score;
  float x;
  float y;
  float w;
  float h;
};

class Detector {
public:
  explicit Detector(const std::string &modelDir, const std::string &labelPath,
                    const int cpuThreadNum, const std::string &cpuPowerMode,
                    int inputWidth, int inputHeight,
                    const std::vector<float> &inputMean,
                    const std::vector<float> &inputStd, float scoreThreshold);

  void Predict(const cv::Mat &rgbImage, std::vector<RESULT> *results,
               double *preprocessTime, double *predictTime,
               double *postprocessTime);

private:
  std::vector<std::string> LoadLabelList(const std::string &path);
  std::vector<cv::Scalar> GenerateColorMap(int numOfClasses);
  void Preprocess(const cv::Mat &rgbaImage);
  void Postprocess(std::vector<RESULT> *results);

private:
  int inputWidth_;
  int inputHeight_;
  std::vector<float> inputMean_;
  std::vector<float> inputStd_;
  float scoreThreshold_;
  std::vector<std::string> labelList_;
  std::vector<cv::Scalar> colorMap_;
  std::shared_ptr<paddle::lite_api::PaddlePredictor> predictor_;
};

struct RESULT_KEYPOINT {
  std::string class_name;
  int num_joints = 17;
  std::vector<float> keypoints;
};

class Detector_KeyPoint {
public:
  explicit Detector_KeyPoint(const std::string &modelDir, const std::string &labelPath,
                    const int cpuThreadNum, const std::string &cpuPowerMode,
                    int inputWidth, int inputHeight,
                    const std::vector<float> &inputMean,
                    const std::vector<float> &inputStd, float scoreThreshold);

  void Predict(const cv::Mat &rgbImage, std::vector<RESULT> *results, std::vector<RESULT_KEYPOINT> *results_kpts, 
               double *preprocessTime, double *predictTime,
               double *postprocessTime);

  void CropImg(const cv::Mat &img, std::vector<cv::Mat> &crop_img, std::vector<RESULT> &results, std::vector<std::vector<float>> &center_bs, std::vector<std::vector<float>> &scale_bs, float expandratio=0.2);
  RESULT FindMaxRect(std::vector<RESULT> *results);
  float get_threshold() {return scoreThreshold_;};

private:
  std::vector<cv::Scalar> GenerateColorMap(int numOfClasses);
  void Preprocess(std::vector<cv::Mat> &bs_images);
  void Postprocess(std::vector<RESULT_KEYPOINT> *results,
                                   std::vector<std::vector<float>>& center_bs,
                                   std::vector<std::vector<float>>& scale_bs);
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

class Pipeline {
public:
  Pipeline(const std::string &modelDir, const std::string &labelPath,
           const int cpuThreadNum, const std::string &cpuPowerMode,
           int inputWidth, int inputHeight, const std::vector<float> &inputMean,
           const std::vector<float> &inputStd, float scoreThreshold);

  bool Process(int inTextureId, int outTextureId, int textureWidth,
               int textureHeight, std::string savedImagePath);

private:
  // Read pixels from FBO texture to CV image
  void CreateRGBAImageFromGLFBOTexture(int textureWidth, int textureHeight,
                                       cv::Mat *rgbaImage,
                                       double *readGLFBOTime) {
    auto t = GetCurrentTime();
    rgbaImage->create(textureHeight, textureWidth, CV_8UC4);
    glReadPixels(0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgbaImage->data);
    *readGLFBOTime = GetElapsedTime(t);
    LOGD("Read from FBO texture costs %f ms", *readGLFBOTime);
  }

  // Write back to texture2D
  void WriteRGBAImageBackToGLTexture(const cv::Mat &rgbaImage, int textureId,
                                     double *writeGLTextureTime) {
    auto t = GetCurrentTime();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rgbaImage.cols, rgbaImage.rows,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgbaImage.data);
    *writeGLTextureTime = GetElapsedTime(t);
    LOGD("Write back to texture2D costs %f ms", *writeGLTextureTime);
  }

  // Visualize the results to origin image
  void VisualizeResults(const std::vector<RESULT> &results, cv::Mat *rgbaImage);
  void VisualizeKptsResults(const std::vector<RESULT> &results, const std::vector<RESULT_KEYPOINT> &results_kpts,
                                      cv::Mat *rgbaImage);
  // Visualize the status(performace data) to origin image
  void VisualizeStatus(double readGLFBOTime, double writeGLTextureTime,
                       double preprocessTime, double predictTime,
                       double postprocessTime, cv::Mat *rgbaImage);

private:
  std::shared_ptr<Detector> detector_;
  std::shared_ptr<Detector_KeyPoint> detector_keypoint_;
};
