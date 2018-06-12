package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static android.os.SystemClock.elapsedRealtime;

public class LoginActivity extends AppCompatActivity {
    private static final String VD_DTAG = "LoginActivity";

    private Spinner mSpinnerAccount;

    private EditText mEditPassword;
    private EditText mEditPassword1;
    private EditText mEditPassword2;
    private EditText mEditPassword3;

    private boolean mRescueMode = false;

    private AccountPreference mAccountPreference;
    private String[] mStrAccount;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();

        new AsyncTaskCheckCabinet().execute();
        if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
            return;
        }

        setContentView(R.layout.activity_login);

        //
        //  Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Login" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        //
        //  For Debugging
        //
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                startActivity( new Intent(v.getContext(), InitialActivity.class) );
//                overridePendingTransition(0, 0);
//                finish();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mService.TurnOff();
            }
        });

        if( !((CinemaInfo)getApplicationContext()).IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !((CinemaInfo)getApplicationContext()).IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        //
        //  Configuration Status Bar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());
        mStrAccount = new String[] {
            AccountPreference.GROUP_ROOT,
            AccountPreference.GROUP_SERVICE,
            AccountPreference.GROUP_CALIBRATOR,
            AccountPreference.GROUP_OPERATOR };

        mSpinnerAccount = (Spinner)findViewById(R.id.spinnerAccount);
        ArrayAdapter<String> adapterAccount = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrAccount );
        mSpinnerAccount.setAdapter(adapterAccount);
        mSpinnerAccount.setSelection( 3 );

        Button btnLoginNormal = (Button)findViewById(R.id.btnLoginNormal);
        btnLoginNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginRescue).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginNormal).setVisibility(View.VISIBLE);

                mEditPassword1.setText("");
                mEditPassword2.setText("");
                mEditPassword3.setText("");

                mRescueMode = false;
            }
        });

        Button btnLoginRescue = (Button)findViewById(R.id.btnLoginRescue);
        btnLoginRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginNormal).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginRescue).setVisibility(View.VISIBLE);

                mEditPassword.setText("");

                mRescueMode = true;
            }
        });

        Button btnAccpetNormal = (Button)findViewById(R.id.btnLoginNormalAccept);
        btnAccpetNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    startActivity( new Intent(getApplicationContext(), TopActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                }
            }
        });

        Button btnAcceptRescue = (Button)findViewById(R.id.btnLoginRescueAccept);
        btnAcceptRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    startActivity( new Intent(getApplicationContext(), TopActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                }
            }
        });

        //
        //  IMM Handler
        //
        mEditPassword = (EditText)findViewById(R.id.editLoginPw);
        mEditPassword1 = (EditText)findViewById(R.id.editLoginPw1);
        mEditPassword2 = (EditText)findViewById(R.id.editLoginPw2);
        mEditPassword3 = (EditText)findViewById(R.id.editLoginPw3);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditPassword.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword1.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword2.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword3.getWindowToken(), 0);
            }
        });
    }

    private boolean CheckAccount() {
        if( !mRescueMode ) {
            //  Normal Login Mode
            Log.i(VD_DTAG, "Normal Mode.");
            String strGroup = mSpinnerAccount.getSelectedItem().toString();
            String strPassword = mEditPassword.getText().toString();

            if( strPassword.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            int index = mAccountPreference.Confirm(strGroup, strPassword);
            if( 0 > index ) {
                ShowMessage("Please check password.");

                mEditPassword.setText("");

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(strGroup);
            info.SetUserId(( mAccountPreference.ReadId(strGroup, index) ));
            info.InsertLog( "Login." );
        }
        else {
            Log.i(VD_DTAG, "Rescue Mode.");
            //  Rescue Login Mode
            String strPassword1 = mEditPassword1.getText().toString();
            String strPassword2 = mEditPassword2.getText().toString();
            String strPassword3 = mEditPassword3.getText().toString();

            if( strPassword1.equals("") ||
                strPassword2.equals("") ||
                strPassword3.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            if( 0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword1) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword2) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword3) ) {
                ShowMessage("Please check password.");

                mEditPassword1.setText(null);
                mEditPassword2.setText(null);
                mEditPassword3.setText(null);

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(AccountPreference.GROUP_ROOT);
            info.InsertLog( "Login." );
        }

        return true;
    }

    //
    //
    //
    private class AsyncTaskCheckCabinet extends AsyncTask<Void, Void, Void> {
        private CinemaInfo mCinemaInfo = (CinemaInfo)getApplicationContext();
        private int mInitMode =
                (((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE) == null) ?
                0 : Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE));

        @Override
        protected Void doInBackground(Void... params) {
            //
            //
            //

            boolean bFirstBoot = ((SystemClock.elapsedRealtime() / 1000) < ((CinemaInfo)getApplicationContext()).GetBootTime());
            if( bFirstBoot ) {
                try {
                    while( true ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "elapsed time : %d sec", SystemClock.elapsedRealtime() / 1000));
                        Thread.sleep(1000);

                        if( (SystemClock.elapsedRealtime() / 1000) > ((CinemaInfo)getApplicationContext()).GetBootTime() )
                            break;
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            //
            //  Detection Cabinet
            //
            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < 0x10 )
                    continue;

                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
                if (result == null || result.length == 0)
                    continue;

                if( 0 > result[0] )
                    continue;

                mCinemaInfo.AddCabinet( (byte)i );
                Log.i(VD_DTAG, String.format(Locale.US, "Add Cabinet ( Cabinet : %d, port : %d, slave : 0x%02x )", (i & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (i & 0x80) >> 7, i & 0x7F ));
            }

            mCinemaInfo.SortCabinet();

            byte[] cabinet = mCinemaInfo.GetCabinet();

            //
            //  Display Version
            //
            byte[] napVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
            byte[] sapVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
            byte[] srvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
            byte[] clnVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
            byte[] pfpgaVersion = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );

            Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );

            Log.i(VD_DTAG, ">> Version Information");
            Log.i(VD_DTAG, String.format(Locale.US, "-. Application : %s", new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date)));
            Log.i(VD_DTAG, String.format(Locale.US, "-. N.AP        : %s", (napVersion != null && napVersion.length != 0) ? new String(napVersion) : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. S.AP        : %s", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion) : "Unknown"));

            for( byte value : cabinet ) {
                byte[] tconVersion = ctrl.Send(NxCinemaCtrl.CMD_TCON_VERSION, null);

                int msbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                int lsbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                Log.i(VD_DTAG, String.format(Locale.US, "-. TCON #%02d    : %05d - %05d", (value & 0x7F) - CinemaInfo.TCON_ID_OFFSET, msbVersion, lsbVersion));
            }
            Log.i(VD_DTAG, String.format(Locale.US, "-. PFPGA       : %s", (pfpgaVersion != null && pfpgaVersion.length != 0) ? String.format(Locale.US, "%05d", ctrl.ByteArrayToInt(pfpgaVersion)) : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Server  : %s", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion) : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Client  : %s", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion) : "Unknown"));

            Log.i(VD_DTAG, String.format(Locale.US, "-. TCON Booting Check : %b", mCinemaInfo.IsCheckTconBooting()));


            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : cabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( !mCinemaInfo.IsFirstBootAccessEEPRom() ) {
                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, "Unknown Error.");
                    return null;
                }

                if( (int)result[0] == 0xFF ) Log.i(VD_DTAG, ">> Fail, EEPROM Read.");
                if( (int)result[0] == 0x01 ) Log.i(VD_DTAG, ">> Update is successful.");
                if( (int)result[0] == 0x00 ) Log.i(VD_DTAG, ">> Update is not needed.");
            }

            if( bFirstBoot ) {
                //
                //  Read EEPROM
                //
                if( mCinemaInfo.IsFirstBootAccessEEPRom() ) {
                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
                    if( result == null || result.length == 0 ) {
                        Log.i(VD_DTAG, "Unknown Error.");
                        return null;
                    }

                    if( (int)result[0] == 0xFF ) Log.i(VD_DTAG, ">> Fail, EEPROM Read.");
                    if( (int)result[0] == 0x01 ) Log.i(VD_DTAG, ">> Update is successful.");
                    if( (int)result[0] == 0x00 ) Log.i(VD_DTAG, ">> Update is not needed.");
                }

                //
                //  Check TCON Booting Status
                //
                if( mCinemaInfo.IsCheckTconBooting() ) {
                    boolean bTconBooting = true;
                    for( byte id : cabinet ) {
                        byte[] result;
                        result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                        if (result == null || result.length == 0) {
                            Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / port: %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id));
                            continue;
                        }

                        if( result[0] == 0 ) {
                            Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / port: %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id, result[0] ));
                            bTconBooting = false;
                        }
                    }

                    if( !bTconBooting ) {
                        Log.i(VD_DTAG, "Fail, TCON booting.");
                        return null;
                    }
                }

                //
                //  PFPGA Mute on
                //
                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

                //
                //  Parse P_REG.txt
                //
                String[] resultPath;
                int enableUniformity = 0;
                int[] enableGamma = {0, 0, 0, 0};

                resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
                for( String file : resultPath ) {
                    ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                    if( info.Parse( file ) ) {
                        enableUniformity = info.GetEnableUpdateUniformity(mInitMode);
                        for( int i = 0; i < info.GetRegister(mInitMode).length; i++ ) {
                            byte[] reg = ctrl.IntToByteArray(info.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                            byte[] data = ctrl.IntToByteArray(info.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                            byte[] inData = ctrl.AppendByteArray(reg, data);

                            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                        }
                    }
                }

                //
                //  Auto Uniformity Correction Writing
                //
                resultPath = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
                for( String file : resultPath ) {
                    LedUniformityInfo info = new LedUniformityInfo();
                    if( info.Parse( file ) ) {
                        if( enableUniformity == 0 ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update Uniformity. ( %s )", file ));
                            continue;
                        }

                        byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                    }
                }

                //
                //  Parse T_REG.txt
                //
                ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
                resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
                for( String file : resultPath ) {
                    tconEEPRomInfo.Parse(file);
                }

                if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
                    enableGamma = tconEEPRomInfo.GetEnableUpdateGamma(mInitMode);
                    for( int i = 0; i < tconEEPRomInfo.GetRegister(mInitMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(tconEEPRomInfo.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(tconEEPRomInfo.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }

                ConfigTconInfo tconUsbInfo = new ConfigTconInfo();
                resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
                for( String file : resultPath ) {
                    tconUsbInfo.Parse(file);
                }

                if( (10 <= mInitMode) && ((mInitMode-10) < tconUsbInfo.GetModeNum())) {
                    enableGamma = tconUsbInfo.GetEnableUpdateGamma(mInitMode-10);
                    for( int i = 0; i < tconUsbInfo.GetRegister(mInitMode-10).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(tconUsbInfo.GetRegister(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(tconUsbInfo.GetData(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }

                //
                //  Write Gamma
                //
                if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update EEPROM Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }
                }

                if( (10 <= mInitMode) ) {
                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update EEPROM Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }

                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 2) ||
                                    (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 2) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 2) ||
                                    (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 2) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update USB Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }
                }

                //
                //  Update Gamma Status
                //
                mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
                mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
                mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
                mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

                //
                //  SW Reset
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

                //
                //  Set Global Register
                //
                {
                    int [][] treg = mCinemaInfo.GetDefaultTReg();
                    for( int i = 0; i < treg.length; i++ ) {
                        if( treg[i][1] == -1 ) {
                            continue;
                        }

                        byte[] reg = ctrl.IntToByteArray(treg[i][0], NxCinemaCtrl.FORMAT_INT16);
                        byte[] dat = ctrl.IntToByteArray(treg[i][1], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, dat);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);

                        if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }

                    int [][] preg = mCinemaInfo.GetDefaultPReg();
                    for( int i = 0; i < preg.length; i++ ) {
                        if( preg[i][1] == -1 ) {
                            continue;
                        }
                        byte[] reg = ctrl.IntToByteArray(preg[i][0], NxCinemaCtrl.FORMAT_INT16);
                        byte[] dat = ctrl.IntToByteArray(preg[i][1], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, dat);

                        ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
                    }
                }

                //
                //  PFPGA Mute off
                //
                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

                //
                //  TCON Initialize
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x09});
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x89});
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "Cabinet Check Start.");
            CinemaLoading.Show( LoginActivity.this );
            mCinemaInfo.ClearCabinet();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
                startActivity( new Intent(getApplicationContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Cabinet Check Done.");

            if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
                finish();
            }
        }
    }

    //
    //  For Internal Toast Message
    //
    private static Toast mToast;

    private void ShowMessage( String strMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_SHORT);

        mToast.setText(strMsg);
        mToast.show();
    }

    //
    //  For Screen Rotation
    //
    private void SetScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        switch( Integer.parseInt(orientation) ) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            default:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
        }
    }

    private void ChangeScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        int curRotate;
        int prvRotate = Integer.parseInt(orientation);
        switch (prvRotate) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            default:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_ROTATE, String.valueOf(curRotate));
    }

    //
    //  For ScreenSaver
    //
    private CinemaService mService = null;
    private boolean mServiceRun = false;

    @Override
    protected void onStart() {
        super.onStart();

        Intent intent = new Intent(this, CinemaService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();

        if( mServiceRun ) {
            unbindService(mConnection);
            mServiceRun = false;
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean isOn = false;
        if( mService != null ) {
            isOn = mService.IsOn();
            mService.RefreshScreenSaver();
        }

        return !isOn || super.dispatchTouchEvent(ev);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            CinemaService.LocalBinder binder = (CinemaService.LocalBinder)service;
            mService = binder.GetService();
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };

}
