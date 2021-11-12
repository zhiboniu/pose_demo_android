package com.baidu.paddle.lite.demo.yolo_detection;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import com.baidu.paddle.lite.demo.yolo_detection.R;
import org.w3c.dom.Text;

import java.util.Objects;

public class SelectActivity extends Activity implements View.OnClickListener {

    private View pose_a_btn;
    private View pose_b_btn;
    private View pose_c_btn;
    private View btn_back;

    private String mode;

    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_select);
        initView();
    }

    private void initView() {
        pose_a_btn = (View) findViewById(R.id.pose_a);
        pose_b_btn = (View) findViewById(R.id.pose_b);
        pose_c_btn = (View) findViewById(R.id.pose_c);
        btn_back = (View) findViewById(R.id.btn_back);

        pose_a_btn.setOnClickListener(this);
        pose_b_btn.setOnClickListener(this);
        pose_c_btn.setOnClickListener(this);
        btn_back.setOnClickListener(this);

        mode = getIntent().getStringExtra("mode");
        TextView title = (TextView) findViewById(R.id.select_title);
        if ("single".equals(mode)) {
            title.setText("单人训练模式");
        } else if ("vs".equals(mode)) {
            title.setText("双人竞技模式");
        }
    }

    @Override
    public void onClick(View v) {
        String pose = new String();
        switch (v.getId()) {
            case R.id.btn_back:
                finish();
                return;
            case R.id.pose_a:
                pose = "pose_a";
                break;
            case R.id.pose_b:
                pose = "pose_b";
                break;
            case R.id.pose_c:
                pose = "pose_c";
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
