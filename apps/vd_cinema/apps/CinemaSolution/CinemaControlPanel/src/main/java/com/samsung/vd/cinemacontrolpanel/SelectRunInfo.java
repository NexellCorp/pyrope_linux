package com.samsung.vd.cinemacontrolpanel;

import android.view.View;
import android.widget.Spinner;

/**
 * Created by doriya on 11/25/16.
 */

public class SelectRunInfo {
    private String mTitle;
    private String[] mSpinnerItem;
    private boolean mIsToggle;
    private boolean mStatus = false;

    private SelectRunAdapter.OnClickListener mOnClickListener;

    public SelectRunInfo( String title ) {
        mTitle = title;
    }

    public SelectRunInfo(String title, String[] spinnerItem) {
        mTitle = title;
        mSpinnerItem = spinnerItem;
    }

    public SelectRunInfo(String title, String[] spinnerItem, boolean isToggle, SelectRunAdapter.OnClickListener listener) {
        mTitle = title;
        mSpinnerItem = spinnerItem;
        mIsToggle = isToggle;
        mOnClickListener = listener;
    }

    public String GetTitle() {
        return mTitle;
    }

    public void SetTitle( String title ) {
        mTitle = title;
    }

    public String[] GetSpinnerItem() {
        return mSpinnerItem;
    }

    public void SetSpinnerItem( String[] spinnerItem ) {
        mSpinnerItem = spinnerItem;
    }

    public void SetOnClickListener(SelectRunAdapter.OnClickListener listener ) {
        mOnClickListener = listener;
    }

    public SelectRunAdapter.OnClickListener GetOnClickListener() {
        return mOnClickListener;
    }

    public void SetIsToggle( boolean isToggle ) {
        mIsToggle = isToggle;
    }

    public boolean GetIsToggle() {
        return mIsToggle;
    }

    public void SetStatus( boolean status ) {
        mStatus = status;
    }

    public boolean GetStatus() {
        return mStatus;
    }
}
