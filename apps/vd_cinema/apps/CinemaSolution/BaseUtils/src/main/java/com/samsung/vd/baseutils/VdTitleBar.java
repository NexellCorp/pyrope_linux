package com.samsung.vd.baseutils;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.TextView;

/**
 * Created by doriya on 8/17/16.
 */
public class VdTitleBar implements View.OnClickListener {
    private static final String VD_DTAG = "VdTitleBar";

    public static int BTN_BACK = 0;
    public static int BTN_EXIT = 1;
    public static int TXT_TITLE = 2;

    private Context mContext;

    private View mView = null;
    private ViewGroup mViewGroup = null;
    private ViewGroup.LayoutParams mViewGroupLayoutParams;

    private ImageButton mBtnBack;
    private ImageButton mBtnExit;
    private TextView mTextTitle;

    private View.OnClickListener mBackListener  = null;
    private View.OnClickListener mExitListener  = null;

    public VdTitleBar(Context context, ViewGroup viewGroup ) {
        mContext = context;

        mViewGroup = viewGroup;
        mViewGroupLayoutParams = viewGroup.getLayoutParams();

        LayoutInflater inflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mView       = inflater.inflate(R.layout.vd_title_bar, null);
        mBtnBack    = (ImageButton)mView.findViewById( R.id.btnBack );
        mBtnExit    = (ImageButton)mView.findViewById( R.id.btnExit );
        mTextTitle  = (TextView)mView.findViewById( R.id.textTitle );

        mBtnBack.setOnClickListener( this );
        mBtnExit.setOnClickListener( this );

        mViewGroup.addView( mView, mViewGroupLayoutParams );
    }

    public void SetTitle( String title ) {
        mTextTitle.setText( title );
    }

    public void SetListener( int id, View.OnClickListener listener ) {
        if( id == BTN_BACK )        mBackListener = listener;
        else if( id == BTN_EXIT )   mExitListener = listener;
    }

    public void SetVisibility( int id, int visibility ) {
        if( id == BTN_BACK )        mBtnBack.setVisibility( visibility );
        else if( id == BTN_EXIT )   mBtnExit.setVisibility( visibility );
        else if( id == TXT_TITLE )  mTextTitle.setVisibility( visibility );
    }

    @Override
    public void onClick(View view) {
        if( ((ImageButton)view).getId() == R.id.btnBack ) {
            if( mBackListener != null )
                mBackListener.onClick(view);
        }
        else if( ((ImageButton)view).getId() == R.id.btnExit ) {
            if( mExitListener != null )
                mExitListener.onClick(view);
        }
    }
}
