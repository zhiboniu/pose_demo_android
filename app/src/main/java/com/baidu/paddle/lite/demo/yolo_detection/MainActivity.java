package com.baidu.paddle.lite.demo.yolo_detection;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;


public class MainActivity extends Activity implements View.OnClickListener {
    private int clickedLogo;

    private Toast myToast;

    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_main);
        initView();
    }

    private void initView() {
        View singleBtn = findViewById(R.id.main_select_singlemode);
        View vsBtn = findViewById(R.id.main_select_vsmode);
        View logo = findViewById(R.id.pplogo);
        singleBtn.setOnClickListener(this);
        vsBtn.setOnClickListener(this);
        logo.setOnClickListener(this);
        clickedLogo = 0;
    }

    @Override
    public void onClick(View v) {
        final int clickedLogoTimes = 7;
        final int clickedLogoToastTimes = 2;
        switch (v.getId()) {
            case R.id.main_select_singlemode:
                Intent i = new Intent(MainActivity.this, SelectActivity.class);
                i.putExtra("mode", "single");
                startActivity(i);
                break;
            case R.id.main_select_vsmode:
                Intent j = new Intent(MainActivity.this, SelectActivity.class);
                j.putExtra("mode", "vs");
                startActivity(j);
                break;
            case R.id.pplogo:
                clickedLogo++;
                if (clickedLogo == clickedLogoTimes) {
                    clickedLogo = 0;
                    Intent k = new Intent(MainActivity.this, TestActivity.class);
                    startActivity(k);
                } else if (clickedLogo >= clickedLogoToastTimes) {
                    String s = "再点击" + (clickedLogoTimes - clickedLogo) + "次进入测试模式。";
                    showToast(s);
                }
            default:
        }
    }

    private void showToast(String text) {
        if (myToast == null) {
            myToast = Toast.makeText(getApplicationContext(), text, Toast.LENGTH_SHORT);
        } else {
            myToast.setText(text);
        }
        myToast.show();
    }
}
