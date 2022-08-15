// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include "Detector.h"

Detector::Detector(const std::string &modelDir, const std::string &labelPath,
                   const int cpuThreadNum, const std::string &cpuPowerMode,
                   int inputWidth, int inputHeight,
                   const std::vector<float> &inputMean,
                   const std::vector<float> &inputStd, float scoreThreshold)
    : inputWidth_(inputWidth), inputHeight_(inputHeight), inputMean_(inputMean),
      inputStd_(inputStd), scoreThreshold_(scoreThreshold) {
  paddle::lite_api::MobileConfig config;
  config.set_model_from_file(modelDir + "/model_det.nb");
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
  std::vector<int64_t> inputShape = {1, 3, inputHeight_, inputWidth_};
  scale_height_ = static_cast<float>(inputHeight_) / static_cast<float>(rgbaImage.rows);
  scale_width_ = static_cast<float>(inputWidth_) / static_cast<float>(rgbaImage.cols);
  auto input_names = predictor_->GetInputNames();
  for (const auto& tensor_name : input_names) {
    if (tensor_name == "image") {
      auto inputTensor = predictor_->GetInputByName(tensor_name);
      // Set the data of input image
//      auto inputTensor = predictor_->GetInput(0);
      inputTensor->Resize(inputShape);
      auto inputData = inputTensor->mutable_data<float>();
      cv::Mat resizedRGBAImage;
      cv::resize(rgbaImage, resizedRGBAImage,
                 cv::Size(inputShape[3], inputShape[2]));
      cv::Mat resizedRGBImage;
      cv::cvtColor(resizedRGBAImage, resizedRGBImage, cv::COLOR_BGRA2RGB);
      resizedRGBImage.convertTo(resizedRGBImage, CV_32FC3, 1.0f);
      Permute(&resizedRGBImage, inputData);
//      NHWC3ToNC3HW(reinterpret_cast<const float *>(resizedRGBImage.data), inputData,
//                   inputMean_.data(), inputStd_.data(), inputShape[3],
//                   inputShape[2]);
    } else if (tensor_name == "im_shape") {
      // Set the size of input image
      auto sizeTensor = predictor_->GetInputByName(tensor_name);
      sizeTensor->Resize({1, 2});
      auto sizeData = sizeTensor->mutable_data<int32_t>();
      sizeData[0] = inputShape[3];
      sizeData[1] = inputShape[2];
    }
    else if (tensor_name == "scale_factor") {
      auto scaleTensor = predictor_->GetInputByName(tensor_name);
      scaleTensor->Resize({1, 2});
      auto* scale_data = scaleTensor->mutable_data<float>();
      scale_data[0] = scale_height_;
      scale_data[1] = scale_width_;
    }
  }
}

void Detector::Postprocess(std::vector<RESULT> *results) {
  // TODO: Unified model output.
  int num_class = 80;
  int reg_max = 7;
  auto output_names = predictor_->GetOutputNames();
  if (arch_ == "PicoDet") {
    std::vector<const float *> output_data_list_;
    output_data_list_.clear();
    for (int i = 0; i < output_names.size(); i++) {
      auto output_tensor = predictor_->GetTensor(output_names[i]);
      const float *outptr = output_tensor->data<float>();
      std::vector<int64_t> output_shape = output_tensor->shape();
      if (i == 0) {
        num_class = output_shape[2];
      }
      if (i == fpn_stride_.size()) {
        reg_max = output_shape[2] / 4 - 1;
      }
      output_data_list_.push_back(outptr);
    }

    //
    std::vector<float> im_shape_ = {static_cast<float>(inputHeight_), static_cast<float>(inputWidth_)};
    std::vector<float> scale_factor_ = {scale_height_, scale_width_};
    PicoDetPostProcess(results, output_data_list_, fpn_stride_,
                       im_shape_, scale_factor_,
                       score_threshold, nms_threshold, num_class, reg_max);
  } else {
    auto output_tensor = predictor_->GetTensor(output_names[0]);
    auto output_shape = output_tensor->shape();
    auto out_bbox_num = predictor_->GetTensor(output_names[1]);
    auto out_bbox_num_shape = out_bbox_num->shape();
    // Calculate output length
    int output_size = 1;
    for (int j = 0; j < output_shape.size(); ++j) {
      output_size *= output_shape[j];
    }

    output_data_.resize(output_size);
    std::copy_n(
        output_tensor->mutable_data<float>(), output_size, output_data_.data());

    int out_bbox_num_size = 1;
    for (int j = 0; j < out_bbox_num_shape.size(); ++j) {
      out_bbox_num_size *= out_bbox_num_shape[j];
    }
    out_bbox_num_data_.resize(out_bbox_num_size);
    std::copy_n(out_bbox_num->mutable_data<int>(),
                out_bbox_num_size,
                out_bbox_num_data_.data());
    NormalPostprocess(1, results, out_bbox_num_data_);
  }
}

