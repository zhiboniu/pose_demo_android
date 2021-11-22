# pose_demo_android
pose demo on android mobile based on PaddleDetection

本工程Android部分基于[Paddle-Lite-Demo](https://github.com/PaddlePaddle/Paddle-Lite-Demo)修改。

算法模型基于[PaddleDetection](https://github.com/PaddlePaddle/PaddleDetection)的[PP-TinyPose](https://github.com/PaddlePaddle/PaddleDetection/tree/develop/configs/keypoint/tiny_pose).

如欲获取更多详情，请点击链接至相应repo中查看。



## APP安装体验
* Andoird APP[下载体验](https://paddledet.bj.bcebos.com/deploy/paddlelite/PP_TinyPose.apk)
* 安装二维码：

<div align="left">
  <img src="./pictures/PP-TinyPose_download.png" width='200'/>
</div>



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


## 更新到最新的预测库
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


