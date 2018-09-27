package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 11/25/16.
 */

public class SelectRunInfo {
    private String mTitle;
    private String[] mSpinnerItem;
    private String[] mBtnText = new String[]{"RUN"};
    private boolean mToggle = false;
    private boolean mStatus = false;

    private SelectRunAdapter.OnClickListener mOnClickListener;

    public SelectRunInfo( String title ) {
        mTitle = title;
    }

    public SelectRunInfo(String title, String[] spinnerItem) {
        mTitle = title;
        mSpinnerItem = spinnerItem;
    }

    public SelectRunInfo(String title, String[] spinnerItem, String[] btnText, boolean toggle, boolean status, SelectRunAdapter.OnClickListener listener) {
        mTitle = title;
        mSpinnerItem = spinnerItem;
        mBtnText = btnText;
        mToggle = toggle;
        mStatus = status;
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

    public void SetToggle( boolean isToggle ) {
        mToggle = isToggle;
    }

    public boolean GetToggle() {
        return mToggle;
    }

    public void SetBtnText( String[] btnText ) {
        mBtnText = btnText;
    }

    public String[] GetBtnText() {
        return mBtnText;
    }

    public void SetStatus( boolean status ) {
        mStatus = status;
    }

    public boolean GetStatus() {
        return mStatus;
    }
}
