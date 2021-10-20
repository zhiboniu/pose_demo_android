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

#include "Pipeline.h"
#include "postprocess.h"

Detector::Detector(const std::string &modelDir, const std::string &labelPath,
                   const int cpuThreadNum, const std::string &cpuPowerMode,
                   int inputWidth, int inputHeight,
                   const std::vector<float> &inputMean,
                   const std::vector<float> &inputStd, float scoreThreshold)
    : inputWidth_(inputWidth), inputHeight_(inputHeight), inputMean_(inputMean),
      inputStd_(inputStd), scoreThreshold_(scoreThreshold) {
  paddle::lite_api::MobileConfig config;
  config.set_model_from_file(modelDir + "/model.nb");
  config.set_threads(cpuThreadNum);
  config.set_power_mode(ParsePowerMode(cpuPowerMode));
  predictor_ =
      paddle::lite_api::CreatePaddlePredictor<paddle::lite_api::MobileConfig>(
          config);

  labelList_ = LoadLabelList(labelPath);
  colorMap_ = GenerateColorMap(labelList_.size());
}

std::vector<std::string> Detector::LoadLabelList(const std::string &labelPath) {
  std::ifstream file;
  std::vector<std::string> labels;
  file.open(labelPath);
  while (file) {
    std::string line;
    std::getline(file, line);
    labels.push_back(line);
  }
  file.clear();
  file.close();
  return labels;
}

std::vector<cv::Scalar> Detector::GenerateColorMap(int numOfClasses) {
  std::vector<cv::Scalar> colorMap = std::vector<cv::Scalar>(numOfClasses);
  for (int i = 0; i < numOfClasses; i++) {
    int j = 0;
    int label = i;
    int R = 0, G = 0, B = 0;
    while (label) {
      R |= (((label >> 0) & 1) << (7 - j));
      G |= (((label >> 1) & 1) << (7 - j));
      B |= (((label >> 2) & 1) << (7 - j));
      j++;
      label >>= 3;
    }
    colorMap[i] = cv::Scalar(R, G, B);
  }
  return colorMap;
}

void Detector::Preprocess(const cv::Mat &rgbaImage) {
  // Set the data of input image
  auto inputTensor = predictor_->GetInput(0);
  std::vector<int64_t> inputShape = {1, 3, inputHeight_, inputWidth_};
  inputTensor->Resize(inputShape);
  auto inputData = inputTensor->mutable_data<float>();
  cv::Mat resizedRGBAImage;
  cv::resize(rgbaImage, resizedRGBAImage, cv::Size(inputShape[3], inputShape[2]));
  cv::Mat resizedRGBImage;
  cv::cvtColor(resizedRGBAImage, resizedRGBImage, cv::COLOR_BGRA2RGB);
  resizedRGBImage.convertTo(resizedRGBImage, CV_32FC3, 1.0 / 255.0f);
  NHWC3ToNC3HW(reinterpret_cast<const float *>(resizedRGBImage.data), inputData,
               inputMean_.data(), inputStd_.data(), inputShape[3],
               inputShape[2]);
  // Set the size of input image
  auto sizeTensor = predictor_->GetInput(1);
  sizeTensor->Resize({1, 2});
  auto sizeData = sizeTensor->mutable_data<int32_t>();
  sizeData[0] = inputShape[3];
  sizeData[1] = inputShape[2];
}

void Detector::Postprocess(std::vector<RESULT> *results) {
  auto outputTensor = predictor_->GetOutput(0);
  auto outputData = outputTensor->data<float>();
  auto outputShape = outputTensor->shape();
  int outputSize = ShapeProduction(outputShape);
  for (int i = 0; i < outputSize; i += 6) {
    // Class id
    auto class_id = static_cast<int>(round(outputData[i]));
    // Confidence score
    auto score = outputData[i + 1];
    if (class_id != 0)
      continue;
    if (score < scoreThreshold_)
      continue;
    RESULT object;
    object.class_name = class_id >= 0 && class_id < labelList_.size()
                            ? labelList_[class_id]
                            : "Unknow";
    object.fill_color = class_id >= 0 && class_id < colorMap_.size()
                            ? colorMap_[class_id]
                            : cv::Scalar(0, 0, 0);
    object.score = score;
    object.x = outputData[i + 2] / inputWidth_;
    object.y = outputData[i + 3] / inputHeight_;
    object.w = (outputData[i + 4] - outputData[i + 2] + 1) / inputWidth_;
    object.h = (outputData[i + 5] - outputData[i + 3] + 1) / inputHeight_;
    results->push_back(object);
  }
}

