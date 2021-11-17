package com.baidu.paddle.lite.demo.yolo_detection;

import android.content.Context;

public class Native {
    static {
        System.loadLibrary("Native");
    }

    private long ctx = 0;

    public boolean init(String modelDir,
                        String labelPath,
                        int cpuThreadNum,
                        String cpuPowerMode,
                        int inputWidth,
                        int inputHeight,
                        float[] inputMean,
                        float[] inputStd,
                        float scoreThreshold) {
        ctx = nativeInit(
                modelDir,
                labelPath,
                cpuThreadNum,
                cpuPowerMode,
                inputWidth,
                inputHeight,
                inputMean,
                inputStd,
                scoreThreshold);
        return ctx == 0;
    }

    public boolean release() {
        if (ctx == 0) {
            return false;
        }
        return nativeRelease(ctx);
    }

    public boolean process(int inTextureId, int outTextureId, int textureWidth, int textureHeight, String savedImagePath) {
        if (ctx == 0) {
            return false;
        }
        int[] records=nativeProcess(ctx, inTextureId, outTextureId, textureWidth, textureHeight, savedImagePath,1,true);
        return true;
    }

    public int[] getActionCount(int inTextureId, int outTextureId, int textureWidth, int textureHeight, String savedImagePath, int actionid, boolean single) {
        return nativeProcess(ctx, inTextureId, outTextureId, textureWidth, textureHeight, savedImagePath, actionid, single);

    }

    public boolean reset() {
        return nativeReset(ctx);
    }

    public static native long nativeInit(String modelDir,
                                         String labelPath,
                                         int cpuThreadNum,
                                         String cpuPowerMode,
                                         int inputWidth,
                                         int inputHeight,
                                         float[] inputMean,
                                         float[] inputStd,
                                         float scoreThreshold);

    public static native boolean nativeRelease(long ctx);

    public static native int[] nativeProcess(long ctx, int inTextureId, int outTextureId, int textureWidth, int textureHeight, String savedImagePath, int actionid, boolean single);

    public static native boolean nativeReset(long ctx);
}
