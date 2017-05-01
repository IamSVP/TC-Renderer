package com.sample.teapot;

/**
 * Created by psrihariv on 5/1/17.
 */

public class UILib {

    static {
        System.loadLibrary("TeapotNativeActivity");
    }

    public static native void setAlgorithm(int i);
}
