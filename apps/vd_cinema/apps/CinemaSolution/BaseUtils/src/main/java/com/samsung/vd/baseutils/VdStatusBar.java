package com.samsung.vd.baseutils;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextClock;

/**
 * Created by doriya on 8/18/16.
 */
public class VdStatusBar {
    private static final String VD_DTAG = "VdStatusBar";

    public static int TXT_TIME = 0;

    private Context mContext;

    private View mView;
    private ViewGroup mViewGroup;
    private ViewGroup.LayoutParams mViewGroupLayoutParams;

    private TextClock mTextClock;

    public VdStatusBar(Context context, ViewGroup viewGroup) {
        mContext = context;

        mViewGroup = viewGroup;
        mViewGroupLayoutParams = viewGroup.getLayoutParams();

        LayoutInflater inflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mView       = inflater.inflate(R.layout.vd_status_bar, null);

        mTextClock = (TextClock)mView.findViewById(R.id.textClock);
        mViewGroup.addView( mView, mViewGroupLayoutParams );
    }

    public void SetVisibility( int id, int visibility ) {
        if( id == TXT_TIME ) mTextClock.setVisibility( visibility );
    }

    public void SetListener( View.OnClickListener listener ) {
        mTextClock.setOnClickListener( listener );
    }
}