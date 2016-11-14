package com.samsung.vd.cinemacontrolpanel;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import android.widget.ListView;

import com.samsung.vd.baseutils.VdPreference;

/**
 * Created by doriya on 10/31/16.
 */
public class CinemaInfo extends Application {
    private static final String VD_DTAG = "CinemaInfo";

    public static final String KEY_INITIALIZE = "cinema.initialize";
    public static final String KEY_TS_CALIBRATION = "touch.calibration";
    public static final String KEY_CABINET_NUM = "cabinet.num";
    public static final String KEY_SCREEN_SAVING = "screen.saving";
    public static final String KEY_INPUT_SOURCE = "input.source";

    private VdPreference mPrefConfig;
    private CinemaLog mLog;

    private static final String CINEMA_CONFIG = "cinema.config";

    private String mUserGroup = "";
    private String mUserId = "";

    private String mSecureMarriage  = "false";
    private String mSecureBootDone  = "false";
    private String mSecureAlive     = "false";

    @Override
    public void onCreate() {
        super.onCreate();

        mPrefConfig = new VdPreference( getApplicationContext(), CINEMA_CONFIG );
        mLog = new CinemaLog( getApplicationContext() );
    }

    //
    //  Account Information
    //
    public String GetUserGroup() {
        return mUserGroup;
    }

    public void SetUserGroup( String userGroup ) {
        mUserGroup = userGroup;
    }

    public String GetUserId() {
        return mUserId;
    }

    public void SetUserId( String userId ) {
        mUserId = userId;
    }

    //
    //  SecureLink Information
    //
    public String GetSecureMarriage() {
        return mSecureMarriage;
    }

    public void SetSecureMarriage( String secureMarriage ) {
        mSecureMarriage = secureMarriage;
    }

    public String GetSecureBootDone() {
        return mSecureBootDone;
    }

    public void SetSecureBootDone( String secureBootDone ) {
        mSecureBootDone = secureBootDone;
    }

    public String GetSecureAlive() {
        return mSecureAlive;
    }

    public void SetSecureAlive( String secureAlive ) {
        mSecureAlive = secureAlive;
    }

    //
    //  System Log
    //
    public void InsertLog( String msg ) {
        String strAccount;
        if( mUserGroup.equals("") || mUserGroup.equals("root") ) {
            strAccount = String.format("%s", mUserGroup);
        }
        else {
            strAccount = String.format("%s ( %s )", mUserGroup, mUserId );
        }

        mLog.Insert(strAccount, msg);
    }

    public void DeleteLog() {
    }

    //
    //  Configuration Load Set / Get / Remove
    //
    public String GetValue( String key ) {
        return mPrefConfig.GetValue(key);
    }

    public void SetValue( String key, String value ) {
        mPrefConfig.SetValue(key, value);
    }

    public void Remove( String key ) {
        mPrefConfig.Remove(key);
    }
}