void Detector::Predict(const cv::Mat &rgbaImage, std::vector<RESULT> *results,
                       double *preprocessTime, double *predictTime,
                       double *postprocessTime) {
  auto t = GetCurrentTime();

  t = GetCurrentTime();
  Preprocess(rgbaImage);
  *preprocessTime = GetElapsedTime(t);
  LOGD("Detector postprocess costs %f ms", *preprocessTime);

  t = GetCurrentTime();
  predictor_->Run();
  *predictTime = GetElapsedTime(t);
  LOGD("Detector predict costs %f ms", *predictTime);

  t = GetCurrentTime();
  Postprocess(results);
  *postprocessTime = GetElapsedTime(t);
  LOGD("Detector postprocess costs %f ms", *postprocessTime);
}

//KeyPoint Detector

Detector_KeyPoint::Detector_KeyPoint(const std::string &modelDir, const std::string &labelPath,
                   const int cpuThreadNum, const std::string &cpuPowerMode,
                   int inputWidth, int inputHeight,
                   const std::vector<float> &inputMean,
                   const std::vector<float> &inputStd, float scoreThreshold)
    : inputWidth_(inputWidth), inputHeight_(inputHeight), inputMean_(inputMean),
      inputStd_(inputStd), scoreThreshold_(scoreThreshold) {
  paddle::lite_api::MobileConfig config;
  config.set_model_from_file(modelDir + "/model_keypoint_nh18.nb");
  config.set_threads(cpuThreadNum);
  config.set_power_mode(ParsePowerMode(cpuPowerMode));
  predictor_keypoint_ =
      paddle::lite_api::CreatePaddlePredictor<paddle::lite_api::MobileConfig>(
          config);

  colorMap_ = GenerateColorMap(20); //coco keypoint number is 17
}

std::vector<cv::Scalar> Detector_KeyPoint::GenerateColorMap(int numOfClasses) {
  std::vector<cv::Scalar> colorMap = std::vector<cv::Scalar>(numOfClasses);
  for (int i = 0; i < numOfClasses; i++) {
    int j = 0;
    int label = i;
    int R = 0, G = 0, B = 0;
    while (label) {
      R |= (((label >> 0) & 1) << (7 - j));
      G |= (((label >> 1) & 1) << (7 - j));
      B |= (((label >> 2) & 1) << (7 - j));
      j++;
      label >>= 3;
    }
    colorMap[i] = cv::Scalar(R, G, B);
  }
  return colorMap;
}

void Detector_KeyPoint::Preprocess(std::vector<cv::Mat> &bs_images) {
  // Set the data of input image
  int batchsize = bs_images.size();
  auto inputTensor = predictor_keypoint_->GetInput(0);
  std::vector<int64_t> inputShape = {batchsize, 3, inputHeight_, inputWidth_};
  inputTensor->Resize(inputShape);
  auto inputData = inputTensor->mutable_data<float>();
  for (int i=0; i<batchsize; i++) {
    cv::Mat resizedRGBAImage;
    cv::resize(bs_images[i], resizedRGBAImage, cv::Size(inputShape[3], inputShape[2]));
    cv::Mat resizedRGBImage;
    cv::cvtColor(resizedRGBAImage, resizedRGBImage, cv::COLOR_BGRA2RGB);
    resizedRGBImage.convertTo(resizedRGBImage, CV_32FC3, 1.0 / 255.0f);
    auto dst_inptr = inputData + i*(3*inputHeight_*inputWidth_);
    NHWC3ToNC3HW(reinterpret_cast<const float *>(resizedRGBImage.data), dst_inptr,
                inputMean_.data(), inputStd_.data(), inputShape[3],
                inputShape[2]);
  }
  // Set the size of input image
//  auto sizeTensor = predictor_keypoint_->GetInput(1);
//  sizeTensor->Resize({1, 2});
//  auto sizeData = sizeTensor->mutable_data<int32_t>();
//  sizeData[0] = inputShape[3];
//  sizeData[1] = inputShape[2];
}

