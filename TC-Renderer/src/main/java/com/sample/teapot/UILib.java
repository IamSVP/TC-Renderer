package com.sample.teapot;

/**
 * Created by tanmay on 4/24/2017.
 */

public class UILib {

    static {
        System.loadLibrary("TeapotNativeActivity");
    }

    public static native void setAlgorithm(int i);
}
