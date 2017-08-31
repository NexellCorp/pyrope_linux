package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends AppCompatActivity {
    private final String VD_DTAG = "TopActivity";

    private byte[]  mCabinet;

    private Spinner mSpinnerInitialValue;
    private Button mBtnUpdateInitialValue;
    private Button mBtnApplyInitialValue;
    private TextView mTextInitModeCurrent;
    private TextView mTextInitModeDescription;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if( intent.getAction().equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                String[] resultPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
                String[] resultUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
                String[] resultTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);

                if( (resultPfpga != null && resultPfpga.length != 0) ||
                    (resultUniformity != null && resultUniformity.length != 0) ||
                    (resultTcon != null && resultTcon.length != 0) ||
                    (resultGamma != null && resultGamma.length != 0) ) {

                    mBtnUpdateInitialValue.setEnabled(true);

                    if( resultPfpga != null ) for(String file : resultPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultUniformity != null ) for(String file : resultUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultTcon != null ) for(String file : resultTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultGamma != null ) for(String file : resultGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                }
            }
            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnUpdateInitialValue.setEnabled(false);
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        IntentFilter filter = new IntentFilter();
        filter.addAction( Intent.ACTION_MEDIA_MOUNTED );
        filter.addAction( Intent.ACTION_MEDIA_EJECT );
        filter.addDataScheme("file");

        registerReceiver( mBroadcastReceiver, filter );
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver( mBroadcastReceiver );
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
        setContentView(R.layout.activity_top);

        //
        //  Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutTop ));
        titleBar.SetTitle( "Cinema LED Display System - Top Menu" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ((CinemaInfo)getApplicationContext()).InsertLog("Logout.");

                startActivity( new Intent(v.getContext(), LoginActivity.class) );
                overridePendingTransition(0, 0);
                finish();
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
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutTop) );

        //
        //  Cinema System Information
        //
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();

        //
        //  Alert Message
        //
        int configCabinetNum = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));
        int detectCabientNum = ((CinemaInfo)getApplicationContext()).GetCabinet().length;
        if( configCabinetNum != detectCabientNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinet() ) {
            String strMessage = String.format(Locale.US, "Please Check Cabinet Number. ( setting: %d, detect: %d )", configCabinetNum, detectCabientNum);
            CinemaAlert.Show( TopActivity.this, "Alert",  strMessage );
        }

        //
        //
        //
        TextView textComment = (TextView)findViewById(R.id.textCommentTop);
        textComment.setText("");
        textComment.append(String.format(Locale.US, "-. TCON Config File\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME));
        textComment.append(String.format(Locale.US, "-. PFPGA Config File\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Uniformity File\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Gamma File\t\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME));
        textComment.append(String.format(Locale.US, "-. Dot Correct Path\t\t\t\t: [USB_TOP]/%s/IDxxx/\n", LedDotCorrectInfo.PATH));
        textComment.append(String.format(Locale.US, "-. Dot Correct Extract Path\t: [USB_TOP]/DOT_CORRECTION_IDxxx/\n"));

        //
        //
        //
        mSpinnerInitialValue= (Spinner)findViewById(R.id.spinnerInitialValue);
        mSpinnerInitialValue.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[] { "1", "2", "3", "4"}));

        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mSpinnerInitialValue.setSelection( (strTemp == null) ? 0 : Integer.parseInt(strTemp));
        mSpinnerInitialValue.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                mTextInitModeDescription.setText(String.format(Locale.US, "This is mode #%d", i + 1));
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        mTextInitModeCurrent = (TextView)findViewById(R.id.textInitModeCurrent);
        mTextInitModeCurrent.setText(String.format(Locale.US, "-. Current Mode : Mode #%d", mSpinnerInitialValue.getSelectedItemPosition() + 1));

        mTextInitModeDescription = (TextView)findViewById(R.id.textInitModeDescription);
        mTextInitModeDescription.setText(String.format(Locale.US, "This is mode #%d", mSpinnerInitialValue.getSelectedItemPosition() + 1));

        mBtnUpdateInitialValue = (Button)findViewById(R.id.btnUpdateInitialValue);
        mBtnUpdateInitialValue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String[] resultPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
                for( String path : resultPfpga ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigPfpgaInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
                for( String path : resultUniformity ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedUniformityInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultQuality = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                for( String path : resultQuality ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigTconInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedGammaInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultPfpga.length != 0) || (resultUniformity.length != 0 ) || (resultQuality.length != 0) || (resultGamma.length != 0) ) {
                    ShowMessage( "Update Initial Value File.");
                    UpdateInitialValue();
                }
            }
        });

        mBtnApplyInitialValue = (Button)findViewById(R.id.btnApplyInitialValue);
        mBtnApplyInitialValue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskInitialValue().execute();
            }
        });

        UpdateInitialValue();

        //
        //
        //
        Button btnMenuDiagnostics = (Button)findViewById(R.id.btnMenuDiagonostics);
        Button btnMenuDisplayCheck = (Button)findViewById(R.id.btnMenuDisplayCheck);
        Button btnMenuDisplayMode = (Button)findViewById(R.id.btnMenuDisplayMode);
        Button btnMenuSystem = (Button)findViewById(R.id.btnMenuSystem);
        Button btnMenuAccount = (Button)findViewById(R.id.btnMenuAccount);

        btnMenuDiagnostics.setOnClickListener(mClickListener);
        btnMenuDisplayCheck.setOnClickListener(mClickListener);
        btnMenuDisplayMode.setOnClickListener(mClickListener);
        btnMenuSystem.setOnClickListener(mClickListener);
        btnMenuAccount.setOnClickListener(mClickListener);

        CinemaInfo info = ((CinemaInfo)getApplicationContext());
        Log.i(VD_DTAG, "--> Login Group : " + info.GetUserGroup());

        if( !info.IsCheckLogin() )
            btnMenuAccount.setEnabled(false);

        if( info.GetUserGroup().equals(AccountPreference.GROUP_OPERATOR) ) {
            btnMenuDiagnostics.setEnabled(false);
            btnMenuDisplayCheck.setEnabled(false);
            btnMenuSystem.setEnabled(false);
            btnMenuAccount.setEnabled(false);
        }

        if( info.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            btnMenuSystem.setEnabled(false);
        }
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch( (v).getId() ) {
                case R.id.btnMenuDiagonostics:
                    startActivity( new Intent(v.getContext(), DiagnosticsActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuDisplayCheck:
                    startActivity( new Intent(v.getContext(), DisplayCheckActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuDisplayMode:
                    startActivity( new Intent(v.getContext(), DisplayModeActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuSystem:
                    startActivity( new Intent(v.getContext(), SystemActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuAccount:
                    startActivity( new Intent(v.getContext(), AccountActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
            }
        }
    };

    private void UpdateInitialValue() {
        String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
        String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
        String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
        String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
        String[] internalPfpga = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        String[] internalUniformity = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
        String[] internalTcon = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET, ConfigTconInfo.NAME);
        String[] internalGamma = FileManager.CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);

        if( (usbPfpga != null && usbPfpga.length != 0) ||
            (usbUniformity != null && usbUniformity.length != 0) ||
            (usbTcon != null && usbTcon.length != 0) ||
            (usbGamma != null && usbGamma.length != 0) ) {
            mBtnUpdateInitialValue.setEnabled(true);
        }

        if( (internalPfpga != null && internalPfpga.length != 0) ||
            (internalUniformity != null && internalUniformity.length != 0) ||
            (internalTcon != null && internalTcon.length != 0) ||
            (internalGamma != null && internalGamma.length != 0) ) {
            mBtnApplyInitialValue.setEnabled(true);
        }
    }

    private class AsyncTaskInitialValue extends AsyncTask<Void, Void, Void> {
        private int mIndexInitialValue = mSpinnerInitialValue.getSelectedItemPosition();

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            //
            //  Check TCON Booting Status
            //
            if( ((CinemaInfo)getApplicationContext()).IsCheckTconBooting() ) {
                boolean bTconBooting = true;
                for( byte id : mCabinet ) {
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
            boolean enableUniformity = false;
            boolean[] enableGamma = {false, };

            resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : resultPath ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    enableUniformity = info.GetEnableUpdateUniformity(mIndexInitialValue);
                    for( int i = 0; i < info.GetRegister(mIndexInitialValue).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mIndexInitialValue)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexInitialValue)[i], NxCinemaCtrl.FORMAT_INT16);
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
                    if( !enableUniformity ) {
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
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                ConfigTconInfo info = new ConfigTconInfo();
                if( info.Parse( file ) ) {
                    enableGamma = info.GetEnableUpdateGamma(mIndexInitialValue);

                    for( int i = 0; i < info.GetRegister(mIndexInitialValue).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mIndexInitialValue)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexInitialValue)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }
            }

            //
            //  Write Gamma
            //
            resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && !enableGamma[0]) ||
                        (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && !enableGamma[1]) ||
                        (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && !enableGamma[2]) ||
                        (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && !enableGamma[3]) ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Gamma. ( %s )", file ));
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

            //
            //  PFPGA Mute off
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
            return null;

        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( TopActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
            mTextInitModeCurrent.setText(String.format(Locale.US, "-. Current Mode : Mode #%d", mIndexInitialValue + 1));
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mIndexInitialValue));
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
        boolean isOn = mService.IsOn();
        mService.RefreshScreenSaver();

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
