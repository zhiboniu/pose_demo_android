# pose_demo_android
pose demo on android mobile based on PaddleDetection

本工程Android部分基于[Paddle-Lite-Demo](https://github.com/PaddlePaddle/Paddle-Lite-Demo)修改。

算法模型基于[PaddleDetection](https://github.com/PaddlePaddle/PaddleDetection)的[PP-TinyPose](https://github.com/PaddlePaddle/PaddleDetection/tree/develop/configs/keypoint/tiny_pose).

如欲获取更多详情，请点击链接至相应repo中查看。



## APP安装体验
* Andoird APP[下载体验](https://bj.bcebos.com/v1/paddledet/deploy/paddlelite/PP-TinyPose_v3.apk)
* 安装二维码：

<div align="left">
  <img src="./pictures/PP-TinyPose_download_v3.png" width='200'/>
</div>


安装问题解决：

如果华为手机安装时提示`发现风险项（该应用为诈骗应用，请勿安装）`，该问题因华为手机有对外部应用的限制，可以按以下步骤解决：

1. 控制中关闭纯净模式（如果有，一般在下拉栏）。

2. 在设置-安全设置中，打开`外部来源应用下载`。关闭`外部来源应用检查`。

如果找不到可以百度搜索查找详细信息。

## 下面部分为本工程使用介绍

## 要求

* Android
    * Android Studio 4.2；
    * adb调试工具；
    * Android手机或开发版；


## 安装
$ git clone https://github.com/zhiboniu/pose_demo_android

* Android
    * 打开Android Studio，在"Welcome to Android Studio"窗口点击"Open an existing Android Studio project"，在弹出的路径选择窗口中进入"image_classification_demo"目录，然后点击右下角的"Open"按钮即可导入工程
    * 通过USB连接Android手机或开发板；
    * 载入工程后，点击菜单栏的Run->Run 'App'按钮，在弹出的"Select Deployment Target"窗口选择已经连接的Android设备（连接失败请检查本机adb工具是否正常），然后点击"OK"按钮；
    * 由于Demo所用到的库和模型均通过app/build.gradle脚本在线下载，因此，第一次编译耗时较长（取决于网络下载速度），请耐心等待；

## 更换自己的模型

1. 使用自己训练的模型，根据PaddleDetection中[lite部署文档](https://github.com/PaddlePaddle/PaddleDetection/tree/release/2.4/deploy/lite)导出模型并转换为lite模型格式（.nb格式的模型）。导出时在config文件中TestReader栏增加`fuse_normalize: true`，将预处理normalize融合进模型。
2. 在`app/src/main/assets/models/yolov3_mobilenet_v3_for_cpu`路径下有**检测lite模型** `model_det.nb`、**关键点lite模型** `model_keypoint.nb` 。依次对应替换。
3. 修改检测模型输入尺寸（默认320）。在`app/src/main/res/values/strings.xml`文件中修改`INPUT_WIDTH_DEFAULT`、`INPUT_HEIGHT_DEFAULT`两项为实际模型使用尺寸。
4. 修改关键点模型尺寸（默认192(w) x 256(h)）。在`app/src/main/cpp/Pipeline.cc`文件中L30行修改输入尺寸（w*h）。


## 更新到最新的预测库（仅在必要时，一般不需要）
* Paddle-Lite项目：https://github.com/PaddlePaddle/Paddle-Lite
* 参考 [Paddle-Lite文档](https://github.com/PaddlePaddle/Paddle-Lite/wiki)，编译IOS预测库、Android和ARMLinux预测库
* 编译最终产物位于 `build.lite.xxx.xxx.xxx` 下的 `inference_lite_lib.xxx.xxx`
### Android更新预测库
* 替换jar文件：将生成的build.lite.android.xxx.gcc/inference_lite_lib.android.xxx/java/jar/PaddlePredictor.jar替换demo中的Paddle-Lite-Demo/PaddleLite-android-demo/image_classification_demo/app/libs/PaddlePredictor.jar
* 替换arm64-v8a jni库文件：将生成build.lite.android.armv8.gcc/inference_lite_lib.android.armv8/java/so/libpaddle_lite_jni.so库替换demo中的Paddle-Lite-Demo/PaddleLite-android-demo/image_classification_demo/app/src/main/jniLibs/arm64-v8a/libpaddle_lite_jni.so
* 替换armeabi-v7a jni库文件：将生成的build.lite.android.armv7.gcc/inference_lite_lib.android.armv7/java/so/libpaddle_lite_jni.so库替换demo中的Paddle-Lite-Demo/PaddleLite-android-demo/image_classification_demo/app/src/main/jniLibs/armeabi-v7a/libpaddle_lite_jni.so.

## 代码结构介绍
主要源码位于工程pose_demo_android/app/src/main目录下，由三大部分组成java、cpp、assets。
* java: Android前端界面相关代码部分
* cpp: 算法实现相关代码
* assets: 模型存放位置

pose_demo_android/app/src目录结构如下：
```
src/
|-- main
    |-- java                                        Java源码（Andoird前端部分）
    |   `-- com.baidu.paddle.lite.demo
    |       |-- common
    |       `-- yolo_detection
    |-- cpp                                         C++ 源码（底层算法部分）
    |   |-- CMakeLists.txt                          编译文件
    |   |-- Native.h                                Jni高级类型数据格式转换辅助函数
    |   |-- Native.cc                               Cpp与Java交互Jni接口
    |   |-- Pipeline.h
    |   |-- Pipeline.cc                             算法整体Pipeline流程
    |   |-- Detector.h
    |   |-- Detector.cc                             检测模型实现
    |   |-- Detector_Kpts.h
    |   |-- Detector_Kpts.cc                        关键点模型实现
    |   |-- postprocess.h
    |   |-- postprocess.cc                          关键点后处理所需函数
    |   |-- pose_action.h
    |   |-- pose_action.cc                          动作识别相关代码
    |   |-- Utils.h
    |   `-- Utils.cc                                模型前处理及其他函数
    `-- assets
        |-- images                                  未使用
        |-- models
        |   `-- yolov3_mobilenet_v3_for_cpu
        |       |--model_det.nb                     检测模型文件
        |       `--model_keypoint.nb                关键点模型文件
        `-- labels
            `-- coco_labels_2014_2017.txt           coco labels文件，实际只用了第一类person
```


## 许可证书

本项目的发布受[Apache 2.0 license](LICENSE)许可认证。


## 贡献代码

我们非常欢迎你可以为PaddleDetection提供代码，也十分感谢你的反馈。
- 感谢[Shigure19](https://github.com/Shigure19)帮忙贡献Android前端代码。

## 引用

```
@misc{ppdet2021,
title={PaddleDetection, Object detection and instance segmentation toolkit based on PaddlePaddle.},
author={PaddlePaddle Authors},
howpublished = {\url{https://github.com/PaddlePaddle/PaddleDetection}},
year={2021}
}
```
