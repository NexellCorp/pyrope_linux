package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaService;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;

/**
 * Created by spjun on 18. 3. 19.
 */

public class BaseActivity extends AppCompatActivity {
    private static final String VD_DTAG = "BaseActivity";

    protected RelativeLayout contentParent;
    protected boolean bI2CFailFlag = false;
    protected boolean bRefreshFlag = false;
    protected View.OnClickListener onRefreshClickListener;
    protected void setOnRefreshClickListener(View.OnClickListener onClickListener) {
        onRefreshClickListener = onClickListener;
    }

    protected NXAsync.AsyncCallback baseAsyncCallback = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            BaseActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( BaseActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            BaseActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                if(null != contentParent) {
                                    contentParent.setVisibility(View.GONE);
                                }
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                if(null != contentParent) {
                                    contentParent.setVisibility(View.VISIBLE);
                                }
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, CinemaService.class), mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if( mServiceRun ) {
            unbindService(mConnection);
            mServiceRun = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean isOn = false;

        if (mServiceRun) {
            isOn = mService.IsOn();
            mService.RefreshScreenSaver();
        }
        return !isOn || super.dispatchTouchEvent(ev);
    }

    //
    // Configuration Title Bar and Status Bar
    //
    VdTitleBar titleBar;
    protected void setCommonUI(int titlebarID , int statusbarID , String title , final Intent intent) {
        titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( titlebarID ));
        titleBar.SetTitle( title );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mServiceRun) mService.TurnOff();
            }
        });

        if(null != intent){
            titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    startActivity( intent );
                    overridePendingTransition(0, 0);
                    finish();
                }
            });
        }else{
            titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        }

        if( !((CinemaInfo)getApplicationContext()).IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !((CinemaInfo)getApplicationContext()).IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( statusbarID ) );
    }

    protected void setRefreshBtn(View.OnClickListener onClickListener) {
        if(null == onClickListener) {
            titleBar.SetVisibility(VdTitleBar.BTN_REFRESH , View.GONE);
            titleBar.SetVisibility(VdTitleBar.TXT_REFRESH , View.GONE);
        }else {
            titleBar.SetVisibility(VdTitleBar.TXT_REFRESH , View.VISIBLE);
            titleBar.SetVisibility(VdTitleBar.BTN_REFRESH , View.VISIBLE);
            titleBar.SetListener(VdTitleBar.BTN_REFRESH, onClickListener);
        }
    }

    //
    //  For Internal Toast Message
    //
    protected static Toast mToast;

    protected void ShowMessage( String strMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_LONG);

        mToast.setText(strMsg);
        mToast.show();
    }
    //
    //  For ScreenSaver
    //
    protected CinemaService mService = null;
    private boolean mServiceRun = false;
    protected CinemaService.ChangeContentsCallback mServiceCallback = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            //Log.i(VD_DTAG , "onServiceConnected");
            CinemaService.LocalBinder binder = (CinemaService.LocalBinder)service;
            mService = binder.GetService();
            if(null != mServiceCallback) {
                mService.RegisterCallback( mServiceCallback );
            }
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };

    //
    //  For Screen Rotation
    //
    protected void SetScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        switch( Integer.parseInt(orientation) ) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            default:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
        }
    }

    protected void ChangeScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        int curRotate;
        int prvRotate = Integer.parseInt(orientation);
        switch (prvRotate) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            default:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_ROTATE, String.valueOf(curRotate));
    }
}
