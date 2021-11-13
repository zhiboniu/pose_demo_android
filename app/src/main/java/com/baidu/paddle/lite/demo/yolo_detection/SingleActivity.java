package com.baidu.paddle.lite.demo.yolo_detection;

import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.app.Activity;
import android.widget.*;

import com.baidu.paddle.lite.demo.common.CameraSurfaceView;
import com.baidu.paddle.lite.demo.common.Utils;

import java.text.DecimalFormat;


public class SingleActivity extends Activity implements View.OnClickListener, CameraSurfaceView.OnTextureChangedListener {
    CameraSurfaceView svPreview;
    String savedImagePath = "";
    int lastFrameIndex = 0;
    long lastFrameTime;
    Native predictor = new Native();

    private TextView timer;
    private TextView count;
    private TextView calories;

    private int count_int;

    private boolean playing;
    private boolean pausing;

    private View beforePlayingControl;
    private View playingControl;
    private View afterPlayingControl;
    private int page;

    private Button btn_start;

    private Button btn_pause;
    private Button btn_stop;
    private Button btn_remake;
    private Button btn_after_replay;
    private Button btn_after_home;

    private View btn_back;
    private View btn_home;

    private Toast myToast;

    private TextView overlay_text;

    private CountDownTimer time;
    private long millisUntilFinished;

    private int action_count;
    private final double calories_per_action = 100;

    private String pose;

    private VideoView sample_video;

    @Override
    protected void onCreate(Bundle savedInstanceBundle) {
        super.onCreate(savedInstanceBundle);
        setContentView(R.layout.activity_single);
        initSettings();
        initView();
        if (!checkAllPermissions()) {
            requestAllPermissions();
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        // Reload settings and re-initialize the predictor
        checkAndUpdateSettings();
        // Open camera until the permissions have been granted
        if (!checkAllPermissions()) {
            svPreview.disableCamera();
        }
        svPreview.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        svPreview.onPause();
    }

    @Override
    protected void onDestroy() {
        if (predictor != null) {
            predictor.release();
        }
        super.onDestroy();
    }

    private void initView() {
        svPreview = (CameraSurfaceView) findViewById(R.id.sv_preview);

        timer = (TextView) findViewById(R.id.time_count);
        count = (TextView) findViewById(R.id.count_count);
        calories = (TextView) findViewById(R.id.calories_count);

        svPreview.setOnTextureChangedListener(this);

        beforePlayingControl = (View) findViewById(R.id.before_playing_control);
        playingControl = (View) findViewById(R.id.playing_control);
        afterPlayingControl = (View) findViewById(R.id.after_playing_control);

        btn_start = (Button) findViewById(R.id.start);
        btn_pause = (Button) findViewById(R.id.pause);
        btn_stop = (Button) findViewById(R.id.stop);
        btn_remake = (Button) findViewById(R.id.remake);
        btn_after_replay = (Button) findViewById(R.id.after_replay);
        btn_after_home = (Button) findViewById(R.id.after_home);
        btn_back = (ImageView) findViewById(R.id.btn_back);
        btn_home = (ImageView) findViewById(R.id.btn_home);

        btn_start.setOnClickListener(this);
        btn_pause.setOnClickListener(this);
        btn_stop.setOnClickListener(this);
        btn_remake.setOnClickListener(this);
        btn_after_replay.setOnClickListener(this);
        btn_after_home.setOnClickListener(this);
        btn_back.setOnClickListener(this);
        btn_home.setOnClickListener(this);

        overlay_text = (TextView) findViewById(R.id.overlay_text);

        playing = false;
        pausing = false;
        action_count = 0;

        pose = getIntent().getStringExtra("pose");
        /*
        String uri = "android.resource://" + getPackageName() + "/";
        if ("pose_a".equals(pose)) {
            uri += R.raw.pose_a_single;
        } else if ("pose_b".equals(pose)) {
            uri += R.raw.pose_b_single;
        } else if ("pose_c".equals(pose)) {
            uri += R.raw.pose_c_single;
        }
        sample_video = (VideoView) findViewById(R.id.sample_video);
        sample_video.setVideoPath(uri);
        sample_video.setVideoURI(Uri.parse(uri));
        sample_video.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mediaPlayer) {
                mediaPlayer.setVolume(0f, 0f);
            }
        });
        sample_video.start();
        */
        pageControl(1);
    }

    public void initSettings() {
        SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.clear();
        editor.commit();
        SettingsActivity.resetSettings();
    }

    private void pageControl(int page) {
        if (page == this.page) {
            return;
        } else {
            this.page = page;
        }
        if (page == 1) {
            beforePlayingControl.setVisibility(View.VISIBLE);
            playingControl.setVisibility(View.GONE);
            afterPlayingControl.setVisibility(View.GONE);
            svPreview.setVisibility(View.GONE);
        } else if (page == 2) {
            beforePlayingControl.setVisibility(View.GONE);
            playingControl.setVisibility(View.VISIBLE);
            afterPlayingControl.setVisibility(View.GONE);
            svPreview.setVisibility(View.VISIBLE);
        } else if (page == 3) {
            beforePlayingControl.setVisibility(View.GONE);
            playingControl.setVisibility(View.GONE);
            afterPlayingControl.setVisibility(View.VISIBLE);
            svPreview.setVisibility(View.GONE);
        }
    }

    private void delay(int seconds, Runnable r) {
        new Handler().postDelayed(r, seconds * 1000);
    }

    private CountDownTimer getCountDownTimer(long millisInFuture, long countDownInterval) {
        return new CountDownTimer(millisInFuture, countDownInterval) {
            @Override
            public void onTick(long l) {
                DecimalFormat decimalFormat = new DecimalFormat("######");
                timer.setText(decimalFormat.format(Math.floor(l / 1000 + 1)) + "s");
                millisUntilFinished = l;
            }

            @Override
            public void onFinish() {
                stop();
            }
        };
    }

    private void start() {
        //获得时间，设定chronometer，跳转到页面2
        EditText tp = (EditText) findViewById(R.id.time_picker);
        int timeSecond = Integer.parseInt(tp.getText().toString());
        time = getCountDownTimer(timeSecond * 1000, 1000);
        overlay_text.setText("准备好了吗？");
        timer.setText(timeSecond + "s");
        pageControl(2);
        new CountDownTimer(3000, 1000) {
            @Override
            public void onTick(long l) {
                overlay_text.setText(String.valueOf(String.valueOf(l / 1000 + 1).charAt(0)));
            }

            @Override
            public void onFinish() {
                overlay_text.setText("");
                showToast("训练开始！");
                time.start();
                playing = true;
            }
        }.start();
    }

    private void stop() {
        //重置chronometer并跳转页面
        overlay_text.setText("");
        time.cancel();
        timer.setText("0s");
        TextView c = (TextView) findViewById(R.id.total_count_text);
        c.setText("总计：" + action_count);
        TextView k = (TextView) findViewById(R.id.total_calories_text);
        k.setText("卡路里：" + calories_per_action * action_count);
        pageControl(3);
    }

    private void pause() {
        //翻转暂停变量，并对chronometer做相应操作
        showToast("暂停训练！");
        if (!playing) {
            return;
        } else {
            pausing = !pausing;
            if (pausing) {
                btn_pause.setText("恢复");
                overlay_text.setText("暂停中……");
                time.cancel();
            } else {
                btn_pause.setText("暂停");
                overlay_text.setText("");
                time = getCountDownTimer(millisUntilFinished, 1000);
                time.start();

            }
        }
    }

    private void remake() {
//重置chronometer并回到主页
        showToast("重新开始训练！");
        if (page == 2) {
            time.cancel();
            timer.setText("0s");
            pageControl(1);
        }
    }

    private void after_replay() {

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start:
                start();
                break;
            case R.id.pause:
                pause();
                break;
            case R.id.stop:
                stop();
                break;
            case R.id.remake:
                remake();
                break;
            case R.id.after_home:
            case R.id.btn_home:
                //going home
                Intent i = new Intent(SingleActivity.this, MainActivity.class);
                startActivity(i);
                break;
            case R.id.after_replay:
                //直接回主页
                if (page == 3) {
                    pageControl(1);
                }
                break;
            case R.id.btn_back:
                finish();
                return;
            default:
        }
    }

