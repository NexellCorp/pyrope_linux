package com.samsung.vd.cinemacontrolpanel;

import android.app.Application;
import android.util.Log;

import com.samsung.vd.baseutils.VdPreference;

import java.util.Arrays;
import java.util.Comparator;
import java.util.Locale;

/**
 * Created by doriya on 10/31/16.
 */
public class CinemaInfo extends Application {
    private static final String VD_DTAG = "CinemaInfo";

    //
    //  Configuration Key
    //
    public static final String KEY_INITIALIZE       = "cinema.initialize";
    public static final String KEY_TS_CALIBRATION   = "touch.calibration";
    public static final String KEY_CABINET_NUM      = "cabinet.num";
    public static final String KEY_SCREEN_SAVING    = "screen.saving";
    public static final String KEY_INITIAL_MODE     = "initial.mode";
    public static final String KEY_SCREEN_ROTATE    = "screen.rotate";
    public static final String KEY_MODE_3D          = "mode.3d";

    public static final String KEY_UPDATE_TGAM0     = "update.tgam0";   // 0: skip, 1: eeprom, 2: usb
    public static final String KEY_UPDATE_TGAM1     = "update.tgam1";   // 0: skip, 1: eeprom, 2: usb
    public static final String KEY_UPDATE_DGAM0     = "update.dgam0";   // 0: skip, 1: eeprom, 2: usb
    public static final String KEY_UPDATE_DGAM1     = "update.dgam1";   // 0: skip, 1: eeprom, 2: usb

    public static final String KEY_TREG_0x018B      = "treg.0x018b";    // REG_WIDTH_CONTROL
    public static final String KEY_TREG_0x018C      = "treg.0x018c";    // REG_SYNC_DELAY
    public static final String KEY_TREG_0x018A      = "treg.0x018a";    // REG_SYNC_REVERSE
    public static final String KEY_TREG_0x018D      = "treg.0x018d";    // REG_SACLE
    public static final String KEY_TREG_0x018E      = "treg.0x018e";    // REG_ZERO_SCALE
    public static final String KEY_TREG_0x0192      = "treg.0x0192";    // REG_SEAM_ON
    public static final String KEY_TREG_0x0055      = "treg.0x0055";    // REG_MODULE_ON
    public static final String KEY_TREG_0x0004      = "treg.0x0004";    // REG_XYZ_TO_RGB
    public static final String KEY_TREG_0x0100      = "treg.0x0100";    // REG_LIVE_LOD_EN
    public static final String KEY_TREG_0x011E      = "treg.0x011E";    // REG_LOD_INSERT_EN
    public static final String KEY_PREG_0x0199      = "preg.0x0199";    // REG_RESOLUTION

    //
    //  Register
    //
    public static final int REG_TCON_FLASH_CC               = 0x0044;
    public static final int REG_TCON_CC_MODULE              = 0x0052;
    public static final int REG_TCON_XYZ_TO_RGB             = 0x0004;
    public static final int REG_TCON_SEAM_ON                = 0x0192;

    public static final int REG_TCON_0x018B                 = 0x018B;
    public static final int REG_TCON_0x018C                 = 0x018C;
    public static final int REG_TCON_0x018A                 = 0x018A;
    public static final int REG_TCON_0x018D                 = 0x018D;
    public static final int REG_TCON_0x018E                 = 0x018E;
    public static final int REG_TCON_0x0192                 = 0x0192;
    public static final int REG_TCON_0x0055                 = 0x0055;
    public static final int REG_TCON_0x0004                 = 0x0004;
    public static final int REG_TCON_0x0100                 = 0x0100;
    public static final int REG_TCON_0x011E                 = 0x011E;

    public static final int REG_PFPGA_NUC_EN                = 0x01B0;
    public static final int REG_PFPGA_0x0199                = 0x0199;

    public static final int RET_ERROR   = -1;
    public static final int RET_FAIL    = 0;
    public static final int RET_PASS    = 1;

    public static final String[] KEY_TREG_DEFAULT   = {
        KEY_TREG_0x018B,
        KEY_TREG_0x018C,
        KEY_TREG_0x018A,
        KEY_TREG_0x018D,
        KEY_TREG_0x018E,
        KEY_TREG_0x0192,
        KEY_TREG_0x0055,
        KEY_TREG_0x0004,
        KEY_TREG_0x0100,
        KEY_TREG_0x011E,
    };

    public static final String[] KEY_PREG_DEFAULT   = {
        KEY_PREG_0x0199,
    };

    public static final int TCON_ID_OFFSET = 16;
    public static final int TCON_MODULE_NUM = 24;

    private VdPreference mPrefConfig;
    private CinemaLog mLog;

    private static final String CINEMA_CONFIG = "cinema.config";

    private String mUserGroup = "";
    private String mUserId = "";

    private String mSecureMarriage  = "false";
    private String mSecureBootDone  = "false";
    private String mSecureAlive     = "false";

    private boolean mFirstBoot      = true;
    private boolean mValidEEPRom    = false;

    //
    //  Cabinet Number  = (slave address & 0x7F ) - offset(16)      :: 0, 1, 2, .., 95
    //  Cabinet ID      = (slave address        ) & (0x00 or 0x80 ) :: 16, 17, 18, .., 112
    //
    private byte[] mCabinet = new byte[0];

