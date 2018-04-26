package com.samsung.vd.cinemacontrolpanel.InfoClass;

/**
 * Created by doriya on 11/14/16.
 */
public class StatusDescribeInfo {
    private String  mTitle;
    private String  mDescription = "";

    public StatusDescribeInfo( String title ) {
        mTitle = title;
    }

    public StatusDescribeInfo( String title, String description ) {
        mTitle = title;
        mDescription = description;
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
}