void Detector_KeyPoint::Postprocess(std::vector<RESULT_KEYPOINT> *results,
                                   std::vector<std::vector<float>>& center_bs,
                                   std::vector<std::vector<float>>& scale_bs) {
  auto outputTensor = predictor_keypoint_->GetOutput(0);
  auto outputdata = outputTensor->data<float>();
  auto output_shape = outputTensor->shape();
  int outputSize = ShapeProduction(output_shape);
  auto outidx_Tensor = predictor_keypoint_->GetOutput(1);
  auto idxoutdata = outidx_Tensor->data<int64_t>();
  auto idx_shape = outidx_Tensor->shape();
  int idxoutSize = ShapeProduction(idx_shape);

  std::vector<float> output;
  std::vector<int64_t> idxout;
  output.resize(outputSize);
  std::copy_n(outputdata, outputSize, output.data());
  idxout.resize(idxoutSize);
  std::copy_n(idxoutdata, idxoutSize, idxout.data());

  std::vector<float> preds(output_shape[1] * 3, 0);

  for (int batchid = 0; batchid < output_shape[0]; batchid++) {
    get_final_preds(output,
                    output_shape,
                    idxout,
                    idx_shape,
                    center_bs[batchid],
                    scale_bs[batchid],
                    preds,
                    batchid,
                    this->use_dark);
    RESULT_KEYPOINT result_item;
    result_item.num_joints = output_shape[1];
    result_item.keypoints.clear();
    for (int i = 0; i < output_shape[1]; i++) {
      result_item.keypoints.emplace_back(preds[i * 3]);
      result_item.keypoints.emplace_back(preds[i * 3 + 1]);
      result_item.keypoints.emplace_back(preds[i * 3 + 2]);
    }
    results->push_back(result_item);
  }
}

void Detector_KeyPoint::Predict(const cv::Mat &rgbaImage, std::vector<RESULT> *results, std::vector<RESULT_KEYPOINT> *results_kpts, 
                       double *preprocessTime, double *predictTime,
                       double *postprocessTime){
  if(results->empty())
    return;
  auto t = GetCurrentTime();
  std::vector<std::vector<float>> center_bs;
  std::vector<std::vector<float>> scale_bs;
  std::vector<cv::Mat> cropimgs;
  RESULT srect = FindMaxRect(results);
  std::vector<RESULT> sresult = {srect};
  CropImg(rgbaImage, cropimgs, *results, center_bs, scale_bs);
  t = GetCurrentTime();
  Preprocess(cropimgs);
  *preprocessTime = GetElapsedTime(t);
  LOGD("Detector_KeyPoint postprocess costs %f ms", *preprocessTime);

  t = GetCurrentTime();
  predictor_keypoint_->Run();
  *predictTime = GetElapsedTime(t);
  LOGD("Detector_KeyPoint predict costs %f ms", *predictTime);

  t = GetCurrentTime();
  Postprocess(results_kpts, center_bs, scale_bs);
  *postprocessTime = GetElapsedTime(t);
  LOGD("Detector_KeyPoint postprocess costs %f ms", *postprocessTime);
}

RESULT Detector_KeyPoint::FindMaxRect(std::vector<RESULT> *results) {
  int maxid = 0;
  for(int i=0; i<results->size();i++){
    if ((*results)[i].h + (*results)[i].w > (*results)[maxid].h + (*results)[maxid].w){
      maxid = i;
    }
  }

  (*results)[maxid].fill_color = colorMap_[1];

  return (*results)[maxid];
}

