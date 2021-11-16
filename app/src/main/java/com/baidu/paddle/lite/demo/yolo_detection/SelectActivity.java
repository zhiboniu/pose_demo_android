package com.baidu.paddle.lite.demo.yolo_detection;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class SelectActivity extends Activity implements View.OnClickListener {

    private String mode;

    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_select);
        initView();
    }

    private void initView() {
        View pose_a_btn = findViewById(R.id.pose_a);
        View pose_b_btn =  findViewById(R.id.pose_b);
        View pose_c_btn =  findViewById(R.id.pose_c);
        View btn_back = findViewById(R.id.btn_back);

        pose_a_btn.setOnClickListener(this);
        pose_b_btn.setOnClickListener(this);
        pose_c_btn.setOnClickListener(this);
        btn_back.setOnClickListener(this);

        mode = getIntent().getStringExtra("mode");
        TextView title =  findViewById(R.id.select_title);
        if ("single".equals(mode)) {
            title.setText("单人训练模式");
        } else if ("vs".equals(mode)) {
            title.setText("双人竞技模式");
        }
    }

    @Override
    public void onClick(View v) {
        int pose=0;
        switch (v.getId()) {
            case R.id.btn_back:
                finish();
                return;
            case R.id.pose_a:
                pose = 2;
                break;
            case R.id.pose_b:
                pose = 1;
                break;
            case R.id.pose_c:
                pose = 3;
                break;
            default:
        }
        Intent i = new Intent();
        if ("single".equals(mode)) {
            i = new Intent(SelectActivity.this, SingleActivity.class);
        } else if ("vs".equals(mode)) {
            i = new Intent(SelectActivity.this, VSActivity.class);
        }
        i.putExtra("pose", pose);
        startActivity(i);
    }
}
