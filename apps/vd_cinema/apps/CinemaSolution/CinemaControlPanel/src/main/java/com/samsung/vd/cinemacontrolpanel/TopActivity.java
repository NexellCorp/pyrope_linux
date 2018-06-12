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
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.lang.ref.WeakReference;
import java.util.Locale;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends AppCompatActivity {
    private final String VD_DTAG = "TopActivity";

    private UIHandler mUIHandler;

    private byte[]  mCabinet;

    private int mInitMode;
    private Button mBtnInitMode;
    private TextView mTextInitMode;

    private TextButtonAdapter mAdapterMode;
    private ConfigTconInfo mTconEEPRomInfo = new ConfigTconInfo();
    private ConfigTconInfo mTconUsbInfo = new ConfigTconInfo();

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

                    mBtnInitMode.setEnabled(true);

                    if( resultPfpga != null ) for(String file : resultPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultUniformity != null ) for(String file : resultUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultTcon != null ) for(String file : resultTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( resultGamma != null ) for(String file : resultGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                }
            }
            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnInitMode.setEnabled(false);
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

        mUIHandler = new UIHandler( this );

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
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
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
        //  Parse TCON Configuration Files.
        //
        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );

        mAdapterMode.clear();
        for( int i = 0; i < 20; i++ ) {
            TextButtonInfo info = new TextButtonInfo(String.format(Locale.US, "mode #%d", i + 1), "");
            mAdapterMode.add(info);
        }

        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconEEPRomInfo.Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconEEPRomInfo.GetModeNum() ) {
                        TextButtonInfo info = mAdapterMode.getItem(i);
                        if( null == info )
                            continue;

                        info.SetText(mTconEEPRomInfo.GetDescription(i));
                        info.SetEnable(true);
                        info.SetOnClickListener(mTextbuttonAdapterClickListener);
                    }

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconUsbInfo.Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconUsbInfo.GetModeNum() ) {
                        TextButtonInfo info = mAdapterMode.getItem(10+i);
                        if (null == info)
                            continue;

                        info.SetText(mTconUsbInfo.GetDescription(i));
                        info.SetEnable(true);
                        info.SetOnClickListener(mTextbuttonAdapterClickListener);
                    }

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        //
        //
        //
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);

        mTextInitMode = (TextView)findViewById(R.id.textInitModeCurrent);
        mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode+1));

        mBtnInitMode = (Button)findViewById(R.id.btnInitModeUpdate);
        mBtnInitMode.setOnClickListener(new View.OnClickListener() {
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
                    FileManager.CopyFile(path, ConfigTconInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedGammaInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultPfpga.length != 0) || (resultUniformity.length != 0 ) || (resultQuality.length != 0) || (resultGamma.length != 0) ) {
                    ShowMessage( "Update Initial Value File.");
                    UpdateInitialValue();
                }

                //
                //  TCON EEPROM Test Code.
                //
//                new AsyncTaskEEPRomRead().execute();
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

    private TextButtonAdapter.OnClickListener mTextbuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            new AsyncTaskInitMode(position).execute();
        }
    };

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
        String[] internalTcon = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        String[] internalGamma = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);

        if( (usbPfpga != null && usbPfpga.length != 0) ||
            (usbUniformity != null && usbUniformity.length != 0) ||
            (usbTcon != null && usbTcon.length != 0) ||
            (usbGamma != null && usbGamma.length != 0) ) {
            mBtnInitMode.setEnabled(true);
        }

        if( (internalPfpga != null && internalPfpga.length != 0) ||
            (internalUniformity != null && internalUniformity.length != 0) ||
            (internalTcon != null && internalTcon.length != 0) ||
            (internalGamma != null && internalGamma.length != 0) ) {
            mBtnInitMode.setEnabled(true);
        }

        mAdapterMode.clear();
        for( int i = 0; i < 20; i++ ) {
            TextButtonInfo info = new TextButtonInfo(String.format(Locale.US, "mode #%d", i + 1), "");
            mAdapterMode.add(info);
        }

        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconEEPRomInfo.Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconEEPRomInfo.GetModeNum() ) {
                        TextButtonInfo info = mAdapterMode.getItem(i);
                        if( null == info )
                            continue;

                        info.SetText(mTconEEPRomInfo.GetDescription(i));
                        info.SetEnable(true);
                        info.SetOnClickListener(mTextbuttonAdapterClickListener);
                    }

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconUsbInfo.Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconUsbInfo.GetModeNum() ) {
                        TextButtonInfo info = mAdapterMode.getItem(10+i);
                        if (null == info)
                            continue;

                        info.SetText(mTconUsbInfo.GetDescription(i));
                        info.SetEnable(true);
                        info.SetOnClickListener(mTextbuttonAdapterClickListener);
                    }

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }
    }

    //
    //  TCON EEPRom Test Code.
    //
    private class AsyncTaskEEPRomRead extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
            if( result == null || result.length == 0 ) {
                Log.i(VD_DTAG, "Unknown Error.");
                return null;
            }

            if( (int)result[0] == 0xFF ) Log.i(VD_DTAG, "Fail, EEPROM Read.");
            if( (int)result[0] == 0x01 ) Log.i(VD_DTAG, "Update is successful.");
            if( (int)result[0] == 0x00 ) Log.i(VD_DTAG, "Update is not needed.");
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

    private class AsyncTaskInitMode extends AsyncTask<Void, Void, Void> {
        private int mInitMode;
        private int mUpdateGamma[] = new int[4];

        public AsyncTaskInitMode(int mode) {
            mInitMode = mode;
            mUpdateGamma[0] = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_UPDATE_TGAM0));
            mUpdateGamma[1] = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_UPDATE_TGAM1));
            mUpdateGamma[2] = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_UPDATE_DGAM0));
            mUpdateGamma[3] = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_UPDATE_DGAM1));
        }

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
            if( (10 > mInitMode) && (mInitMode < mTconEEPRomInfo.GetModeNum())) {
                enableGamma = mTconEEPRomInfo.GetEnableUpdateGamma(mInitMode);
                for( int i = 0; i < mTconEEPRomInfo.GetRegister(mInitMode).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(mTconEEPRomInfo.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(mTconEEPRomInfo.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }
            }

            if( (10 <= mInitMode) && ((mInitMode-10) < mTconUsbInfo.GetModeNum())) {
                enableGamma = mTconUsbInfo.GetEnableUpdateGamma(mInitMode-10);
                for( int i = 0; i < mTconUsbInfo.GetRegister(mInitMode-10).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(mTconUsbInfo.GetRegister(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(mTconUsbInfo.GetData(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
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
            if( (10 > mInitMode) && (mInitMode < mTconEEPRomInfo.GetModeNum())) {
                resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                for( String file : resultPath ) {
                    LedGammaInfo info = new LedGammaInfo();
                    if( info.Parse( file ) ) {
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
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
                            Log.i(VD_DTAG, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
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

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update USB Gamma. ( %s )", file ));
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
            ((CinemaInfo)getApplicationContext()).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
            ((CinemaInfo)getApplicationContext()).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
            ((CinemaInfo)getApplicationContext()).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
            ((CinemaInfo)getApplicationContext()).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

            //
            //  SW Reset
            //
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

            //
            //  PFPGA Mute off
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

            return null;

        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "Mode Configuration Start.");
            CinemaLoading.Show( TopActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode + 1));
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mInitMode));
            CinemaLoading.Hide();

            Log.i(VD_DTAG, String.format("Mode Configuration Done. ( mode = %d )", mInitMode + 1));
        }
    }

    //
    //
    //
    CinemaService.ChangeContentsCallback mCallback = new CinemaService.ChangeContentsCallback() {
        @Override
        public void onChangeContentsCallback(int mode) {
            mUIHandler.sendEmptyMessage(0);
        }
    };

    private static class UIHandler extends Handler {
        private WeakReference<TopActivity> mActivity;

        public UIHandler( TopActivity activity ) {
            mActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            TopActivity activity = mActivity.get();
            if( activity != null ) {
                activity.handleMessage(msg);
            }
        }
    }

    private void handleMessage( Message msg ) {
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
        mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode+1));
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
            mService.RegisterCallback( mCallback );
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };
}
