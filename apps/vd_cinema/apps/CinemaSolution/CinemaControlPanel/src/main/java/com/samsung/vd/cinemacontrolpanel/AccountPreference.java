package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;

import com.samsung.vd.baseutils.VdPreference;

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

        if (null == mPreference.GetValue(keyPw)) {
            mPreference.SetValue(keyPw, pw);
            return true;
        }

        Log.i(VD_DTAG, "Fail, Add Account.");
        return false;
    }

    //
    //  For Normal User & Not Consider Slot
    //
    public boolean Add(String group, String id, String pw) {
        if (group == null || id == null || pw == null)
            return false;

        for (int i = 0; i < MAX_ACCOUNT_NUM; i++) {
            String keyId = "id." + group + String.valueOf(i);
            String keyPw = "pw." + group + String.valueOf(i);

            if (null == mPreference.GetValue(keyId) && null == mPreference.GetValue(keyPw)) {
                mPreference.SetValue(keyId, id);
                mPreference.SetValue(keyPw, pw);

                return true;
            }
        }

        Log.i(VD_DTAG, "Fail, Add Account.");
        return false;
    }

    //
    //  For Normal User & Consider Slot
    //
    public boolean Add(String group, int index, String id, String pw) {
        if (group == null || id == null || pw == null)
            return false;

        if (index < 0 || index >= MAX_ACCOUNT_NUM)
            return false;

        String keyId = "id." + group + String.valueOf(index);
        String keyPw = "pw." + group + String.valueOf(index);

        if (null == mPreference.GetValue(keyId) && null == mPreference.GetValue(keyPw)) {
            mPreference.SetValue(keyId, id);
            mPreference.SetValue(keyPw, pw);

            return true;
        }

        Log.i(VD_DTAG, "Fail, Add Account.");
        return false;
    }

    //
    //  Read Account Information
    //
    public String ReadId(String group, int index ) {
        if (group == null )
            return null;

        if (index < 0 || index >= MAX_ACCOUNT_NUM)
            return null;

        String keyId = "id." + group + String.valueOf(index);
        return mPreference.GetValue(keyId);
    }

    public String ReadPw(String group, int index ) {
        if (group == null )
            return null;

        if( index < 0 || index >= MAX_ACCOUNT_NUM)
            return null;

        String keyPw = "pw." + group + String.valueOf(index);
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
    public void Remove(String group, int index) {
        String keyId = "id." + group + String.valueOf(index);
        String keyPw = "pw." + group + String.valueOf(index);

        mPreference.Remove(keyId);
        mPreference.Remove(keyPw);
    }

    //
    //  Confirm Login
    //
    public boolean Confirm(String group, String pw) {
        if( group.equals(GROUP_ROOT) ) {
            String keyPw = "pw." + group;

            String strTemp = mPreference.GetValue( keyPw );
            if( null != strTemp && strTemp.equals(pw) ) {
                Log.i( VD_DTAG, "Success, Login." );
                return true;
            }

            Log.i( VD_DTAG, "Fail, Login." );
            return false;
        }
        else {
            for( int i = 0; i < MAX_ACCOUNT_NUM; i++ )
            {
                String keyPw = "pw." + group + String.valueOf(i);

                String strTemp = mPreference.GetValue( keyPw );
                if( null != strTemp && strTemp.equals(pw) ) {
                    Log.i( VD_DTAG, "Success, Login." );
                    return true;
                }
            }
        }

        Log.i( VD_DTAG, "Fail, Login." );
        return false;
    }
}