void Detector::Predict(const cv::Mat &rgbaImage, std::vector<RESULT> *results,
                       double *preprocessTime, double *predictTime,
                       double *postprocessTime) {
  auto t = GetCurrentTime();

  t = GetCurrentTime();
  Preprocess(rgbaImage);
  *preprocessTime = GetElapsedTime(t);
  LOGD("Detector preprocess costs %f ms", *preprocessTime);

  t = GetCurrentTime();
  predictor_->Run();
  *predictTime = GetElapsedTime(t);
  LOGD("Detector predict costs %f ms", *predictTime);

  t = GetCurrentTime();
  Postprocess(results);
  *postprocessTime = GetElapsedTime(t);
  LOGD("Detector postprocess costs %f ms", *postprocessTime);
}

float fast_exp(float x) {
  union {
    uint32_t i;
    float f;
  } v{};
  v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
  return v.f;
}

template <typename _Tp>
int activation_function_softmax(const _Tp *src, _Tp *dst, int length) {
  const _Tp alpha = *std::max_element(src, src + length);
  _Tp denominator{0};

  for (int i = 0; i < length; ++i) {
    dst[i] = fast_exp(src[i] - alpha);
    denominator += dst[i];
  }

  for (int i = 0; i < length; ++i) {
    dst[i] /= denominator;
  }

  return 0;
}

// PicoDet decode
RESULT Detector::disPred2Bbox(const float *&dfl_det, int label, float score,
                              int x, int y, int stride,
                              std::vector<float> im_shape, int reg_max) {
  float ct_x = (x + 0.5) * stride;
  float ct_y = (y + 0.5) * stride;
  std::vector<float> dis_pred;
  dis_pred.resize(4);
  for (int i = 0; i < 4; i++) {
    float dis = 0;
    float *dis_after_sm = new float[reg_max + 1];
    activation_function_softmax(dfl_det + i * (reg_max + 1), dis_after_sm,
                                reg_max + 1);
    for (int j = 0; j < reg_max + 1; j++) {
      dis += j * dis_after_sm[j];
    }
    dis *= stride;
    dis_pred[i] = dis;
    delete[] dis_after_sm;
  }
  int xmin = (int)(std::max)(ct_x - dis_pred[0], .0f);
  int ymin = (int)(std::max)(ct_y - dis_pred[1], .0f);
  int xmax = (int)(std::min)(ct_x + dis_pred[2], (float)im_shape[0]);
  int ymax = (int)(std::min)(ct_y + dis_pred[3], (float)im_shape[1]);

  RESULT result_item;
  result_item.x = xmin;
  result_item.y = ymin;
  result_item.w = xmax - xmin;
  result_item.h = ymax - ymin;
  result_item.class_id = label;
  result_item.score = score;
  result_item.class_name =
      label >= 0 && label < labelList_.size() ? labelList_[label] : "Unknow";
  result_item.fill_color = label >= 0 && label < colorMap_.size()
                               ? colorMap_[label]
                               : cv::Scalar(0, 0, 0);

  return result_item;
}

