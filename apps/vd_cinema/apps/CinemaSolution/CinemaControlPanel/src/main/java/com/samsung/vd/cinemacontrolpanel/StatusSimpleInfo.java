package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 10/31/16.
 */
public class StatusSimpleInfo {
    private String   mId;
    private int      mStatus = -1;      // -1: unknown, 0: fail, 1: pass

    public StatusSimpleInfo( String id ) {
        mId = id;
    }

    public StatusSimpleInfo( String id, int status ) {
        mId = id;
        mStatus = status;
    }

    public String GetId() {
        return mId;
    }

    public void SetId( String id ) {
        mId = id;
    }

    public int GetStatus() {
        return mStatus;
    }

    public void SetStatus( int status ) {
        mStatus = status;
    }
}
