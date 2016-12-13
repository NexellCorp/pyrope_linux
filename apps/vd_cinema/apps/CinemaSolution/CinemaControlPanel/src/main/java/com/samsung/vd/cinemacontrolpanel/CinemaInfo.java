package com.samsung.vd.cinemacontrolpanel;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import android.widget.ListView;

import com.samsung.vd.baseutils.VdPreference;

import java.util.Arrays;
import java.util.Comparator;

/**
 * Created by doriya on 10/31/16.
 */
public class CinemaInfo extends Application {
    private static final String VD_DTAG = "CinemaInfo";

    public static final String KEY_INITIALIZE       = "cinema.initialize";
    public static final String KEY_TS_CALIBRATION   = "touch.calibration";
    public static final String KEY_CABINET_NUM      = "cabinet.num";
    public static final String KEY_SCREEN_SAVING    = "screen.saving";
    public static final String KEY_INPUT_SOURCE     = "input.source";
    public static final String KEY_INPUT_RESOLUTION = "input.resolution";
    public static final String KEY_MASTERING_MODE   = "mastering.mode";

    public static final int OFFSET_TCON = 16;

    private VdPreference mPrefConfig;
    private CinemaLog mLog;

    private static final String CINEMA_CONFIG = "cinema.config";

    private String mUserGroup = "";
    private String mUserId = "";

    private String mSecureMarriage  = "false";
    private String mSecureBootDone  = "false";
    private String mSecureAlive     = "false";

    private byte[] mCabinet = new byte[0];

    //
    //  for debug
    //
    public boolean IsCheckCabinet() {
        return false;
    }

    public boolean IsCheckLogin() {
        return false;
    }

    public boolean IsDetectTamper() { return true; }

    public boolean IsStandAlone() {
        return true;
    }

    //
    //  Create Instance
    //
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
        Log.i(VD_DTAG, String.format("[ %s ] %s", strAccount, msg ));
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

    //
    //  For TCON Index
    //
    public void AddCabinet( byte cabinetId ) {
        byte[] tmpData = Arrays.copyOf(mCabinet, mCabinet.length + 1);
        tmpData[mCabinet.length] = cabinetId;
        mCabinet = tmpData;
    }

    public void ClearCabinet() {
        mCabinet = new byte[0];
    }

    public byte[] GetCabinet() {
        return mCabinet;
    }

    public void SortCabinet() {
        Byte[] tmpData = new Byte[mCabinet.length];
        for( int i = 0; i < tmpData.length; i++ ) {
            tmpData[i] = mCabinet[i];
        }

        Arrays.sort( tmpData, new Comparator<Byte>() {
            @Override
            public int compare(Byte lhs, Byte rhs) {
                byte srcData1 = lhs;
                byte srcData2 = rhs;

                if( (srcData1 & 0x7F) < (srcData2 & 0x7F) ) {
                    return -1;
                }
                else if( (srcData1 & 0x7F) > (srcData2 & 0x7F) ) {
                    return 1;
                }
                else {
                    if( srcData1 < srcData2 ) {
                        return 1;
                    }
                    else if( srcData1 > srcData2 ) {
                        return -1;
                    }
                }
                return 0;
            }
        });

        mCabinet = new byte[tmpData.length];
        for(int i = 0; i < tmpData.length; i++) {
            mCabinet[i] = tmpData[i];
        }
    }
}
