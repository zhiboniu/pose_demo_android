package com.baidu.paddle.lite.demo.yolo_detection;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.app.Activity;
import android.widget.Toast;
import com.baidu.paddle.lite.demo.yolo_detection.TestActivity;
import com.baidu.paddle.lite.demo.yolo_detection.R;


public class MainActivity extends Activity implements View.OnClickListener {
    private int clickedLogo;
    private View single_btn;
    private View vs_btn;
    private View logo;

    private Toast myToast;

    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_main);
        initView();
    }

    private void initView() {
        single_btn = (View) findViewById(R.id.main_select_singlemode);
        vs_btn = (View) findViewById(R.id.main_select_vsmode);
        logo=(View) findViewById(R.id.pplogo);
        single_btn.setOnClickListener(this);
        vs_btn.setOnClickListener(this);
        logo.setOnClickListener(this);
        clickedLogo=0;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.main_select_singlemode:
                Intent i = new Intent(MainActivity.this, SelectActivity.class);
                startActivity(i);
                break;
            case R.id.main_select_vsmode:
                Intent j = new Intent(MainActivity.this, SelectActivity.class);
                startActivity(j);
                break;
            case R.id.pplogo:
                clickedLogo++;
                if(clickedLogo==7) {
                    clickedLogo = 0;
                    Intent k = new Intent(MainActivity.this, TestActivity.class);
                    startActivity(k);
                }else if(clickedLogo>=2){
                    String s="再点击"+String.valueOf(7-clickedLogo)+"次进入测试模式。";
                    showToast(s);
                }
        }
    }
    private void showToast(String text){
        if(myToast==null){
            myToast = Toast.makeText(getApplicationContext(),text,Toast.LENGTH_SHORT);
            myToast.show();
        }else{
            myToast.setText(text);
            myToast.show();
        }
    }
}
