package com.samsung.vd.cinemacontrolpanel;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import java.util.Locale;

/**
 * Created by doriya on 9/7/17.
 */

public class CinemaLoading2 extends Activity {
    private static final String VD_DTAG = "CinemaLoading";

    private static Activity mActivity = null;
    private static Dialog mDialog = null;
    private static boolean mShow = false;
    private static boolean mShowing = false;

    private static long mCurTime;

    //
    //  Usage : CinemaLoading.Show( getApplicationContext() )
    //
    public static synchronized void Show(Context context) {
        if( mShow ) return;

        mCurTime = System.currentTimeMillis();
        Log.i(VD_DTAG, "Process Start.");

        Intent intent = new Intent( context, CinemaLoading2.class );
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
        mShow = true;
    }

    //
    //  Usage : CinemaLoading.Hide();
    //
    public static synchronized void Hide() {
        if( !mShow ) return;

        new Handler().post(new Runnable(){
            @Override
            public void run() {
                while(!mShowing) {
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                if( null != mDialog ) {
                    mDialog.dismiss();
                    mDialog = null;
                }

                if( null != mActivity ) {
                    mActivity.finish();
                    mActivity = null;
                }

                Log.i(VD_DTAG, String.format(Locale.US, "Process End. ( %d mSec )", System.currentTimeMillis() - mCurTime ));
                mShow = false;
            }
        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if( null == mActivity ) {
            mActivity = this;
        }

        if( null == mDialog ) {
            ProgressBar progressBar = new ProgressBar(this);
            ViewGroup.LayoutParams params = new ViewGroup.LayoutParams( ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT );

            mDialog = new Dialog(this, R.style.TransparentDialog);
            mDialog.addContentView( progressBar, params );
            mDialog.setCancelable( true );
        }

        mDialog.show();
        mShowing = true;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mShowing = false;
    }
}