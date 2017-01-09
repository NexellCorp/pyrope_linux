package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 10/31/16.
 */
public class StatusDetailInfo {
    private String  mTitle;
    private int     mSlave = 0x00;
    private int     mStatus = -1;      // -1: unknown, 0: fail, 1: pass
    private String  mDescription = "";

    public StatusDetailInfo( String title ) {
        mTitle = title;
    }

    public StatusDetailInfo( String title, int status, String description ) {
        mTitle = title;
        mStatus = status;
        mDescription = description;
    }

    public StatusDetailInfo( String title, int slave, int status, String description ) {
        mTitle = title;
        mSlave = slave;
        mStatus = status;
        mDescription = description;
    }

    public String GetTitle() {
        return mTitle;
    }

    public void SetTitle( String title ) {
        mTitle = title;
    }

    public int GetSlave() {
        return mSlave;
    }

    public void SetSlave( int slave ) {
        mSlave = slave;
    }

    public int GetStatus() {
        return mStatus;
    }

    public void SetStatus( int status ) {
        mStatus = status;
    }

    public String GetDescription() {
        return mDescription;
    }

    public void SetDescription( String description ) {
        mDescription = description;
    }
}
