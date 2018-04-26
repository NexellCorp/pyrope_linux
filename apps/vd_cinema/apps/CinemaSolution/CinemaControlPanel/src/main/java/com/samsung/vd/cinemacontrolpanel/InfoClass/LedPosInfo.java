package com.samsung.vd.cinemacontrolpanel.InfoClass;

/**
 * Created by doriya on 11/11/16.
 */
public class LedPosInfo {
    private String  mId;
    private String  mPosX = "-";     // "-": unknown
    private String  mPosY = "-";     // "-": unknown

    public LedPosInfo( String id ) {
        mId = id;
    }

    public LedPosInfo( String id, String posX, String posY ) {
        mId = id;
        mPosX = posX;
        mPosY = posY;
    }

    public String GetId() {
        return mId;
    }

    public void SetId( String id ) {
        mId = id;
    }

    public String GetPosX() {
        return mPosX;
    }

    public void SetPosX( String posX ) {
        mPosX = posX;
    }

    public String GetPosY() {
        return mPosY;
    }

    public void SetPosY( String posY ) {
        mPosY = posY;
    }
}
