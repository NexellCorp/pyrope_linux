package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 12/16/16.
 */

public class CheckRunInfo {
    private String  mTitle;
    private String  mDescription = "";
    private boolean mIsCheck = false;

    public CheckRunInfo( String title ) {
        mTitle = title;
    }

    public CheckRunInfo( String title, String description ) {
        mTitle = title;
        mDescription = description;
    }

    public CheckRunInfo( String title, String description, boolean isCheck ) {
        mTitle = title;
        mDescription = description;
        mIsCheck = isCheck;
    }

    public String GetTitle() {
        return mTitle;
    }

    public void SetTitle( String title ) {
        mTitle = title;
    }

    public String GetDescription() {
        return mDescription;
    }

    public void SetDescription( String description ) {
        mDescription = description;
    }

    public boolean GetChecked() {
        return mIsCheck;
    }

    public void SetChecked( boolean isCheck ) {
        mIsCheck = isCheck;
    }
}
