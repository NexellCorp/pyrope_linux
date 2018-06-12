package com.samsung.vd.baseutils;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.util.Log;

import java.util.Locale;

/**
 * Created by doriya on 10/24/16.
 */
public class VdPreference {
    private static final String VD_DTAG = "VdPreference";

    //
    //  If Preference is changed in Native Layer,
    //      Android SharedPreference do not know that.
    //
//    private SharedPreferences mPreference = null;
//
//    public VdPreference(Context context, String strPreference ) {
//        mPreference = context.getSharedPreferences( strPreference, Context.MODE_PRIVATE );
//    }
//
//    public String GetValue( String key ) {
//        return mPreference.getString( key, null );
//    }
//
//    public void SetValue( String key, String value ) {
//        SharedPreferences.Editor editor = mPreference.edit();
//        editor.putString( key, value );
//        editor.apply();
//    }
//
//    public void Remove( String key ) {
//        SharedPreferences.Editor editor = mPreference.edit();
//        editor.remove( key );
//        editor.apply();
//    }

    private String mStrPreference;

    public VdPreference(Context context, String strPreference ) {
        mStrPreference = strPreference;
    }

    public String GetValue( String key ) {
        return NX_PreferenceRead( mStrPreference, key );
    }

    public void SetValue( String key, String value ) {
        NX_PreferenceWrite( mStrPreference, key, value );
    }

    public void Remove( String key ) {
        NX_PreferenceRemove( mStrPreference, key );
    }

    static {
        Log.i( VD_DTAG, "Load JNI Library.");
        System.loadLibrary("nxpreferencendk");
    }

    public native String    NX_PreferenceRead( String config, String key );
    public native void      NX_PreferenceWrite( String config, String key, String value );
    public native void      NX_PreferenceRemove( String config, String key );
}
