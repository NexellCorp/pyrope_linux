package com.samsung.vd.cinemacontrolpanel.InfoClass;

/**
 * Created by doriya on 10/31/16.
 */
public class StatusSimpleInfo {
    public static final int PASS = 1;
    public static final int FAIL = 0;
    public static final int ERROR = -1;

    private String  mTitle;
    private int     mStatus = -1;      // -1: error, 0: fail, 1: pass

    public StatusSimpleInfo( String title ) {
        mTitle = title;
    }

    public StatusSimpleInfo( String title, int status ) {
        mTitle  = title;
        mStatus = status;
    }

    public String GetTitle() {
        return mTitle;
    }

    public void SetTitle( String title ) {
        mTitle = title;
    }

    public int GetStatus() {
        return mStatus;
    }

    public void SetStatus( int status ) {
        mStatus = status;
    }
}