void Detector_KeyPoint::CropImg(const cv::Mat &img, std::vector<cv::Mat> &crop_img, std::vector<RESULT> &results, std::vector<std::vector<float>> &center_bs, std::vector<std::vector<float>> &scale_bs, float expandratio) {
  int w = img.cols;
  int h = img.rows;
  for (int i=0; i<std::min(int(results.size()), 4); i++) {
    auto area = results[i];
    int crop_x1 = std::max(0, static_cast<int>(area.x * w));
    int crop_y1 = std::max(0, static_cast<int>(area.y * h));
    int crop_x2 = std::min(img.cols -1, static_cast<int>((area.x + area.w)*w));
    int crop_y2 = std::min(img.rows - 1, static_cast<int>((area.y + area.h)*h));
    int center_x = (crop_x1 + crop_x2)/2.;
    int center_y = (crop_y1 + crop_y2)/2.;
    int half_h = (crop_y2 - crop_y1)/2.;
    int half_w = (crop_x2 - crop_x1)/2.;

    //adjust h or w to keep image ratio, expand the shorter edge
    if (half_h*3 > half_w*4){
      half_w = static_cast<int>(half_h*0.75);
    }
    else{
      half_h = static_cast<int>(half_w*4/3);
    }

    int roi_x1 = center_x - static_cast<int>(half_w*(1+expandratio));
    int roi_y1 = center_y - static_cast<int>(half_h*(1+expandratio));
    int roi_x2 = center_x + static_cast<int>(half_w*(1+expandratio));
    int roi_y2 = center_y + static_cast<int>(half_h*(1+expandratio));
    crop_x1 = std::max(0, roi_x1);
    crop_y1 = std::max(0, roi_y1);
    crop_x2 = std::min(img.cols -1, roi_x2);
    crop_y2 = std::min(img.rows - 1, roi_y2);
    cv::Mat src_img = img(cv::Range(crop_y1, crop_y2+1), cv::Range(crop_x1, crop_x2 + 1));

    cv::Mat dst_img;
    cv::copyMakeBorder(src_img, dst_img, std::max(0, -roi_y1), std::max(0, roi_y2-img.rows +1), std::max(0, -roi_x1), std::max(0, roi_x2-img.cols +1), cv::BorderTypes::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    
    crop_img.emplace_back(dst_img);

    std::vector<float> center, scale;
    center.emplace_back(center_x);
    center.emplace_back(center_y);
    scale.emplace_back((half_w*2)*(1+expandratio));
    scale.emplace_back((half_h*2)*(1+expandratio));

    center_bs.emplace_back(center);
    scale_bs.emplace_back(scale);
  }
  

}

//Pipeline

Pipeline::Pipeline(const std::string &modelDir, const std::string &labelPath,
                   const int cpuThreadNum, const std::string &cpuPowerMode,
                   int inputWidth, int inputHeight,
                   const std::vector<float> &inputMean,
                   const std::vector<float> &inputStd, float scoreThreshold) {
  detector_.reset(new Detector(modelDir, labelPath, cpuThreadNum, cpuPowerMode,
                               inputWidth, inputHeight, inputMean, inputStd,
                               scoreThreshold));
  detector_keypoint_.reset(new Detector_KeyPoint(modelDir, labelPath, cpuThreadNum, cpuPowerMode,
                               192, 256, inputMean, inputStd,
                               0.5));
}

void Pipeline::VisualizeResults(const std::vector<RESULT> &results,
                                cv::Mat *rgbaImage) {
  int w = rgbaImage->cols;
  int h = rgbaImage->rows;
  for (int i = 0; i < results.size(); i++) {
    RESULT object = results[i];
    cv::Rect boundingBox =
        cv::Rect(object.x * w, object.y * h, object.w * w, object.h * h) &
        cv::Rect(0, 0, w - 1, h - 1);
    // Configure text size
    std::string text = object.class_name + " ";
    text += std::to_string(static_cast<int>(object.score * 100)) + "%";
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    double fontScale = 1.5f;
    float fontThickness = 1.0f;
    cv::Size textSize =
        cv::getTextSize(text, fontFace, fontScale, fontThickness, nullptr);
    // Draw roi object, text, and background
    cv::rectangle(*rgbaImage, boundingBox, object.fill_color, 2);
    cv::rectangle(*rgbaImage,
                  cv::Point2d(boundingBox.x,
                              boundingBox.y - round(textSize.height * 1.25f)),
                  cv::Point2d(boundingBox.x + boundingBox.width, boundingBox.y),
                  object.fill_color, -1);
    cv::putText(*rgbaImage, text, cv::Point2d(boundingBox.x, boundingBox.y),
                fontFace, fontScale, cv::Scalar(255, 255, 255), fontThickness);
  }
}


void Pipeline::VisualizeKptsResults(const std::vector<RESULT> &results, const std::vector<RESULT_KEYPOINT> &results_kpts,
                                    cv::Mat *rgbaImage) {
  int w = rgbaImage->cols;
  int h = rgbaImage->rows;
  for (int i = 0; i < results.size(); i++) {
    RESULT object = results[i];
    cv::Rect boundingBox =
        cv::Rect(object.x * w, object.y * h, object.w * w, object.h * h) &
        cv::Rect(0, 0, w - 1, h - 1);
    // Configure text size
    std::string text = object.class_name + " ";
    text += std::to_string(static_cast<int>(object.score * 100)) + "%";
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    double fontScale = 1.5f;
    float fontThickness = 1.0f;
    cv::Size textSize =
        cv::getTextSize(text, fontFace, fontScale, fontThickness, nullptr);
    // Draw roi object, text, and background
    cv::rectangle(*rgbaImage, boundingBox, object.fill_color, 2);
    cv::rectangle(*rgbaImage,
                  cv::Point2d(boundingBox.x,
                              boundingBox.y - round(textSize.height * 1.25f)),
                  cv::Point2d(boundingBox.x + boundingBox.width, boundingBox.y),
                  object.fill_color, -1);
    cv::putText(*rgbaImage, text, cv::Point2d(boundingBox.x, boundingBox.y),
                fontFace, fontScale, cv::Scalar(255, 255, 255), fontThickness);
  }

  const int edge[][2] = {{0, 1},
                         {0, 2},
                         {1, 3},
                         {2, 4},
                         {3, 5},
                         {4, 6},
                         {5, 7},
                         {6, 8},
                         {7, 9},
                         {8, 10},
                         {5, 11},
                         {6, 12},
                         {11, 13},
                         {12, 14},
                         {13, 15},
                         {14, 16},
                         {11, 12}};
  float kpts_threshold = detector_keypoint_->get_threshold();
  for (int batchid = 0; batchid < results_kpts.size(); batchid++) {
    for (int i = 0; i < results_kpts[batchid].num_joints; i++) {
      if (results_kpts[batchid].keypoints[i * 3] > kpts_threshold) {
        int x_coord = int(results_kpts[batchid].keypoints[i * 3 + 1]);
        int y_coord = int(results_kpts[batchid].keypoints[i * 3 + 2]);
        cv::circle(*rgbaImage,
                   cv::Point2d(x_coord, y_coord),
                   2,
                   cv::Scalar(0, 255, 255),
                   2);
      }
    }
    for (int i = 0; i < results_kpts[batchid].num_joints; i++) {
      if(results_kpts[batchid].keypoints[edge[i][0] * 3] < kpts_threshold || results_kpts[batchid].keypoints[edge[i][1] * 3] < kpts_threshold)
        continue;
      int x_start = int(results_kpts[batchid].keypoints[edge[i][0] * 3 + 1]);
      int y_start = int(results_kpts[batchid].keypoints[edge[i][0] * 3 + 2]);
      int x_end = int(results_kpts[batchid].keypoints[edge[i][1] * 3 + 1]);
      int y_end = int(results_kpts[batchid].keypoints[edge[i][1] * 3 + 2]);
      cv::line(*rgbaImage,
               cv::Point2d(x_start, y_start),
               cv::Point2d(x_end, y_end),
               cv::Scalar(0, 255, 255),
               2);
    }
  }
}

void Pipeline::VisualizeStatus(double readGLFBOTime, double writeGLTextureTime,
                               double preprocessTime, double predictTime,
                               double postprocessTime, cv::Mat *rgbaImage) {
  char text[255];
  cv::Scalar fontColor = cv::Scalar(255, 0, 0);
  int fontFace = cv::FONT_HERSHEY_PLAIN;
  double fontScale = 2.f;
  float fontThickness = 2;
  sprintf(text, "Read GLFBO time: %.1f ms", readGLFBOTime);
  cv::Size textSize =
      cv::getTextSize(text, fontFace, fontScale, fontThickness, nullptr);
  textSize.height *= 1.25f;
  cv::Point2d offset(10, textSize.height + 15);
  cv::putText(*rgbaImage, text, offset, fontFace, fontScale, fontColor,
              fontThickness);
  sprintf(text, "Write GLTexture time: %.1f ms", writeGLTextureTime);
  offset.y += textSize.height;
  cv::putText(*rgbaImage, text, offset, fontFace, fontScale, fontColor,
              fontThickness);
  sprintf(text, "Preprocess time: %.1f ms", preprocessTime);
  offset.y += textSize.height;
  cv::putText(*rgbaImage, text, offset, fontFace, fontScale, fontColor,
              fontThickness);
  sprintf(text, "Predict time: %.1f ms", predictTime);
  offset.y += textSize.height;
  cv::putText(*rgbaImage, text, offset, fontFace, fontScale, fontColor,
              fontThickness);
  sprintf(text, "Postprocess time: %.1f ms", postprocessTime);
  offset.y += textSize.height;
  cv::putText(*rgbaImage, text, offset, fontFace, fontScale, fontColor,
              fontThickness);
}

static std::vector<RESULT> results;
static int idx = 0;
bool Pipeline::Process(int inTexureId, int outTextureId, int textureWidth,
                       int textureHeight, std::string savedImagePath) {
  static double readGLFBOTime = 0, writeGLTextureTime = 0;
  double preprocessTime = 0, predictTime = 0, postprocessTime = 0;
  double preprocessTime_kpts = 0, predictTime_kpts = 0, postprocessTime_kpts = 0;

  // Read pixels from FBO texture to CV image
  cv::Mat rgbaImage;
  CreateRGBAImageFromGLFBOTexture(textureWidth, textureHeight, &rgbaImage,
                                  &readGLFBOTime);

  // Feed the image, run inference and parse the results
  if (idx%2 == 0 or results.empty()){
    idx = 0;
    results.clear();
    detector_->Predict(rgbaImage, &results, &preprocessTime, &predictTime,
                       &postprocessTime);
  }
  idx++;

  //add keypoint pipeline
  std::vector<RESULT_KEYPOINT> results_kpts;
  detector_keypoint_->Predict(rgbaImage, &results, &results_kpts, &preprocessTime_kpts, &predictTime_kpts, &postprocessTime_kpts);

  // Visualize the objects to the origin image
//  VisualizeResults(results, &rgbaImage);
  VisualizeKptsResults(results, results_kpts, &rgbaImage);

  // Visualize the status(performance data) to the origin image
  VisualizeStatus(readGLFBOTime, writeGLTextureTime, preprocessTime_kpts,
                  predictTime_kpts, postprocessTime_kpts, &rgbaImage);

  // Dump modified image if savedImagePath is set
  if (!savedImagePath.empty()) {
    cv::Mat bgrImage;
    cv::cvtColor(rgbaImage, bgrImage, cv::COLOR_RGBA2BGR);
    imwrite(savedImagePath, bgrImage);
  }

  // Write back to texture2D
  WriteRGBAImageBackToGLTexture(rgbaImage, outTextureId, &writeGLTextureTime);
  return true;
}
