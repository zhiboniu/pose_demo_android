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
            final int t = i;
            newPose.setPoseName(pose_name[t]);
            int poseImageId = getResources().getIdentifier(pose_image[t], "drawable", getPackageName());
            newPose.setPoseImage(poseImageId);
            newPose.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent myIntent = new Intent();
                    if ("single".equals(mode)) {
                        myIntent = new Intent(SelectActivity.this, SingleActivity.class);
                    } else if ("vs".equals(mode)) {
                        myIntent = new Intent(SelectActivity.this, VSActivity.class);
                    }
                    myIntent.putExtra("pose", pose_action_id[t]);
                    myIntent.putExtra("i", t);
                    startActivity(myIntent);
                }
            });
            poseSelect.addView(newPose);

        }
        View btn_back = findViewById(R.id.btn_back);

        btn_back.setOnClickListener(this);


    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_back:
                finish();
                return;
            default:
        }
    }
}
