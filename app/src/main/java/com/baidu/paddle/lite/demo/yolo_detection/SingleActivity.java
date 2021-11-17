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
import java.util.ArrayList;


public class SingleActivity extends Activity implements View.OnClickListener, CameraSurfaceView.OnTextureChangedListener, SeekBar.OnSeekBarChangeListener {
    CameraSurfaceView svPreview;
    String savedImagePath = "";
    Native predictor = new Native();
    //上方指示器
    private TextView timer;
    private TextView count;
    private TextView calories;
    //运行状态变量
    private boolean playing;
    private boolean pausing;
    //主要界面
    private View beforePlayingControl;
    private View playingControl;
    private View afterPlayingControl;
    private int page;
    //提示工具
    private Toast myToast;
    private TextView overlayText;
    //主计时器
    private CountDownTimer time;
    private long millisUntilFinished;
    //动作计数
    private int actionCount;
    private ArrayList<Double> caloriesPerAction;
    private int[] action_id;
    //动作代码
    private int pose;

    //时间选择器
    private int timeSecond = 15;
    private TextView timeShow;

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
        // Open camera until the permissions have been granted
        checkAndUpdateSettings();
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
        playing = false;
        pausing = false;
        actionCount = 0;
        pose = getIntent().getIntExtra("i", 1);
        caloriesPerAction = new ArrayList<>();
        action_id = getResources().getIntArray(R.array.pose_action_id);
        for (String s : getResources().getStringArray(R.array.calories)
        ) {
            caloriesPerAction.add(Double.valueOf(s));
        }
        String[] title = getResources().getStringArray(R.array.pose_name);

        svPreview = findViewById(R.id.sv_preview);
        svPreview.setOnTextureChangedListener(this);

        SeekBar timePicker = findViewById(R.id.time_picker);
        timePicker.setOnSeekBarChangeListener(this);

        timer = findViewById(R.id.time_count);
        count = findViewById(R.id.count_count);
        calories = findViewById(R.id.calories_count);

        beforePlayingControl = findViewById(R.id.before_playing_control);
        playingControl = findViewById(R.id.playing_control);
        afterPlayingControl = findViewById(R.id.after_playing_control);

        //所有按钮
        Button btnStart = findViewById(R.id.start);
        Button btnPause = findViewById(R.id.pause);
        Button btnStop = findViewById(R.id.stop);
        Button btnRemake = findViewById(R.id.remake);
        Button btnAfterReplay = findViewById(R.id.after_replay);
        Button btnAfterHome = findViewById(R.id.after_home);
        View btnBack = findViewById(R.id.btn_back);
        View btnHome = findViewById(R.id.btn_home);

        btnStart.setOnClickListener(this);
        btnPause.setOnClickListener(this);
        btnStop.setOnClickListener(this);
        btnRemake.setOnClickListener(this);
        btnAfterReplay.setOnClickListener(this);
        btnAfterHome.setOnClickListener(this);
        btnBack.setOnClickListener(this);
        btnHome.setOnClickListener(this);

        overlayText = findViewById(R.id.overlay_text);
        timeShow = findViewById(R.id.time_show);
        TextView poseTitle = findViewById(R.id.title_single);
        poseTitle.setText(title[pose]);

