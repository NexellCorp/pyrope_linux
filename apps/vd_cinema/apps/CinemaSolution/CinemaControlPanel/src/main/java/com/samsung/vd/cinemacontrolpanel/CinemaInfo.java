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
    public static final String KEY_TREG_0x018D      = "treg.0x018d";    // REG_SCALE
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
    public static final int REG_TCON_PITCH_INFO             = 0x000E;   // P2.5: d'25xxx, P3.3: d'33xxx
    public static final int REG_TCON_WIDTH_CONTROL          = 0x018B;
    public static final int REG_TCON_SYNC_DELAY             = 0x018C;
    public static final int REG_TCON_SYNC_REVERSE           = 0x018A;
    public static final int REG_TCON_SCALE                  = 0x018D;
    public static final int REG_TCON_ZERO_SCALE             = 0x018E;
    public static final int REG_TCON_LIVE_LOD_EN            = 0x0100;
    public static final int REG_TCON_LOD_INSERT_EN          = 0x011E;
    public static final int REG_TCON_XYZ_TO_RGB             = 0x0004;
    public static final int REG_TCON_CC_CABINET             = 0x0052;
    public static final int REG_TCON_CC_PIXEL               = 0x0044;
    public static final int REG_TCON_SEAM_ON                = 0x0192;
    public static final int REG_TCON_CC_MODULE              = 0x0055;
    public static final int REG_TCON_SCAN_MODE              = 0x0120;

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

    public static final int REG_PFPGA_PF_MODEL              = 0x01F0;   // P2.5: 0      , P3.3: 1
    public static final int REG_PFPGA_PF_SCREEN_SEL         = 0x01C8;   // 01: Left, 10: Right, 11: Both
    public static final int REG_PFPGA_NUC_EN                = 0x01B0;
    public static final int REG_PFPGA_RESOLUTION            = 0x0199;

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

    public static final int TCON_BASE_OFFSET = 16;
    public static final int TCON_P33_RIGHT_OFFSET = 200;
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
    //  Cabinet Number  = cabinet number for display ( 0, 1, 2, 3 ... )
    //  Cabinet Id      = cabinet slave address with i2c port information.
    //  Cabinet Slave   = cabinet slave address without i2c port information.
    //  Cabinet Port    = cabinet i2c port
    //
    public static final int SCREEN_TYPE_P25     = 0;
    public static final int SCREEN_TYPE_P33     = 1;

    public static final int PORT_L = 0;
    public static final int PORT_R = 1;

    private int mScreenType = SCREEN_TYPE_P25;

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
    public boolean IsCheckInitial() {
        return false;
    }

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

    public boolean IsCheckScreenInJni() {
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
    public int GetScreenType() {
        return mScreenType;
    }

    public byte[] GetCabinet() {
        return mCabinet;
    }

    public boolean IsCabinetValidPort( int port ) {
        for( byte id : mCabinet ) {
            if( port == ((id & 0x80) >> 7) )
                return true;
        }
        return false;
    }

    public byte GetCabinetId( int number ) {
        byte id = 0x00;
        switch( mScreenType ) {
            case SCREEN_TYPE_P25:
                id = ((number % TCON_BASE_OFFSET) < 8) ? (byte)(number + TCON_BASE_OFFSET) : (byte)((number | 0x80) + TCON_BASE_OFFSET);
                break;
            case SCREEN_TYPE_P33:
                id = (number < TCON_P33_RIGHT_OFFSET) ? (byte)(number + TCON_BASE_OFFSET) : (byte)((number + TCON_BASE_OFFSET) | 0x80);
                break;
            default:
                break;
        }
        return id;
    }

    public int GetCabinetPort( byte id ) {
        return (id & 0x80) >> 7;
    }

    public byte GetCabinetSlave( byte id ) {
        return (byte)(id & 0x7F);
    }

    public int GetCabinetNumber( byte id ) {
        int cabinet = -1;

        switch( mScreenType ) {
            case SCREEN_TYPE_P25:
                cabinet = ((id & 0x7F) - TCON_BASE_OFFSET);
                break;
            case SCREEN_TYPE_P33:
                cabinet = ((id & 0x7F) - TCON_BASE_OFFSET) + (((id & 0x80) >> 7) * TCON_P33_RIGHT_OFFSET);
                break;
            default:
                break;
        }

        return cabinet;
    }

    public void AddCabinet() {
        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

        mCabinet = new byte[0];

        for (int i = 0; i < 255; i++) {
            // Read Slave Address
            if ((i & 0x7F) < 0x10)
                continue;

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte) i});
            if (result == null || result.length == 0)
                continue;

            if (0x01 != result[0])
                continue;

            boolean bDuplicate = false;
            for (byte cabinet : mCabinet) {
                if (cabinet == i) {
                    bDuplicate = true;
                    break;
                }
            }

            if (!bDuplicate) {
                mCabinet = Arrays.copyOf(mCabinet, mCabinet.length + 1);
                mCabinet[mCabinet.length - 1] = (byte) i;
            }
        }

        if( 0 < mCabinet.length ) {
            //  Cabinet Sorting
            Byte[] cabinet = new Byte[mCabinet.length];
            for( int i = 0; i < mCabinet.length; i++ ) {
                cabinet[i] = mCabinet[i];
            }

            Arrays.sort(
                    cabinet,
                    (mScreenType == SCREEN_TYPE_P33) ? mComparatorScreenTypeP33 : mComparatorScreenTypeP25
            );

            for( int i = 0; i < cabinet.length; i++ ) {
                mCabinet[i] = cabinet[i];
            }
        }
    }

    public void ShowCabinet() {
        for( byte id : mCabinet ) {
            Log.i(VD_DTAG, String.format(Locale.US, "Add Cabinet. ( cabinet: %d, port: %d, slave: 0x%02x )",
                    GetCabinetNumber(id), GetCabinetPort(id), GetCabinetSlave(id)));
        }
    }

    private Comparator<Byte> mComparatorScreenTypeP25 = new Comparator<Byte>() {
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
    };

    private Comparator<Byte> mComparatorScreenTypeP33 = new Comparator<Byte>() {
        @Override
        public int compare(Byte lhs, Byte rhs) {
            byte src1 = lhs;
            byte src2 = rhs;

            if( ((src1 & 0x80) >> 7) == ((src2 & 0x80) >> 7) ) {
                if( src1 < src2 ) {
                    return -1;
                }
                else if( src1 > src2 ) {
                    return 1;
                }
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
    };

    //
    //  For Check ScreenType
    //
    public void CheckScreenType() {
        if( IsCheckScreenInJni() ) {
            Log.i(VD_DTAG, ">>> Start CheckScreenType().");
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] result;

            result = ctrl.Send(NxCinemaCtrl.CMD_PLATFORM_SCREEN_TYPE, null);
            if( result == null || result.length == 0 ) {
                Log.i(VD_DTAG, "Fail, CheckScreen.");
                return;
            }

            mScreenType = (int)result[0];
        }
        else {
            //
            //  Not Use This Function
            //
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] result, inData;
            byte[] reg, dat;
            int defScreenSel;

            {
                reg = ctrl.IntToByteArray(REG_PFPGA_PF_SCREEN_SEL, NxCinemaCtrl.FORMAT_INT16);
                result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_READ, reg);
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "fail, pfpga register read.( reg: 0x%04x )", NxCinemaCtrl.CMD_PFPGA_REG_READ));
                    return;
                }

                defScreenSel = ctrl.ByteArrayToInt(result);

                reg = ctrl.IntToByteArray(REG_PFPGA_PF_SCREEN_SEL, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0003, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
            }

            for( int j = 0; j < 2; j++ ) {
                reg = ctrl.IntToByteArray(REG_PFPGA_PF_MODEL, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(j, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                for( int i = 0; i < 255; i++ ) {
                    if( (i & 0x7F) < 0x10 )
                        continue;

                    result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
                    if (result == null || result.length == 0)
                        continue;

                    if( 0x01 != result[0] )
                        continue;

                    Log.i(VD_DTAG, String.format(Locale.US, ">>> Check Screen Type in Cabinet #%d. ( port: %d, slave: 0x%02x )",
                            GetCabinetNumber((byte)i), GetCabinetPort((byte)i), GetCabinetSlave((byte)i)));

                    inData = new byte[] { (byte)i };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(REG_TCON_PITCH_INFO, NxCinemaCtrl.FORMAT_INT16));
                    result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );

                    if( result == null || result.length != NxCinemaCtrl.FORMAT_INT32 )
                        continue;

                    int type = ctrl.ByteArrayToInt(result) / 1000;
                    switch( type ) {
                        case 33:
                            Log.i(VD_DTAG, String.format(">>> Screen Type is P3.3 ( %d )", ctrl.ByteArrayToInt(result)));

                            reg = ctrl.IntToByteArray(REG_PFPGA_PF_MODEL, NxCinemaCtrl.FORMAT_INT16);
                            dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
                            inData = ctrl.AppendByteArray(reg, dat);
                            ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                            reg = ctrl.IntToByteArray(REG_PFPGA_PF_SCREEN_SEL, NxCinemaCtrl.FORMAT_INT16);
                            dat = ctrl.IntToByteArray(0x0003, NxCinemaCtrl.FORMAT_INT16);
                            inData = ctrl.AppendByteArray(reg, dat);
                            ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                            mScreenType = SCREEN_TYPE_P33;
                            break;

                        default:
                            Log.i(VD_DTAG, String.format(">>> Screen Type is P2.5 ( %d )", ctrl.ByteArrayToInt(result)));

                            reg = ctrl.IntToByteArray(REG_PFPGA_PF_MODEL, NxCinemaCtrl.FORMAT_INT16);
                            dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                            inData = ctrl.AppendByteArray(reg, dat);
                            ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                            reg = ctrl.IntToByteArray(REG_PFPGA_PF_SCREEN_SEL, NxCinemaCtrl.FORMAT_INT16);
                            dat = ctrl.IntToByteArray(defScreenSel, NxCinemaCtrl.FORMAT_INT16);
                            inData = ctrl.AppendByteArray(reg, dat);
                            ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                            mScreenType = SCREEN_TYPE_P25;
                            break;
                    }
                    return;
                }
            }

            Log.i(VD_DTAG, ">>> Unknown Screen Type. --> Set Default Screen Type P2.5");
            {
                reg = ctrl.IntToByteArray(REG_PFPGA_PF_MODEL, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                reg = ctrl.IntToByteArray(REG_PFPGA_PF_SCREEN_SEL, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(defScreenSel, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                mScreenType = SCREEN_TYPE_P25;
            }
        }
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
