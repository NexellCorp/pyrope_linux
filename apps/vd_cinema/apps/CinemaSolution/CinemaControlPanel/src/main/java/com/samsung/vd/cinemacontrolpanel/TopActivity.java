package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "TopActivity";

    private CinemaInfo mCinemaInfo;

    private int mInitMode;
    private Button mBtnInitMode;
    private TextView mTextInitMode;

    private TextButtonAdapter mAdapterMode;
    private ConfigTconInfo mTconEEPRomInfo = new ConfigTconInfo();
    private ConfigTconInfo mTconUsbInfo = new ConfigTconInfo();

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if( null == action )
                return;

            if( action.equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
                String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
                String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);

                if( (usbPfpga != null && usbPfpga.length != 0) ||
                    (usbUniformity != null && usbUniformity.length != 0) ||
                    (usbTcon != null && usbTcon.length != 0) ||
                    (usbGamma != null && usbGamma.length != 0) ) {

                    mBtnInitMode.setEnabled(true);

                    if( usbPfpga != null ) for(String file : usbPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( usbUniformity != null ) for(String file : usbUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( usbTcon != null ) for(String file : usbTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( usbGamma != null ) for(String file : usbGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                }
            }
            if( action.equals(Intent.ACTION_MEDIA_EJECT) ) {
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
        setContentView(R.layout.activity_top);

        //
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();

        final ViewGroup rootGroup = null;
        View listViewFooter = null;
        LayoutInflater inflater = (LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        if( inflater != null ) listViewFooter = inflater.inflate(R.layout.listview_footer_blank, rootGroup, false);

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
                mCinemaInfo.InsertLog("Logout.");
                Launch(v.getContext(), LoginActivity.class);
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TurnOff();
            }
        });

        if( !mCinemaInfo.IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !mCinemaInfo.IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        //
        //  Configuration Status Bar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutTop) );

        //
        //  Alert Message
        //
        String strCabinetNum = mCinemaInfo.GetValue(CinemaInfo.KEY_CABINET_NUM);
        int configCabinetNum = Integer.parseInt( (strCabinetNum != null) ? strCabinetNum : "0" );
        int detectCabientNum = mCinemaInfo.GetCabinet().length;
        if( configCabinetNum != detectCabientNum && mCinemaInfo.IsCheckCabinetNum() ) {
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
        //  EEPROM Status
        //
        TextView textEEPRomStatus = (TextView)findViewById(R.id.textEEPRomStatus);
        textEEPRomStatus.setText( mCinemaInfo.IsValidEEPRom() ? "VALID" : "INVALID"  );

        //
        //  Parse TCON Configuration Files.
        //
        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );

        String strTemp = mCinemaInfo.GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);

        mTextInitMode = (TextView)findViewById(R.id.textInitModeCurrent);
        mTextInitMode.setText( (0 > mInitMode) ?
                String.format(Locale.US, "Mode Invalid") :
                String.format(Locale.US, "Mode #%d", mInitMode + 1)
        );

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

        Log.i(VD_DTAG, "--> Login Group : " + mCinemaInfo.GetUserGroup());
        if( !mCinemaInfo.IsCheckLogin() )
            btnMenuAccount.setEnabled(false);

        if( mCinemaInfo.GetUserGroup().equals(AccountPreference.GROUP_OPERATOR) ) {
            btnMenuDiagnostics.setEnabled(false);
            btnMenuDisplayCheck.setEnabled(false);
            btnMenuSystem.setEnabled(false);
            btnMenuAccount.setEnabled(false);
        }

        if( mCinemaInfo.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            btnMenuSystem.setEnabled(false);
        }

        this.RegisterTmsCallback(new CinemaService.TmsEventCallback() {
            @Override
            public void onTmsEventCallback(Object[] values) {
                if( !(values instanceof Integer[]) )
                    return;

                if( 0 > (Integer)values[0] ) {

                    return;
                }

                if( CinemaTask.CMD_TMS_QUE > (Integer)values[0] ) {
                    mInitMode = (Integer)values[0];
                    mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode +1));
                }

                if( CinemaTask.CMD_TMS_QUE <= (Integer)values[0] ) {
                    UpdateInitialValue();
                }
            }
        });
    }

    private TextButtonAdapter.OnClickListener mTextButtonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_MODE,
                    getApplicationContext(),
                    position,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            if( !(values instanceof Integer[]) )
                                return;

                            if( 0 > (Integer)values[0] )
                                Log.i(VD_DTAG, "Fail, Change Mode.");
                            else {
                                mInitMode = (Integer)values[0];
                                mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode + 1));

                                Log.i(VD_DTAG, String.format("Change Mode Done. ( mode = %d )", (Integer)values[0] + 1));
                            }
                            HideProgress();
                        }
                    },
                    null
            );
        }
    };

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch( (v).getId() ) {
                case R.id.btnMenuDiagonostics:
                    Launch(v.getContext(), DiagnosticsActivity.class);
                    break;
                case R.id.btnMenuDisplayCheck:
                    Launch(v.getContext(), DisplayCheckActivity.class);
                    break;
                case R.id.btnMenuDisplayMode:
                    Launch(v.getContext(), DisplayModeActivity.class);
                    break;
                case R.id.btnMenuSystem:
                    Launch(v.getContext(), SystemActivity.class);
                    break;
                case R.id.btnMenuAccount:
                    Launch(v.getContext(), AccountActivity.class);
                    break;
            }
        }
    };

    private void UpdateInitialValue() {
        String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
        String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
        String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
        String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);

        if( (usbPfpga != null && usbPfpga.length != 0) ||
            (usbUniformity != null && usbUniformity.length != 0) ||
            (usbTcon != null && usbTcon.length != 0) ||
            (usbGamma != null && usbGamma.length != 0) ) {

            mBtnInitMode.setEnabled(true);

            if( usbPfpga != null ) for(String file : usbPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
            if( usbUniformity != null ) for(String file : usbUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
            if( usbTcon != null ) for(String file : usbTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
            if( usbGamma != null ) for(String file : usbGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
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
                        info.SetEnable(
                                (mTconEEPRomInfo.GetDataMode(i) == ConfigTconInfo.MODE_BOTH) ||
                                (mTconEEPRomInfo.GetDataMode(i) == ConfigTconInfo.MODE_3D && mCinemaInfo.IsMode3D())
                        );
                        info.SetOnClickListener(mTextButtonAdapterClickListener);
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
                        info.SetEnable(
                                (mTconUsbInfo.GetDataMode(i) == ConfigTconInfo.MODE_BOTH) ||
                                (mTconUsbInfo.GetDataMode(i) == ConfigTconInfo.MODE_3D && mCinemaInfo.IsMode3D())
                        );
                        info.SetOnClickListener(mTextButtonAdapterClickListener);
                    }

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }
    }
}
