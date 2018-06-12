package com.samsung.vd.cinemacontrolpanel;

import java.util.ArrayList;

/**
 * Created by doriya on 12/21/16.
 */

public class StatusDescribeExpandableInfo {
    private String mGroup;
    private ArrayList<StatusDescribeInfo> mInfo = new ArrayList<>();

    public StatusDescribeExpandableInfo( String group ) {
        mGroup = group;
    }

    public String GetGroup() {
        return mGroup;
    }

    public void SetGroup( String group ) {
        mGroup = group;
    }

    public StatusDescribeInfo GetInfo( int pos ) {
        return mInfo.get(pos);
    }

    public void AddInfo( StatusDescribeInfo info ) {
        mInfo.add( info );
    }

    public void Clear() {
        mInfo.clear();
    }

    public int GetSize() {
        return mInfo.size();
    }
}
