package com.samsung.vd.baseutils;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * Created by doriya on 10/24/16.
 */
public class VdPreference {
    private final String VD_DTAG = "VdPreference";

    //
    //  Cinema Preference Part
    //
    public static final String CINEMA_PREFERENCE = "cinema.preference";

    public static final String KEY_TS_CALIBRATION = "touch.calibration";
    public static final String KEY_CABINET_NUM   = "cabinet.num";
    public static final String KEY_SCREEN_SAVING = "screen.saving";
    public static final String KEY_INPUT_SOURCE  = "input.source";
    
    private SharedPreferences mPreference = null;

    public VdPreference(Context context, String strPrefrence ) {
        mPreference = context.getSharedPreferences( strPrefrence, Context.MODE_PRIVATE );
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
