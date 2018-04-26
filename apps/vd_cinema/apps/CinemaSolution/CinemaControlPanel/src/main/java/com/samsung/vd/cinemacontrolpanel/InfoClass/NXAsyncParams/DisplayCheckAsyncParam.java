package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.SelectRunAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeExpandableAdapter;

/**
 * Created by spjun on 18. 4. 11.
 */

public class DisplayCheckAsyncParam {

    //
    //AsyncTaskTestPattern
    //
    int DisplayCheckFuncIndex , DisplayCheckPatternIndex;
    boolean DisplayCheckStatus;

    public int getDisplayCheckFuncIndex() {
        return DisplayCheckFuncIndex;
    }

    public void setDisplayCheckFuncIndex(int displayCheckFuncIndex) {
        DisplayCheckFuncIndex = displayCheckFuncIndex;
    }

    public int getDisplayCheckPatternIndex() {
        return DisplayCheckPatternIndex;
    }

    public void setDisplayCheckPatternIndex(int displayCheckPatternIndex) {
        DisplayCheckPatternIndex = displayCheckPatternIndex;
    }

    public boolean isDisplayCheckStatus() {
        return DisplayCheckStatus;
    }

    public void setDisplayCheckStatus(boolean displayCheckStatus) {
        DisplayCheckStatus = displayCheckStatus;
    }

    //progress
    int [] resDat;

    public int[] getResDat() {
        return resDat;
    }

    public void setResDat(int[] resDat) {
        this.resDat = resDat;
    }

    //
    //AsyncTaskAccumulation
    //
    StatusDescribeExpandableAdapter DisplayCheckAdapter;

    public StatusDescribeExpandableAdapter getDisplayCheckAdapter() {
        return DisplayCheckAdapter;
    }

    public void setDisplayCheckAdapter(StatusDescribeExpandableAdapter displayCheckAdapter) {
        DisplayCheckAdapter = displayCheckAdapter;
    }





    //StopTestPattern method
    SelectRunAdapter mAdapterTestPattern;

    public SelectRunAdapter getmAdapterTestPattern() {
        return mAdapterTestPattern;
    }

    public void setmAdapterTestPattern(SelectRunAdapter mAdapterTestPattern) {
        this.mAdapterTestPattern = mAdapterTestPattern;
    }



    //UpdateTestPattern
    int[] mPatternDat;

    public int[] getmPatternDat() {
        return mPatternDat;
    }

    public void setmPatternDat(int[] mPatternDat) {
        this.mPatternDat = mPatternDat;
    }
}
