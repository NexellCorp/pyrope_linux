package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import android.content.Context;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.LedPosAdapter;

/**
 * Created by spjun on 18. 4. 13.
 */

public class StatusDetailAdapterAsyncParam {
    //
    //AsyncTaskLedPos
    //
    LedPosAdapter mAdapter;
    byte mId;
    Context context;

    public LedPosAdapter getmAdapter() {
        return mAdapter;
    }

    public void setmAdapter(LedPosAdapter mAdapter) {
        this.mAdapter = mAdapter;
    }

    public byte getmId() {
        return mId;
    }

    public void setmId(byte mId) {
        this.mId = mId;
    }

    public Context getContext() {
        return context;
    }

    public void setContext(Context context) {
        this.context = context;
    }
}
