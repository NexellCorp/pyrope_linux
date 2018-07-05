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
import java.util.concurrent.Semaphore;

/**
 * Created by doriya on 9/7/17.
 */

public class CinemaLoading extends Activity {
    private static final String VD_DTAG = "CinemaLoading";

    private static Handler mHandler = null;
    private static Activity mActivity = null;
    private static Dialog mDialog = null;
    private static Semaphore mSemShow = new Semaphore( 1 );
    private static long mCurTime;

    //
    //  Usage : CinemaLoading.Show( getApplicationContext() )
    //
    public static void Show(Context context) {
        try {
            if( mSemShow.availablePermits() == 0 )
                return;

            mSemShow.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mCurTime = System.currentTimeMillis();
        // Log.i(VD_DTAG, "Process Start.");

        Intent intent = new Intent( context, CinemaLoading.class );
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    //
    //  Usage : CinemaLoading.Hide();
    //
    public static void Hide() {
        if( mSemShow.availablePermits() != 0 ) {
            return;
        }

        if( null != mHandler ) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        mHandler = new Handler();
        mHandler.postDelayed(new Runnable(){
            @Override
            public void run() {
                if( null != mDialog ) {
                    mDialog.dismiss();
                    mDialog = null;
                }

                if( null != mActivity ) {
                    mActivity.finish();
                    mActivity = null;
                }

                // Log.i(VD_DTAG, String.format(Locale.US, "Process End. ( %d mSec )", System.currentTimeMillis() - mCurTime ));
            }
        },100 );
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
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSemShow.release();
    }
}
