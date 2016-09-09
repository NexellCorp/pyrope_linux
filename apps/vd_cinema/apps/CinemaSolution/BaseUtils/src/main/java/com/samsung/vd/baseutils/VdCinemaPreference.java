package com.samsung.vd.baseutils;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * Created by doriya on 8/17/16.
 */
public class VdCinemaPreference {
    private final String VD_DTAG = "VdCinemaPreference";

    private static final String mPrefernceName = "cinema.preference";
    private SharedPreferences mPreference = null;

    public VdCinemaPreference( Context context) {
        mPreference = context.getSharedPreferences( mPrefernceName, Context.MODE_PRIVATE );
    }

    public String GetPreference( String key ) {
        return mPreference.getString( key, null );
    }

    public void SetPrefernece( String key, String value ) {
        SharedPreferences.Editor editor = mPreference.edit();
        editor.putString( key, value );
        editor.apply();
    }
}
