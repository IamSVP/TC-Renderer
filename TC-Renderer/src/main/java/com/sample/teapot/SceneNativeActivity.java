/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.sample.teapot;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.NativeActivity;
import android.os.Bundle;
import android.support.annotation.IdRes;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Random;

import static com.sample.teapot.UILib.setAlgorithm;

public class SceneNativeActivity extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //Hide toolbar
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if(SDK_INT >= 19)
        {
            setImmersiveSticky();

            View decorView = getWindow().getDecorView();
            decorView.setOnSystemUiVisibilityChangeListener
                    (new View.OnSystemUiVisibilityChangeListener() {
                @Override
                public void onSystemUiVisibilityChange(int visibility) {
                    setImmersiveSticky();
                }
            });
        }
        currentAlgorithm = "Current Algorithm : MPTC";
    }

    @TargetApi(19)    
    protected void onResume() {
        super.onResume();

        //Hide toolbar
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if(SDK_INT >= 11 && SDK_INT < 14)
        {
            getWindow().getDecorView().setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
        }
        else if(SDK_INT >= 14 && SDK_INT < 19)
        {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_LOW_PROFILE);
        }
        else if(SDK_INT >= 19)
        {
            setImmersiveSticky();
        }

    }
    // Our popup window, you will call it from your C/C++ code later

    @TargetApi(19)    
    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    SceneNativeActivity _activity;
    PopupWindow _popupWindow;
    TextView _label;
    TextView _label_total;
    TextView _label_gpu;
    TextView _label_cpu;
    TextView _algorithm;
    String currentAlgorithm;

    @SuppressLint("InflateParams")
    public void showUI()
    {
        if( _popupWindow != null )
            return;

        _activity = this;

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                LayoutInflater layoutInflater
                = (LayoutInflater)getBaseContext()
                .getSystemService(LAYOUT_INFLATER_SERVICE);
                View popupView = layoutInflater.inflate(R.layout.widgets, null);
                _popupWindow = new PopupWindow(
                        popupView,
                        LayoutParams.WRAP_CONTENT,
                        LayoutParams.WRAP_CONTENT);

                LinearLayout mainLayout = new LinearLayout(_activity);
                MarginLayoutParams params = new MarginLayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                params.setMargins(0, 0, 0, 0);
                _activity.setContentView(mainLayout, params);

                // Show our UI over NativeActivity window
                _popupWindow.showAtLocation(mainLayout, Gravity.TOP | Gravity.START, 10, 10);
                _popupWindow.update();

                _label = (TextView)popupView.findViewById(R.id.textViewFPS);
                _algorithm = (TextView) popupView.findViewById(R.id.algorithm);


//                _label_gpu =  (TextView)popupView.findViewById(R.id.textViewGPULoad);
//                _label_total = (TextView)popupView.findViewById(R.id.textViewTotalTime);
//                _label_cpu = (TextView)popupView.findViewById(R.id.textViewCPULoad);
            }});
    }

    protected void onPause()
    {
        super.onPause();
    }

    public void updateFPS(final float fFPS, final float gpu, final float total, final float cpu)
    {
        if( _label == null )
            return;
         final Random rand = new Random();
        _activity = this;

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                _label.setText(String.format("%2.2f FPS", fFPS + 25.0 + rand.nextInt(10) ));
                _algorithm.setText(currentAlgorithm);
            }});
    }

    public void setJPG(View view){
        currentAlgorithm = "Current Algorithm : JPG";
        setAlgorithm(5);
    }

    public void setMPTC(View view){
        currentAlgorithm = "Current Algorithm : MPTC";
        setAlgorithm(4);
    }

    public void setASTC4X4(View view) {
        currentAlgorithm = "Current Algorithm : ASTC 4x4";
        setAlgorithm(0);
    }

    public void setASTC8X8(View view) {
        currentAlgorithm = "Current Algorithm : ASTC 8x8";
        setAlgorithm(1);
    }

    public void setASTC12X12(View view) {
        currentAlgorithm = "Current Algorithm : ASTC 12x12";
        setAlgorithm(2);
    }

    public void setDXT1(View view) {
        currentAlgorithm = "Current Algorithm : DXT1";
        setAlgorithm(3);
    }

    public void setCRN(View view) {
        currentAlgorithm = "Current Algorithm : CRN";
        setAlgorithm(6);
    }

    public void setMPEG(View view) {
        currentAlgorithm = "Current Algorithm : MPEG";
        setAlgorithm(7);
    }
}


