package com.samsung.vd.baseutils;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * Created by doriya on 10/24/16.
 */
public class VdPreference {
    private final String VD_DTAG = "VdPreference";

    private SharedPreferences mPreference = null;

    public VdPreference(Context context, String strPreference ) {
        mPreference = context.getSharedPreferences( strPreference, Context.MODE_PRIVATE );
    }

    public String GetValue( String key ) {
        return mPreference.getString( key, null );
    }

    public void SetValue( String key, String value ) {
        SharedPreferences.Editor editor = mPreference.edit();
        editor.putString( key, value );
        editor.apply();
    }

    public void Remove( String key ) {
        SharedPreferences.Editor editor = mPreference.edit();
        editor.remove( key );
        editor.apply();
    }
}
