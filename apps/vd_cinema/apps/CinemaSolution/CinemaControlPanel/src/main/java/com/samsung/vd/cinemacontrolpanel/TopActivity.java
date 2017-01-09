package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.text.Layout;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends AppCompatActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

    private byte[]  mCabinet;

    private Spinner mSpinnerImageQuality;
    private Button mBtnUpdateImageQuality;
    private Button mBtnApplyImageQuality;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if( intent.getAction().equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                String[] resultQuality = CheckFileInUsb(LedQualityInfo.PATH_SOURCE, LedQualityInfo.NAME);
                String[] resultGamma = CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);

                if( (resultQuality != null && resultQuality.length != 0) || (resultGamma != null && resultGamma.length != 0) ) {
                    mBtnUpdateImageQuality.setEnabled(true);
                }
            }
            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnUpdateImageQuality.setEnabled(false);
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
        setContentView(R.layout.activity_top);

        //
        //  Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutTop ));
        titleBar.SetTitle( "Cinema LED Display System - Top Menu" );
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

        mSpinnerImageQuality = (Spinner)findViewById(R.id.spinnerImageQuality);
        mSpinnerImageQuality.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[] { "1", "2", "3", "4"}));

        mBtnUpdateImageQuality = (Button)findViewById(R.id.btnUpdateImageQuality);
        mBtnUpdateImageQuality.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String[] resultQuality = CheckFileInUsb(LedQualityInfo.PATH_SOURCE, LedQualityInfo.NAME);
                for( String path : resultQuality ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileCopy(path, LedQualityInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileCopy(path, LedGammaInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultQuality.length != 0) || (resultGamma.length != 0) ) {
                    ShowMessage( "Update Image Quality File.");
                    UpdateImageQuality();
                }
            }
        });

        mBtnApplyImageQuality = (Button)findViewById(R.id.btnApplyImageQuality);
        mBtnApplyImageQuality.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskImageQuality().execute();
            }
        });

        UpdateImageQuality();

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

    private void UpdateImageQuality() {
        String[] usbQuality = CheckFileInUsb(LedQualityInfo.PATH_SOURCE, LedQualityInfo.NAME);
        String[] usbGamma = CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
        String[] internalQuality = CheckFile(LedQualityInfo.PATH_TARGET, LedQualityInfo.NAME);
        String[] internalGamma = CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);

        if( (usbQuality != null && usbQuality.length != 0) || (usbGamma != null && usbGamma.length != 0) ) {
            mBtnUpdateImageQuality.setEnabled(true);
        }

        if( (internalQuality != null && internalQuality.length != 0) || (internalGamma != null && internalGamma.length != 0) ) {
            mBtnApplyImageQuality.setEnabled(true);
        }
    }

    //
    //
    //
    public static void FileCopy(String inFile, String outFile) {
        FileInputStream inStream = null;
        FileOutputStream outStream = null;

        FileChannel inChannel = null;
        FileChannel outChannel = null;

        try {
            inStream = new FileInputStream(inFile);
            outStream = new FileOutputStream(outFile);

            inChannel = inStream.getChannel();
            outChannel = outStream.getChannel();

            long size = inChannel.size();
            inChannel.transferTo(0, size, outChannel);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                outChannel.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                inChannel.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            try{
                outStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            try{
                inStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private String[] CheckFile( String topdir, String regularExpression ) {
        String[] result = new String[0];
        File topfolder = new File( topdir );
        File[] toplist = topfolder.listFiles();
        if( toplist == null || toplist.length == 0 )
            return result;

        Pattern pattern = Pattern.compile( regularExpression );
        for( File file : toplist ) {
            if( !file.isFile() )
                continue;

            Matcher matcher = pattern.matcher(file.getName());
            if( matcher.matches() ) {
                String[] temp = Arrays.copyOf( result, result.length + 1);
                temp[result.length] = file.getAbsolutePath();
                result = temp;
            }
        }

        return result;
    }

    private String[] CheckFileInUsb( String topdir, String regularExpression ) {
        String[] result = new String[0];
        for( int i = 0; i < 10; i++ ) {
            File topfolder = new File( String.format(Locale.US, "/storage/usbdisk%d/%s", i, topdir) );
            File[] toplist = topfolder.listFiles();
            if( toplist == null || toplist.length == 0 )
                continue;

            Pattern pattern = Pattern.compile( regularExpression );
            for( File file : toplist ) {
                if( !file.isFile() )
                    continue;

                Matcher matcher = pattern.matcher(file.getName());
                if( matcher.matches() ) {
                    String[] temp = Arrays.copyOf( result, result.length + 1);
                    temp[result.length] = file.getAbsolutePath();
                    result = temp;
                }
            }
        }

        return result;
    }

    private class AsyncTaskImageQuality extends AsyncTask<Void, Void, Void> {
        private int mIndexQuality = mSpinnerImageQuality.getSelectedItemPosition();

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

            byte[] resultLvds = ctrl.Send( mCabinet[0], NxCinemaCtrl.CMD_TCON_LVDS_STATUS, null );
            if (resultLvds == null || resultLvds.length == 0 ) {
                Log.i(VD_DTAG, "LVDS Signal is not valid.");
                return null;
            }

            if( resultLvds[0] == (byte)0x00 ) {
                Log.i(VD_DTAG, "LVDS Signal is not valid.");
                return null;
            }

            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

            String[] result;
            result = CheckFile(LedQualityInfo.PATH_TARGET, LedQualityInfo.NAME);
            for( String file : result ) {
                LedQualityInfo info = new LedQualityInfo();
                if( info.Parse( file ) ) {
                    for( int i = 0; i < info.GetRegister().length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister()[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_QUALITY, inData);
                        if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_QUALITY, inData);

                        Log.i(VD_DTAG, String.format(Locale.US, ">> [%d] reg(0x%02x), data(0x%02x%02x)", mIndexQuality, reg[0], data[0], data[1]));
                    }
                }
            }

            result = CheckFile(LedQualityInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);
            for( String file : result ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    int cmd;
                    if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                        cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                    else
                        cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                    byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                    byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                    byte[] inData = ctrl.AppendByteArray(table, data);

                    if( bValidPort0 ) ctrl.Send( 0x09, cmd + info.GetChannel(), inData );
                    if( bValidPort1 ) ctrl.Send( 0x89, cmd + info.GetChannel(), inData );
                }
            }

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