void Detector::PicoDetPostProcess(std::vector<RESULT> *results,
                                  std::vector<const float *> outs,
                                  std::vector<int> fpn_stride,
                                  std::vector<float> im_shape,
                                  std::vector<float> scale_factor,
                                  float score_threshold, float nms_threshold,
                                  int num_class, int reg_max) {
  std::vector<std::vector<RESULT>> bbox_results;
  bbox_results.resize(num_class);
  int in_h = im_shape[0], in_w = im_shape[1];
  for (int i = 0; i < fpn_stride.size(); ++i) {
    int feature_h = in_h / fpn_stride[i];
    int feature_w = in_w / fpn_stride[i];
    for (int idx = 0; idx < feature_h * feature_w; idx++) {
      const float *scores = outs[i] + (idx * num_class);

      int row = idx / feature_w;
      int col = idx % feature_w;
      float score = 0;
      int cur_label = 0;
      for (int label = 0; label < num_class; label++) {
        if (scores[label] > score) {
          score = scores[label];
          cur_label = label;
        }
      }
      if (score > score_threshold && cur_label == 0) {
        const float *bbox_pred =
            outs[i + fpn_stride.size()] + (idx * 4 * (reg_max + 1));
        bbox_results[cur_label].push_back(
            disPred2Bbox(bbox_pred, cur_label, score, col, row, fpn_stride[i],
                         im_shape, reg_max));
      }
    }
  }
  for (int i = 0; i < (int)bbox_results.size(); i++) {
    nms(bbox_results[i], nms_threshold);

    for (auto box : bbox_results[i]) {
      box.x = box.x / scale_factor[1];
      box.w = box.w / scale_factor[1];
      box.y = box.y / scale_factor[0];
      box.h = box.h / scale_factor[0];
      results->push_back(box);
    }
  }
}

void Detector::NormalPostprocess(const int batchsize,
                       std::vector<RESULT>* result,
                       std::vector<int> bbox_num) {
  result->clear();
  int start_idx = 0;
  for (int im_id = 0; im_id < batchsize; im_id++) {
    int rh = 1;
    int rw = 1;
    for (int j = start_idx; j < start_idx + bbox_num[im_id]; j++) {
      // Class id
      int class_id = static_cast<int>(round(output_data_[0 + j * 6]));
      // Confidence score
      float score = output_data_[1 + j * 6];
      if (score < score_threshold) {
        continue;
      }

      int xmin = (output_data_[2 + j * 6] * rw);
      int ymin = (output_data_[3 + j * 6] * rh);
      int xmax = (output_data_[4 + j * 6] * rw);
      int ymax = (output_data_[5 + j * 6] * rh);
      int wd = xmax - xmin;
      int hd = ymax - ymin;

      RESULT result_item;
      result_item.x = xmin;
      result_item.y = ymin;
      result_item.h = hd;
      result_item.w = wd;
      result_item.class_id = class_id;
      result_item.score = score;
      result->push_back(result_item);
    }
    start_idx += bbox_num[im_id];
  }
}

void nms(std::vector<RESULT> &input_boxes, float nms_threshold) {
  std::sort(input_boxes.begin(), input_boxes.end(),
            [](RESULT a, RESULT b) { return a.score > b.score; });
  std::vector<float> vArea(input_boxes.size());
  for (int i = 0; i < int(input_boxes.size()); ++i) {
    vArea[i] = input_boxes.at(i).w * input_boxes.at(i).h;
  }
  for (int i = 0; i < int(input_boxes.size()); ++i) {
    for (int j = i + 1; j < int(input_boxes.size());) {
      float xx1 = (std::max)(input_boxes[i].x, input_boxes[j].x);
      float yy1 = (std::max)(input_boxes[i].y, input_boxes[j].y);
      float xx2 = (std::min)(input_boxes[i].x+input_boxes[i].w, input_boxes[j].x+input_boxes[j].w);
      float yy2 = (std::min)(input_boxes[i].y+input_boxes[i].h, input_boxes[j].y+input_boxes[j].h);
      float w = (std::max)(float(0), xx2 - xx1 + 1);
      float h = (std::max)(float(0), yy2 - yy1 + 1);
      float inter = w * h;
      float ovr = inter / (vArea[i] + vArea[j] - inter);
      if (ovr >= nms_threshold) {
        input_boxes.erase(input_boxes.begin() + j);
        vArea.erase(vArea.begin() + j);
      } else {
        j++;
      }
    }
  }
}
