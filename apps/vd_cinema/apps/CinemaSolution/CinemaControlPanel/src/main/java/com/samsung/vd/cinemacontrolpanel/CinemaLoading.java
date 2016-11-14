package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.content.Context;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ProgressBar;

/**
 * Created by doriya on 11/11/16.
 */
public class CinemaLoading {
    private static Dialog mDialog = null;

    //
    //  Usage : CinemaLoading.Show( Activity.this );
    //
    public static void Show( Context context ) {
        if( null == mDialog ) {
            mDialog = new Dialog(context, R.style.TransDialog);
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
        }
    }
}
