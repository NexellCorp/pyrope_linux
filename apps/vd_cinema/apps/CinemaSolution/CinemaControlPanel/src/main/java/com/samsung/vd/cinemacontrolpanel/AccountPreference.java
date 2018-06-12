package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;

import com.samsung.vd.baseutils.VdPreference;

import java.util.Locale;

/**
 * Created by doriya on 10/24/16.
 */
public class AccountPreference {
    private final String VD_DTAG = "AccountPreference";

    private VdPreference mPreference;

    private static final String CINEMA_ACCOUNT = "cinema.account";
    private static final int MAX_ACCOUNT_NUM = 3;

    public static final String GROUP_ROOT = "root";
    public static final String GROUP_SERVICE = "service";
    public static final String GROUP_OPERATOR = "operator";
    public static final String GROUP_CALIBRATOR = "calibrator";

    public AccountPreference(Context context) {
        mPreference = new VdPreference(context, CINEMA_ACCOUNT);
    }

    //
    //  For Root User
    //
    public boolean Add(String group, String pw) {
        if (group == null || pw == null)
            return false;

        String keyPw = "pw." + group;

        mPreference.SetValue(keyPw, pw);

        Log.i(VD_DTAG, String.format( Locale.US, "-----> [%s] password( %s )", group, pw) );
        return true;
    }

    //
    //  For Normal User
    //
    public boolean Add(String group, int idx, String id, String pw) {
        if (group == null || id == null || pw == null)
            return false;

        if (idx < 0 || idx >= MAX_ACCOUNT_NUM)
            return false;

        String keyId = "id." + group + String.valueOf(idx);
        String keyPw = "pw." + group + String.valueOf(idx);

        mPreference.SetValue(keyId, id);
        mPreference.SetValue(keyPw, pw);

        Log.i(VD_DTAG, String.format( Locale.US, "-----> [%s-%d] id( %s ), password( %s )", group, idx, id, pw) );
        return true;
    }

    //
    //  Read Account Information
    //
    public String ReadId(String group, int idx ) {
        if (group == null )
            return "";

        if ( group.equals(GROUP_ROOT) )
            return "";

        if (idx < 0 || idx >= MAX_ACCOUNT_NUM)
            return "";

        String keyId = "id." + group + String.valueOf(idx);
        return mPreference.GetValue(keyId);
    }

    public String ReadPw(String group, int idx ) {
        if (group == null )
            return "";

        if( idx < 0 || idx >= MAX_ACCOUNT_NUM)
            return "";

        String keyPw = "pw." + group + String.valueOf(idx);
        return mPreference.GetValue(keyPw);
    }

    //
    //  For Root User
    //
    public void Remove(String group) {
        String strPw = "pw." + group;
        mPreference.Remove(strPw);
    }

    //
    //  For Normal User
    //
    public void Remove(String group, int idx) {
        String keyId = "id." + group + String.valueOf(idx);
        String keyPw = "pw." + group + String.valueOf(idx);

        mPreference.Remove(keyId);
        mPreference.Remove(keyPw);
    }

    //
    //  Confirm Login
    //
    public int Confirm(String group, String pw) {
        if( group.equals(GROUP_ROOT) ) {
            String keyPw = "pw." + group;

            String strTemp = mPreference.GetValue( keyPw );
            if( null != strTemp && strTemp.equals(pw) ) {
                Log.i( VD_DTAG, "Success, Login." );
                return 0;
            }

            Log.i( VD_DTAG, "Fail, Login." );
            return -1;
        }
        else {
            for( int i = 0; i < MAX_ACCOUNT_NUM; i++ )
            {
                String keyPw = "pw." + group + String.valueOf(i);

                String strTemp = mPreference.GetValue( keyPw );
                if( null != strTemp && strTemp.equals(pw) ) {
                    Log.i( VD_DTAG, "Success, Login." );
                    return i;
                }
            }
        }

        Log.i( VD_DTAG, "Fail, Login." );
        return -1;
    }
}
