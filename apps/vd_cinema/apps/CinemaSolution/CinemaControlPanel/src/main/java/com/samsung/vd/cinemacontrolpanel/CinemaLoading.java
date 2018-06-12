package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.content.Context;
import android.util.Log;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ProgressBar;

import java.util.Locale;

/**
 * Created by doriya on 11/11/16.
 */
public class CinemaLoading {
    private static final String VD_DTAG = "CinemaLoading";
    private static Dialog mDialog = null;
    private static long mCurTime;

    //
    //  Usage : CinemaLoading.Show( Activity.this );
    //
    public static void Show( Context context ) {
        if( null == mDialog ) {
            mCurTime = System.currentTimeMillis();
            Log.i(VD_DTAG, "Process Start.");

            mDialog = new Dialog(context, R.style.TransparentDialog);
            ProgressBar progressBar = new ProgressBar(context);
            ViewGroup.LayoutParams params = new ViewGroup.LayoutParams( LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT );
            mDialog.addContentView( progressBar, params );
            mDialog.setCancelable( false );
        }
        mDialog.show();
    }

    //
    //  Usage : CinemaLoading.Hide();
    //
    public static void Hide() {
        if( null != mDialog ) {
            mDialog.dismiss();
            mDialog = null;
            Log.i(VD_DTAG, String.format(Locale.US, "Process End. ( %d mSec )", System.currentTimeMillis() - mCurTime ));
        }
    }
}