        //演示视频播放
        String uri = "android.resource://" + getPackageName() + "/";
        if (pose == 1) {
            uri += R.raw.pose_a_single;
        } else if (pose == 2) {
            uri += R.raw.pose_b_single;
        } else if (pose == 3) {
            uri += R.raw.pose_c_single;
        }
        //演示视频
        VideoView sampleVideo = findViewById(R.id.sample_video);
        sampleVideo.setVideoPath(uri);
        sampleVideo.setVideoURI(Uri.parse(uri));
        sampleVideo.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mediaPlayer) {
                mediaPlayer.setVolume(0f, 0f);
            }
        });
        sampleVideo.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mediaPlayer) {
                mediaPlayer.seekTo(0);
            }
        });
        sampleVideo.start();
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

    private CountDownTimer getCountDownTimer(long millisInFuture) {
        return new CountDownTimer(millisInFuture, 1000) {
            @Override
            public void onTick(long l) {
                DecimalFormat td = new DecimalFormat("######");
                timer.setText(td.format(Math.floor(l / 1000 + 1)) + "s");
                millisUntilFinished = l;
                count.setText(String.valueOf(actionCount));
                double caloriesValue = actionCount * caloriesPerAction.get(pose);
                DecimalFormat cd = new DecimalFormat("######.#");
                calories.setText(cd.format(caloriesValue) + "cal");
            }

            @Override
            public void onFinish() {
                stop();
            }
        };
    }

    private void start() {
        //获得时间，设定chronometer，跳转到页面2
        //变量初始化
        actionCount = 0;
        count.setText("0");
        calories.setText("0cal");
        time = getCountDownTimer(timeSecond * 1000L);
        overlayText.setText("准备好了吗？");
        timer.setText(timeSecond + "s");

        new CountDownTimer(4000, 1000) {
            @Override
            public void onTick(long l) {
                if (l < 1000) {
                    overlayText.setText("训练开始！");
                } else {
                    overlayText.setText(String.valueOf(String.valueOf(l / 1000).charAt(0)));
                }
            }

            @Override
            public void onFinish() {
                overlayText.setText("");
                predictor.reset();
                time.start();
                playing = true;

            }
        }.start();
        pageControl(2);
    }

    private void stop() {
        //重置chronometer并跳转页面
        TextView c = findViewById(R.id.total_count_text);
        c.setText("总计：" + actionCount);
        TextView k = findViewById(R.id.total_calories_text);
        double caloriesValue = actionCount * caloriesPerAction.get(pose);
        DecimalFormat cd = new DecimalFormat("######.#");
        k.setText("卡路里：" + cd.format(caloriesValue) + "cal");
        pageControl(3);
        clean();
    }

    private void pause() {
        //翻转暂停变量，并对chronometer做相应操作
        showToast("暂停训练！");
        Button btnPause = findViewById(R.id.pause);
        if (playing) {
            pausing = !pausing;
            if (pausing) {
                btnPause.setText("恢复");
                overlayText.setText("暂停中……");
                time.cancel();
                svPreview.releaseCamera();
            } else {
                btnPause.setText("暂停");
                overlayText.setText("");
                time = getCountDownTimer(millisUntilFinished);
                time.start();
                svPreview.openCamera();
            }
        }
    }

    private void remake() {
        //重置chronometer并回到主页
        Intent intent = getIntent();
        finish();
        startActivity(intent);
    }

    private void clean() {
        try {
            timer.setText("0s");
            actionCount = 0;
            time.cancel();
            overlayText.setText("");
            playing = false;
            pausing = false;
        } catch (Exception e) {
            e.printStackTrace();
        }
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
            case R.id.after_replay:
                //直接回主页
                remake();
                break;
            case R.id.after_home:
            case R.id.btn_home:
                //going home
                Intent i = new Intent(SingleActivity.this, MainActivity.class);
                startActivity(i);
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
        } else {
            myToast.setText(text);
        }
        myToast.show();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progressValue, boolean fromUser) {
        timeSecond = progressValue;
        timeShow.setText(timeSecond + "s");
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public boolean onTextureChanged(int inTextureId, int outTextureId, int textureWidth, int textureHeight) {
        synchronized (this) {
            savedImagePath = SingleActivity.this.savedImagePath;
        }
        boolean modified = predictor.process(inTextureId, outTextureId, textureWidth, textureHeight, savedImagePath);
        if (!savedImagePath.isEmpty()) {
            synchronized (this) {
                SingleActivity.this.savedImagePath = "";
            }
        }
        actionCount = predictor.getActionCount(action_id[pose], true)[0];
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
