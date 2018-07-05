package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.Locale;

/**
 * Created by doriya on 5/23/18.
 */

public class CinemaBaseActivity extends AppCompatActivity {
    private CinemaService mService = null;
    private boolean mServiceRun = false;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
    }

    @Override
    protected void onStart() {
        super.onStart();

        Intent intent = new Intent(this, CinemaService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
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
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean isOn = false;
        if( mService != null ) {
            isOn = mService.IsOn();
            mService.RefreshScreenSaver();
        }

        return !isOn || super.dispatchTouchEvent(ev);
    }

    //
    //  TMS Event Done
    //
    private CinemaService.TmsEventCallback mTmsEventCallback;
    protected void RegisterTmsCallback( CinemaService.TmsEventCallback callback ) {
        mTmsEventCallback = callback;
    }

    //
    //  Launch Current Activity to Another Activity
    //
    protected void Launch( Context packageContext, Class<?> cls ) {
        startActivity( new Intent(packageContext, cls) );
        overridePendingTransition(0, 0);
        finish();
    }

    protected void Launch( Context packageContext, Class<?> cls, int delay ) {
        final Context context = packageContext;
        final Class<?> target = cls;

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                startActivity( new Intent(context, target) );
                overridePendingTransition(0, 0);
                finish();
            }
        }, delay);
    }

    protected void LaunchPackage( Context packageContext, String packageName ) {
        //
        //  This is not required "finish()"
        //
        startActivity(packageContext.getPackageManager().getLaunchIntentForPackage(packageName));
    }

    //
    //  For Internal Toast Message
    //
    private static Toast mToast;

    protected void ShowMessage( String szMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_SHORT);

        mToast.setText(szMsg);
        mToast.show();
    }

    //
    //  For Internal Loading
    //
    private Dialog mDialog = null;
    private int mDialogInst = 0;

    protected synchronized void ShowProgress() {
        if( null == mDialog ) {
            mDialog = new Dialog(CinemaBaseActivity.this, R.style.TransparentDialog);
            ProgressBar progressBar = new ProgressBar(CinemaBaseActivity.this);
            ViewGroup.LayoutParams params = new ViewGroup.LayoutParams( ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT );
            mDialog.addContentView( progressBar, params );
            mDialog.setCancelable( false );
        }

        mDialog.show();
        mDialogInst++;
    }

    protected synchronized void HideProgress() {
        mDialogInst--;
        if( mDialogInst <= 0 ) {
            if( null != mDialog ) {
                mDialog.dismiss();
                mDialog = null;
            }
            mDialogInst = 0;
        }
    }

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

    //
    //  For ScreenSaver
    //
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            CinemaService.LocalBinder binder = (CinemaService.LocalBinder)service;
            mService = binder.GetService();
            mService.RegisterTmsCallback( mTmsEventCallback );
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
            mService.TurnOff();
        }
    };

    protected void RefreshScreenSaver() {
        if( mService != null ) {
            mService.RefreshScreenSaver();
        }
    }

    protected void TurnOff() {
        if( mService != null ) {
            mService.TurnOff();
        }
    }
}
