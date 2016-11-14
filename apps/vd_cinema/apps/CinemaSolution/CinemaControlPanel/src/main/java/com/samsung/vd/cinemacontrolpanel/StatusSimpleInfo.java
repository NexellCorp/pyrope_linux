package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 10/31/16.
 */
public class StatusSimpleInfo {
    private String  mTitle;
    private int     mStatus = -1;      // -1: unknown, 0: fail, 1: pass

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