    private void showToast(String text) {
        if (myToast == null) {
            myToast = Toast.makeText(getApplicationContext(), text, Toast.LENGTH_SHORT);
            myToast.show();
        } else {
            myToast.setText(text);
            myToast.show();
        }
    }

    @Override
    public boolean onTextureChanged(int inTextureId, int outTextureId, int textureWidth, int textureHeight) {
        String savedImagePath = "";
        synchronized (this) {
            savedImagePath = SingleActivity.this.savedImagePath;
        }
        boolean modified = predictor.process(inTextureId, outTextureId, textureWidth, textureHeight, savedImagePath);
        if (!savedImagePath.isEmpty()) {
            synchronized (this) {
                SingleActivity.this.savedImagePath = "";
            }
        }
        lastFrameIndex++;
        if (lastFrameIndex >= 30) {
            final int fps = (int) (lastFrameIndex * 1e9 / (System.nanoTime() - lastFrameTime));
            /*runOnUiThread(new Runnable() {
                public void run() {
                    tvStatus.setText(Integer.toString(fps) + "fps");
                }
            });*/
            lastFrameIndex = 0;
            lastFrameTime = System.nanoTime();
        }
        return modified;
    }

    public void checkAndUpdateSettings() {

        if (SettingsActivity.checkAndUpdateSettings(this)) {
            String realModelDir = getCacheDir() + "/" + SettingsActivity.modelDir;
            Utils.copyDirectoryFromAssets(this, SettingsActivity.modelDir, realModelDir);
            String realLabelPath = getCacheDir() + "/" + SettingsActivity.labelPath;
            Utils.copyFileFromAssets(this, SettingsActivity.labelPath, realLabelPath);
            predictor.init(
                    realModelDir,
                    realLabelPath,
                    SettingsActivity.cpuThreadNum,
                    SettingsActivity.cpuPowerMode,
                    SettingsActivity.inputWidth,
                    SettingsActivity.inputHeight,
                    SettingsActivity.inputMean,
                    SettingsActivity.inputStd,
                    SettingsActivity.scoreThreshold);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (grantResults[0] != PackageManager.PERMISSION_GRANTED || grantResults[1] != PackageManager.PERMISSION_GRANTED) {
            new AlertDialog.Builder(SingleActivity.this)
                    .setTitle("Permission denied")
                    .setMessage("Click to force quit the app, then open Settings->Apps & notifications->Target " +
                            "App->Permissions to grant all of the permissions.")
                    .setCancelable(false)
                    .setPositiveButton("Exit", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            SingleActivity.this.finish();
                        }
                    }).show();
        }
    }

    private void requestAllPermissions() {
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.CAMERA}, 0);
    }

    private boolean checkAllPermissions() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
                && ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED;
    }
}
