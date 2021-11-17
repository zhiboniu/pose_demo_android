package com.baidu.paddle.lite.demo.yolo_detection;

import android.app.Activity;
import android.content.Intent;
import android.content.res.TypedArray;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.RequiresApi;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SelectActivity extends Activity implements View.OnClickListener {

    private String mode;

    private String[] pose_name;

    private String[] pose_image;

    private int[] pose_action_id;

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_select);
        initView();
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void initView() {
        pose_name = getResources().getStringArray(R.array.pose_name);
        pose_image = getResources().getStringArray(R.array.pose_image);
        pose_action_id = getResources().getIntArray(R.array.pose_action_id);
        mode = getIntent().getStringExtra("mode");
        TextView title = findViewById(R.id.select_title);
        if ("single".equals(mode)) {
            title.setText("单人训练模式");
        } else if ("vs".equals(mode)) {
            title.setText("双人竞技模式");
        }
        LinearLayout poseSelect = findViewById(R.id.pose_select);
        for (int i = 1; i < pose_name.length; i++) {
            PoseButton newPose = new PoseButton(getApplicationContext());
            int finalI = i;
            newPose.setPoseName(pose_name[finalI]);
            newPose.setPoseImage(getResources().getIdentifier(pose_name[finalI], "drawable", getApplicationContext().getPackageName()));
            newPose.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent myIntent = new Intent();
                    if ("single".equals(mode)) {
                        myIntent = new Intent(SelectActivity.this, SingleActivity.class);
                    } else if ("vs".equals(mode)) {
                        myIntent = new Intent(SelectActivity.this, VSActivity.class);
                    }
                    myIntent.putExtra("pose", pose_action_id[finalI]);
                    startActivity(myIntent);
                }
            });
            poseSelect.addView(newPose);

        }
        View pose_a_btn = findViewById(R.id.pose_a);
        View pose_b_btn = findViewById(R.id.pose_b);
        View pose_c_btn = findViewById(R.id.pose_c);
        View btn_back = findViewById(R.id.btn_back);

        pose_a_btn.setOnClickListener(this);
        pose_b_btn.setOnClickListener(this);
        pose_c_btn.setOnClickListener(this);
        btn_back.setOnClickListener(this);


    }

    @Override
    public void onClick(View v) {
        int pose = 0;
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