    //
    //  Default TCON / PFPGA register
    //
    private int[][] mDefaultTReg = {
        { REG_TCON_0x018B, -1 },
        { REG_TCON_0x018C, -1 },
        { REG_TCON_0x018A, -1 },
        { REG_TCON_0x018D, -1 },
        { REG_TCON_0x018E, -1 },
        { REG_TCON_0x0192, -1 },
        { REG_TCON_0x0055, -1 },
        { REG_TCON_0x0004, -1 },
        { REG_TCON_0x0100, -1 },
        { REG_TCON_0x011E, -1 },
    };

    private int[][] mDefaultPReg = {
        { REG_PFPGA_0x0199, -1 },
    };

    //
    //  for debug
    //
    public boolean IsCheckCabinetNum() {
        return false;
    }

    public boolean IsCheckLogin() {
        return false;
    }

    public boolean IsCheckTconBooting() {
        return true;
    }

    public boolean IsCheckTconLvds() {
        return true;
    }

    public boolean IsDetectTamper() {
		return true;
	}

    public boolean IsEnableExit() {
        return false;
    }

    public boolean IsEnableRotate() {
        return true;
    }

    public boolean IsFirstBootAccessEEPRom() {
		return true;
	}

    public int GetBootTime() {
        return 30;
    }

    //
    //  Create Instance
    //
    @Override
    public void onCreate() {
        super.onCreate();

        mPrefConfig = new VdPreference( getApplicationContext(), CINEMA_CONFIG );
        mLog = new CinemaLog( getApplicationContext() );

        UpdateDefaultRegister();
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
    //  System Information
    //
    public boolean IsFirstBoot() {
        return mFirstBoot;
    }

    public void SetFirstBoot(boolean bFirst) {
        mFirstBoot = bFirst;
    }

    public boolean IsValidEEPRom() {
        return mValidEEPRom;
    }

    public void SetValidEEPRom(boolean bValid) {
        mValidEEPRom = bValid;
    }

    public boolean IsMode3D() {
        String strTemp = GetValue( KEY_MODE_3D );
        return (strTemp != null) && strTemp.equals("true");
    }

    public void SetMode3D( boolean bEnable ) {
        SetValue( KEY_MODE_3D, bEnable ? "true" : "false" );
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

        boolean bDuplicate = false;
        for( byte cabinet : mCabinet ) {
            if( cabinet == cabinetId ) {
                bDuplicate = true;
                break;
            }
        }

        if( !bDuplicate ) {
            tmpData[mCabinet.length] = cabinetId;
            mCabinet = Sort( tmpData );
            Log.i(VD_DTAG, String.format(Locale.US, "Add Cabinet Id. ( 0x%02X - port: %d, slave: 0x%02X )",
                    cabinetId, (cabinetId & 0x80) >> 7, (cabinetId & 0x7F)));
        }
        else {
            Log.i(VD_DTAG, String.format(Locale.US, "Warning, Duplicate Cabinet Id. ( 0x%02X - port: %d, slave: 0x%02X )",
                    cabinetId, (cabinetId & 0x80) >> 7, (cabinetId & 0x7F)));
        }
    }

    public void ClearCabinet() {
        mCabinet = new byte[0];
    }

    public byte[] GetCabinet() {
        return mCabinet;
    }

    private byte[] Sort( byte[] src ) {
        Byte[] tmp = new Byte[src.length];
        for( int i = 0; i < src.length; i++ ) {
            tmp[i] = src[i];
        }

        Arrays.sort( tmp, new Comparator<Byte>() {
            @Override
            public int compare(Byte lhs, Byte rhs) {
                byte src1 = lhs;
                byte src2 = rhs;

                if( (src1 & 0x7F) < (src2 & 0x7F) ) {
                    return -1;
                }
                else if( (src1 & 0x7F) > (src2 & 0x7F) ) {
                    return 1;
                }
                else {
                    if( src1 < src2 ) {
                        return 1;
                    }
                    else if( src1 > src2 ) {
                        return -1;
                    }
                }
                return 0;
            }
        });

        byte[] dst = new byte[tmp.length];
        for( int i = 0; i < tmp.length; i++ ) {
            dst[i] = tmp[i];
        }
        return dst;
    }

    //
    //  For Default Register
    //
    public void UpdateDefaultRegister() {
        for( int i = 0; i < KEY_TREG_DEFAULT.length; i++ )
        {
            String strValue = GetValue( KEY_TREG_DEFAULT[i] );
            if( strValue == null )
                continue;

            mDefaultTReg[i][1] = Integer.parseInt(strValue, 10);
        }

        for( int i = 0; i < KEY_PREG_DEFAULT.length; i++ )
        {
            String strValue = GetValue( KEY_PREG_DEFAULT[i] );
            if(strValue == null)
                continue;

            mDefaultPReg[i][1] = Integer.parseInt(strValue, 10);
        }

        Log.i(VD_DTAG, ">>> Current TCON Global Register");
        for( int i = 0; i < KEY_TREG_DEFAULT.length; i++ ) {
            Log.i(VD_DTAG, String.format(">>> reg( 0x%04X ), dat( 0x%04X )", mDefaultTReg[i][0], mDefaultTReg[i][1]));
        }

        Log.i(VD_DTAG, ">>> Current PFPGA Global Register");
        for( int i = 0; i < KEY_PREG_DEFAULT.length; i++ ) {
            Log.i(VD_DTAG, String.format(">>> reg( 0x%04X ), dat( 0x%04X )", mDefaultPReg[i][0], mDefaultPReg[i][1]));
        }
    }

    public int[][] GetDefaultTReg() {
        return mDefaultTReg;
    }

    public int[][] GetDefaultPReg() {
        return mDefaultPReg;
    }
}
