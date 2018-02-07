package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdSpinCtrl;
import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.Comparator;
import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private UIHandler mUIHandler;

    private LayoutInflater  mInflater;
    private LinearLayout    mParentLayoutMastering;
    private LinearLayout[]  mLayoutMastering;

    private byte[]  mCabinet;

    private String[] mStrModeMastering = {
            "DCI", "HDR", "MASTER"
    };

    private String[][] mTextMastering = {
            { "CC00",           "16383" },
            { "CC01",           "16383" },
            { "CC02",           "16383" },
            { "CC10",           "16383" },
            { "CC11",           "16383" },
            { "CC12",           "16383" },
            { "CC20",           "16383" },
            { "CC21",           "16383" },
            { "CC22",           "16383" },
            { "Global",         "7"     },
            { "R Channel",      "511"   },
            { "G Channel",      "511"   },
            { "B Channel",      "511"   },
            { "R Channel(Low)", "15"    },
            { "G Channel(Low)", "15"    },
            { "B Channel(Low)", "15"    },
    };

    private int[] mRegHideMastering = {
            0x0082, 0x0097,
    };

    private int[][] mDataHideMastering = {
            { 0x0000, 0x0000 },
            { 0x0001, 0x0001 },
            { 0x0000, 0x0000 },
    };

    private int[] mRegMastering = {
            0x0056,     0x0057,     0x0058,     //  REG_CC00,       REG_CC01,       REG_CC02
            0x0059,     0x005A,     0x005B,     //  REG_CC10,       REG_CC11,       REG_CC12
            0x005C,     0x005D,     0x005E,     //  REG_CC20,       REG_CC21,       REG_CC22
            0x00DD,                             //  REG_BC
            0x00B9,     0x00BA,     0x00BB,     //  REG_CC_R01,     REG_CC_G01,     REG_CC_B01
            0x00B6,     0x00B7,     0x00B8,     //  REG_LGSE1_R,    REG_LGSE1_G,    REG_LGSE1_B
    };

    private int[] mDataDci = {
            6361,       74,         269,
            272,        6586,       29,
            0,          128,        6701,
            1,
            145,        98,         67,
            14,         15,         12,
    };

    private int[] mDataHdr = {
            12869,      1989,       507,
            607,        15065,      177,
            0,          0,          16383,
            4,
            252,        234,        111,
            5,          5,          8,
    };

    private int[] mDataMaster = new int[mTextMastering.length];

    private int[][] mDataMastering = {
            mDataDci,
            mDataHdr,
            mDataMaster,
    };

    private Spinner mSpinnerMastering;
    private Button[] mBtnMastering = new Button[mTextMastering.length];
    private SeekBar[] mSeekBarMastering = new SeekBar[mTextMastering.length];
    private EditText[] mValueMastering = new EditText[mTextMastering.length];

    private String mMasteringMode = mStrModeMastering[0];
    private int mMasteringModePos = 0;

    private Spinner mSpinnerUniformity;
    private Button mBtnEnableUniformity;
    private Button mBtnUpdateUniformity;
    private Button mBtnApplyUniformity;

    private TextView mTextImageQuality;
    private Button mBtnUpdateImageQuality;

    private TextButtonAdapter mAdapterMode;
    private ConfigTconInfo mTconEEPRomInfo = new ConfigTconInfo();
    private ConfigTconInfo mTconUsbInfo = new ConfigTconInfo();

    private CheckRunAdapter mAdapterDotCorrect;

    private EditText mEditCabinet;

    private Button mBtnDotCorrectCheckAll;
    private Button mBtnDotCorrectUnCheckAll;
    private Button mBtnDotCorrectApply;

    private Spinner mSpinnerDotCorrectExtractId;
    private Spinner mSpinnerDotCorrectExtractModule;
    private Button mBtnDotCorrectExtract;

    private Spinner mSpinnerWhiteSeamCabinetId;

    private VdSpinCtrl[] mSpinWhiteSeam = new VdSpinCtrl[4];
    private CheckBox mCheckWhiteSeamEmulation;
    private Button mBtnWhiteSeamApply;
    private Spinner mSpinnerSuspendTime;

    private VdSpinCtrl mSpinSyncWidth;
    private Button mBtnApplySyncWidth;
    private VdSpinCtrl mSpinSyncDelay;
    private Button mBtnApplySyncDelay;
    private CheckBox mCheckSyncReverse;
    private CheckBox mCheckScale;
    private CheckBox mCheckZeroScale;
    private CheckBox mCheckSeam;
    private CheckBox mCheckModule;
    private CheckBox mCheckXyzInput;
    private CheckBox mCheckLedOpenDetection;
    private CheckBox mCheckLodRemoval;
    private Button mBtnLodReset;

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
                String[] resultUniformity = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
                if( (resultPfpga != null && resultPfpga.length != 0) || (resultUniformity != null && resultUniformity.length != 0 ) ) {
                    mBtnUpdateUniformity.setEnabled(true);
                }

                String[] resultTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                if( (resultTcon != null && resultTcon.length != 0) || (resultGamma != null && resultGamma.length != 0) ) {
                    mBtnUpdateImageQuality.setEnabled(true);
                }

                String[] resultDot = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
                if( (resultDot != null && resultDot.length != 0) ) {
                    new AsyncTaskAdapterDotCorrection(mAdapterDotCorrect).execute();
                    mBtnDotCorrectCheckAll.setEnabled(true);
                    mBtnDotCorrectUnCheckAll.setEnabled(true);
                    mBtnDotCorrectApply.setEnabled(true);
                }

                if( resultPfpga != null ) for(String file : resultPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                if( resultUniformity != null ) for(String file : resultUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                if( resultTcon != null ) for(String file : resultTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                if( resultGamma != null ) for(String file : resultGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));

                String strIndex = mSpinnerDotCorrectExtractId.getSelectedItem().toString();
                if( strIndex != null && !strIndex.equals("") ) {
                    mBtnDotCorrectExtract.setEnabled(true);
                }
            }

            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnUpdateUniformity.setEnabled(false);
                mBtnUpdateImageQuality.setEnabled(false);

                mBtnDotCorrectCheckAll.setEnabled(false);
                mBtnDotCorrectUnCheckAll.setEnabled(false);
                mBtnDotCorrectApply.setEnabled(false);

                mBtnDotCorrectExtract.setEnabled(false);

                mAdapterDotCorrect.clear();
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        //
        //  This is called after onCreate().
        //
        mLayoutMastering = new LinearLayout[mTextMastering.length];
        for( int i = 0; i < mTextMastering.length; i++ ) {
            mLayoutMastering[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_mastering, mParentLayoutMastering, false );
            AddViewMastering( i, mLayoutMastering, mTextMastering );
        }

        UpdateMasteringMode();

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
        setContentView(R.layout.activity_display_mode);

        mUIHandler = new UIHandler( this );

        //
        //  Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Display Mode" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InitWhiteSeamValue();

                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InitWhiteSeamValue();

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
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //  Cinema System Information
        //
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
        AddTabs();

        //
        //  Cabinet Number & Module Number String
        //
        String[] strCabinetNumber;
        if( mCabinet.length == 0 ) {
            strCabinetNumber = new String[]{ "" };
        }
        else {
            strCabinetNumber = new String[mCabinet.length];
            for( int i = 0; i < mCabinet.length; i++ ) {
                strCabinetNumber[i] = String.valueOf((mCabinet[i] & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
            }
        }

        String[] strCabinetNumberWithAll = new String[mCabinet.length+1];
        strCabinetNumberWithAll[0] = "ALL";
        if( mCabinet.length == 0 ) {
            strCabinetNumberWithAll = new String[]{ "" };
        }
        else {
            strCabinetNumberWithAll = new String[mCabinet.length+1];
            strCabinetNumberWithAll[0] = "ALL";
            for( int i = 0; i < mCabinet.length; i++ ) {
                strCabinetNumberWithAll[i+1] = String.valueOf((mCabinet[i] & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
            }
        }

        String [] strModuleWithAll =  new String[25];
        strModuleWithAll[0] = "ALL";
        for( int i = 0; i < 24; i++ ) {
            strModuleWithAll[i+1] = String.format(Locale.US, "MODULE #%d", i);
        }

        String[] resultPath;

        //
        //  MASTERING
        //
        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayoutMastering = (LinearLayout)findViewById(R.id.layoutMastering);

        mMasteringMode = ((CinemaInfo)getApplicationContext()).GetValue( CinemaInfo.KEY_MASTERING_MODE );
        if( mMasteringMode == null ) {
            mMasteringMode = mStrModeMastering[0];
            ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_MASTERING_MODE, mMasteringMode);
        }

        mSpinnerMastering = (Spinner)findViewById(R.id.spinnerMasteringMode);
        mSpinnerMastering.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrModeMastering));
        for( int i = 0; i < mStrModeMastering.length; i++ ) {
            if( mMasteringMode.equals(mStrModeMastering[i]) ) {
                mSpinnerMastering.setSelection(i);
                mMasteringModePos = i;
                break;
            }
        }

        Button btnMasteringMode = (Button)findViewById(R.id.btnMasteringMode);
        btnMasteringMode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mMasteringMode = mSpinnerMastering.getSelectedItem().toString();
                mMasteringModePos = mSpinnerMastering.getSelectedItemPosition();

                ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_MASTERING_MODE, mMasteringMode);
                if( !IsMasterMode() ) {
                    new AsyncTaskMasteringWrite().execute();
                }
                else {
                    new AsyncTaskMasteringWrite().execute();
                    new AsyncTaskMasteringRead().execute();
                }

                UpdateMasteringMode();

                ((CinemaInfo)getApplicationContext()).InsertLog(String.format( Locale.US, "Change Mastering Mode. ( %s Mode )", mMasteringMode ));
            }
        });

        //
        //  Uniformity Correction
        //
        mSpinnerUniformity = (Spinner)findViewById(R.id.spinnerUniformity);
//        mSpinnerUniformity.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[]{"1", "2", "3", "4"}));

        mBtnUpdateUniformity = (Button)findViewById(R.id.btnUpdateUniformity);
        mBtnUpdateUniformity.setOnClickListener(new View.OnClickListener() {
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

                if( (resultPfpga.length != 0) || (resultUniformity.length != 0) ) {
                    ShowMessage( "Update Uniformity File.");
                    UpdateUniformityCorrection();
                }
            }
        });

        mBtnEnableUniformity = (Button)findViewById(R.id.btnEnableUniformity);
        mBtnEnableUniformity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
                if( mBtnEnableUniformity.getText().toString().equals("ENABLE") ) {
                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN , NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
                    if( result == null || result.length == 0 || result[0] == (byte)0xFF ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "i2c write fail.( reg: 0x%04X, dat: 0x%04X )", NxCinemaCtrl.REG_PFPGA_NUC_EN, 0x0000 ));
                    }

                    mBtnEnableUniformity.setText("DISABLE");
                }
                else {
                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN , NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
                    if( result == null || result.length == 0 || result[0] == (byte)0xFF ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "i2c write fail.( reg: 0x%04X, dat: 0x%04X )", NxCinemaCtrl.REG_PFPGA_NUC_EN, 0x0001 ));
                    }

                    mBtnEnableUniformity.setText("ENABLE");
                }
            }
        });

        mBtnApplyUniformity = (Button)findViewById(R.id.btnApplyUniformity);
        mBtnApplyUniformity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskUniformityCorrection().execute();
            }
        });


        //
        //  IMAGE QUALITY
        //
        mTextImageQuality = (TextView)findViewById(R.id.textModeCurrent);

        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        int mode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
        mTextImageQuality.setText(String.format(Locale.US, "Mode #%d", mode+1));

        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        mAdapterMode.clear();

        for( String file : resultPath ) {
            if( mTconEEPRomInfo.Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconEEPRomInfo.GetModeNum() )
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i+1),
                                        String.format(Locale.US, "%s", mTconEEPRomInfo.GetDescription(i)),
                                        mTextbuttonAdapterClickListener )
                        );
                    else
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 1), "")
                        );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, "mode #%d", i + 1), "")
                    );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconUsbInfo.Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconUsbInfo.GetModeNum() )
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 11),
                                        String.format(Locale.US, "%s", mTconEEPRomInfo.GetDescription(i)),
                                        mTextbuttonAdapterClickListener )
                        );
                    else
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 11), "")
                        );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, "mode #%d", i + 11), "")
                    );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }


        mBtnUpdateImageQuality = (Button)findViewById(R.id.btnModeUpdate);
        mBtnUpdateImageQuality.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String[] resultTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                for( String path : resultTcon ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigTconInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedGammaInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultTcon.length != 0) || (resultGamma.length != 0) ) {
                    ShowMessage( "Update Image Quality File.");
                    UpdateImageQuality();
                }
            }
        });


        //
        //  DOT CORRECTION
        //
        ListView listViewDotCorrect = (ListView)findViewById(R.id.listview_dot_correction);
        listViewDotCorrect.addFooterView(listViewFooter);

        mAdapterDotCorrect = new CheckRunAdapter(this, R.layout.listview_row_check_run);
        listViewDotCorrect.setAdapter( mAdapterDotCorrect );

        mBtnDotCorrectCheckAll = (Button)findViewById(R.id.btnDotCorrectionCheckAll);
        mBtnDotCorrectUnCheckAll = (Button)findViewById(R.id.btnDotCorrectionUnCheckAll);
        mBtnDotCorrectApply = (Button)findViewById(R.id.btnDotCorrectionApply);

        mBtnDotCorrectCheckAll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                for( int i = 0; i < mAdapterDotCorrect.getCount(); i++ ) {
                    CheckRunInfo info = mAdapterDotCorrect.getItem(i);
                    info.SetChecked( true );
                }
                mAdapterDotCorrect.notifyDataSetChanged();
            }
        });

        mBtnDotCorrectUnCheckAll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                for( int i = 0; i < mAdapterDotCorrect.getCount(); i++ ) {
                    CheckRunInfo info = mAdapterDotCorrect.getItem(i);
                    info.SetChecked( false );
                }
                mAdapterDotCorrect.notifyDataSetChanged();
            }
        });

        mBtnDotCorrectApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskDotCorrection(mAdapterDotCorrect).execute();
            }
        });


        //
        //  DOT CORRECTION EXTRACT
        //
        mSpinnerDotCorrectExtractId = (Spinner)findViewById(R.id.spinnerDotCorrectionExtractId);
        mSpinnerDotCorrectExtractId.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strCabinetNumber) );

        mSpinnerDotCorrectExtractModule = (Spinner)findViewById(R.id.spinnerDotCorrectionExtractModule);
        mSpinnerDotCorrectExtractModule.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strModuleWithAll) );

        mBtnDotCorrectExtract = (Button)findViewById(R.id.btnDotCorrectionExtract);
        mBtnDotCorrectExtract.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String strIndex = mSpinnerDotCorrectExtractId.getSelectedItem().toString();
                if( strIndex == null || strIndex.equals("") )
                    return;

                int index = mCabinet[mSpinnerDotCorrectExtractId.getSelectedItemPosition()];
                int module= (mSpinnerDotCorrectExtractModule.getSelectedItemPosition() == 0 ) ?
                        LedDotCorrectInfo.MAX_MODULE_NUM : mSpinnerDotCorrectExtractModule.getSelectedItemPosition() - 1;

                new AsyncTaskDotCorrectionExtract(index, module).execute();
            }
        });


        //
        //  WHITE SEAM VALUE
        //
        mBtnWhiteSeamApply = (Button)findViewById(R.id.btnWhiteSeamApply);
        mCheckWhiteSeamEmulation = (CheckBox)findViewById(R.id.checkWhiteSeamEmulation);

        mSpinnerWhiteSeamCabinetId = (Spinner)findViewById(R.id.spinnerWhiteSeamCabinetId);
        mSpinnerWhiteSeamCabinetId.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strCabinetNumberWithAll) );
        mSpinnerWhiteSeamCabinetId.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        mSpinWhiteSeam[0] = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamTop);
        mSpinWhiteSeam[1] = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamBottom);
        mSpinWhiteSeam[2] = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamLeft);
        mSpinWhiteSeam[3] = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamRight);

        for( VdSpinCtrl spin : mSpinWhiteSeam ) {
            spin.SetRange(0, 32767);
            spin.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
                @Override
                public void onChange(int value) {
                    new AsyncTaskWhiteSeamEmulate(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
                }
            });
            spin.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                    new AsyncTaskWhiteSeamEmulate(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
                    return false;
                }
            });
        }

        mBtnWhiteSeamApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskWhiteSeamWrite(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
            }
        });

        //
        //  Global
        //
        mSpinSyncWidth = (VdSpinCtrl)findViewById(R.id.spinSyncWidth);
        mBtnApplySyncWidth = (Button)findViewById(R.id.btnApplySyncWidth);
        mSpinSyncDelay = (VdSpinCtrl)findViewById(R.id.spinSyncDelay);
        mBtnApplySyncDelay = (Button)findViewById(R.id.btnApplySyncDelay);
        mCheckSyncReverse = (CheckBox)findViewById(R.id.checkSyncReverse);
        mCheckScale = (CheckBox)findViewById(R.id.checkScale);
        mCheckZeroScale = (CheckBox)findViewById(R.id.checkZeroScale);
        mCheckSeam = (CheckBox)findViewById(R.id.checkSeam);
        mCheckModule = (CheckBox)findViewById(R.id.checkModule);
        mCheckXyzInput = (CheckBox)findViewById(R.id.checkXyzInput);
        mCheckLedOpenDetection = (CheckBox)findViewById(R.id.checkLedOpenDetect);
        mCheckLodRemoval = (CheckBox)findViewById(R.id.checkLodRemoval);
        mBtnLodReset = (Button)findViewById(R.id.btnLodReset);

        mSpinSyncWidth.SetRange(0, 4095);
        mSpinSyncDelay.SetRange(0, 4095);

        UnregisterListener();

        //
        //  Spinner Cabinet Number
        //
        String curCabinetNum = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM);

        mEditCabinet = (EditText)findViewById(R.id.editCabinet);
        mEditCabinet.setText(curCabinetNum);

        Button btnCabinetNum = (Button)findViewById(R.id.btnCabinetNumApply);
        btnCabinetNum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AsyncTaskCheckCabinet().execute();
            }
        });


        //
        //  Spinner Suspend Time
        //
        String[] strSuspendTime = {
            "Disable", "1 min", "3 min", "5 min", "10 min", "20 min", "30 min"
        };

        mSpinnerSuspendTime = (Spinner)findViewById(R.id.spinnerSuspendTime);
        mSpinnerSuspendTime.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strSuspendTime) );

        Button btnSuspendTime = (Button)findViewById(R.id.btnSuspendTimeApply);
        btnSuspendTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ShowMessage("Apply");
                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[mSpinnerSuspendTime.getSelectedItemPosition()]);
                mService.RefreshScreenSaver();
            }
        });

        UpdateSetup();

        //
        //  IMM Handler
        //
        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
//                imm.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(), 0);
//                getCurrentFocus().clearFocus();

                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditCabinet.getWindowToken(), 0);

                for( int i = 0; i < mValueMastering.length; i++) {
                    String strValue = mValueMastering[i].getText().toString();
                    if( strValue.equals("") ) {
                        strValue = "0";
                    }

                    if( Integer.parseInt(strValue, 10) > mSeekBarMastering[i].getMax() ) {
                        strValue = String.valueOf(mSeekBarMastering[i].getMax());
                    }

                    mValueMastering[i].setText(strValue);
                    mSeekBarMastering[i].setProgress(Integer.parseInt(strValue, 10));

                    int pos = mSpinnerMastering.getSelectedItemPosition();
                    int progress = mSeekBarMastering[i].getProgress();
                    if( mDataMastering[pos][i] == progress && mBtnMastering[i].isEnabled() ) {
                        mBtnMastering[i].setEnabled(false);
                    }
                    if( mDataMastering[pos][i] != progress && !mBtnMastering[i].isEnabled() ) {
                        if( IsMasterMode() ) {
                            mBtnMastering[i].setEnabled(true);
                        }
                    }
                }
            }
        });
    }

    private void RegisterListener() {
        mBtnApplySyncWidth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ApplySyncWidth();
            }
        });

        mSpinSyncWidth.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                ApplySyncWidth();
            }
        });

        mSpinSyncWidth.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                ApplySyncWidth();
                return false;
            }
        });

        mBtnApplySyncDelay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ApplySyncDelay();
            }
        });

        mSpinSyncDelay.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                ApplySyncDelay();
            }
        });

        mSpinSyncDelay.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                ApplySyncDelay();
                return false;
            }
        });

        mCheckSyncReverse.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplySyncReverse();
            }
        });

        mCheckScale.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyScale();
            }
        });

        mCheckZeroScale.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyZeroScale();
            }
        });

        mCheckSeam.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplySeam();
            }
        });

        mCheckModule.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyModule();
            }
        });

        mCheckXyzInput.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyXyzInput();
            }
        });

        mCheckLedOpenDetection.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyLedOpenDetection();
            }
        });

        mCheckLodRemoval.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                ApplyLodRemoval();
            }
        });

        mBtnLodReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ApplyLodReset();
            }
        });
    }

    private void UnregisterListener() {
        mBtnApplySyncWidth.setOnClickListener(null);
        mSpinSyncWidth.SetOnChangeListener(null);
        mSpinSyncWidth.SetOnEditorActionListener(null);
        mBtnApplySyncDelay.setOnClickListener(null);
        mSpinSyncDelay.SetOnChangeListener(null);
        mSpinSyncDelay.SetOnEditorActionListener(null);
        mCheckSyncReverse.setOnCheckedChangeListener(null);
        mCheckScale.setOnCheckedChangeListener(null);
        mCheckZeroScale.setOnCheckedChangeListener(null);
        mCheckSeam.setOnCheckedChangeListener(null);
        mCheckModule.setOnCheckedChangeListener(null);
        mCheckXyzInput.setOnCheckedChangeListener(null);
        mCheckLedOpenDetection.setOnCheckedChangeListener(null);
        mCheckLodRemoval.setOnCheckedChangeListener(null);
        mBtnLodReset.setOnClickListener(null);
    }

    private TextButtonAdapter.OnClickListener mTextbuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            new AsyncTaskImageQuality(position).execute();
        }
    };

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec0 = tabHost.newTabSpec("TAB0");
        tabSpec0.setIndicator("Mastering Mode");
        tabSpec0.setContent(R.id.tabMasteringMode);

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Uniformity");
        tabSpec1.setContent(R.id.tabUniformityCorrection);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Image Quality");
        tabSpec2.setContent(R.id.tabImageQuality);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Pixel Correction");
        tabSpec3.setContent(R.id.tabDotCorrection);

        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec("TAB4");
        tabSpec4.setIndicator("Pixel Correction Extract");
        tabSpec4.setContent(R.id.tabDotCorrectionExtract);

        TabHost.TabSpec tabSpec5 = tabHost.newTabSpec("TAB5");
        tabSpec5.setIndicator("White Seam Value");
        tabSpec5.setContent(R.id.tabWhiteSeamValue);

        TabHost.TabSpec tabSpec6 = tabHost.newTabSpec("TAB6");
        tabSpec6.setIndicator("Global");
        tabSpec6.setContent(R.id.tabGlobal);

        TabHost.TabSpec tabSpec7 = tabHost.newTabSpec("TAB7");
        tabSpec7.setIndicator("Set up");
        tabSpec7.setContent(R.id.tabSetup);

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);
        tabHost.addTab(tabSpec4);
        tabHost.addTab(tabSpec5);
        tabHost.addTab(tabSpec6);
        tabHost.addTab(tabSpec7);

        for( int i = 0; i < tabHost.getTabWidget().getChildCount(); i++ ) {
            View tabView = tabHost.getTabWidget().getChildAt(i);
//            tabView.getLayoutParams().height = ;
            tabView.setPadding( 5, 5, 5, 5 );
        }

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateMasteringMode();
            if( tabId.equals("TAB1") ) UpdateUniformityCorrection();
            if( tabId.equals("TAB2") ) UpdateImageQuality();
            if( tabId.equals("TAB3") ) UpdateDotCorrection();
            if( tabId.equals("TAB4") ) UpdateDotCorrectionExtract();
            if( tabId.equals("TAB5") ) UpdateWhiteSeamValue();
            if( tabId.equals("TAB6") ) UpdateGlobal();
            if( tabId.equals("TAB7") ) UpdateSetup();
        }
    };

    private boolean IsMasterMode() {
        return (mMasteringMode != null) && mMasteringMode.equals("MASTER");
    }

    private void UpdateMasteringMode() {
        InitWhiteSeamValue();

        for( int i = 0; i < mStrModeMastering.length; i++ ) {
            if( mMasteringMode.equals(mStrModeMastering[i]) ) {
                mSpinnerMastering.setSelection(i);

                if( !IsMasterMode() ) {
                    for( int j = 0; j < mSeekBarMastering.length; j++ ) {
                        mSeekBarMastering[j].setEnabled(false);
                        mValueMastering[j].setEnabled(false);
                        mSeekBarMastering[j].setProgress( mDataMastering[mMasteringModePos][j] );
                        mValueMastering[j].setText( String.valueOf(mDataMastering[mMasteringModePos][j]) );
                    }
                }
                else {
                    for( int j = 0; j < mSeekBarMastering.length; j++ ) {
                        mSeekBarMastering[j].setEnabled(true);
                        mValueMastering[j].setEnabled(true);
                    }
                    new AsyncTaskMasteringRead().execute();
                }
                break;
            }
        }
    }

    private void UpdateUniformityCorrection() {
        InitWhiteSeamValue();

        String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
        String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
        String[] internalPfpga = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        String[] internalUniformity =FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);

        if( (usbPfpga != null && usbPfpga.length != 0) || (usbUniformity != null && usbUniformity.length != 0) ) {
            mBtnUpdateUniformity.setEnabled(true);
        }

        if( (internalPfpga != null && internalPfpga.length != 0) || (internalUniformity != null && internalUniformity.length != 0) ) {
            mBtnApplyUniformity.setEnabled(true);
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] resultEnable = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_READ, ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN, NxCinemaCtrl.FORMAT_INT16) );
        if( (resultEnable != null) && resultEnable[3] == (byte)1 ) {
            mBtnEnableUniformity.setText("ENABLE");
        }
        else {
            mBtnEnableUniformity.setText("DISABLE");
        }

        //
        //
        //
        mSpinnerUniformity.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[]{ " " }));
        mSpinnerUniformity.setEnabled(false);

        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        for( String file : resultPath ) {
            ConfigPfpgaInfo info = new ConfigPfpgaInfo();
            if( info.Parse( file ) ) {
                if( info.GetModeNum() != 0 ) {
                    String[] strTemp = new String[info.GetModeNum()];
                    for( int i = 0; i < info.GetModeNum(); i++ )
                    {
                        strTemp[i] = String.format(Locale.US, "%d", i+1);
                    }
                    mSpinnerUniformity.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strTemp));
                    mSpinnerUniformity.setEnabled(true);
                }
            }
        }
    }

    private void UpdateImageQuality() {
        InitWhiteSeamValue();

        //
        //  Update Button
        //
        String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
        String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
//        String[] internalQuality = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
//        String[] internalGamma = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);

        if( (usbTcon != null && usbTcon.length != 0) || (usbGamma != null && usbGamma.length != 0) ) {
            mBtnUpdateImageQuality.setEnabled(true);
        }

//        if( (internalQuality != null && internalQuality.length != 0) || (internalGamma != null && internalGamma.length != 0) ) {
//            mBtnApplyImageQuality.setEnabled(true);
//        }

        //
        //
        //
        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        mAdapterMode.clear();

        for( String file : resultPath ) {
            if( mTconEEPRomInfo.Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconEEPRomInfo.GetModeNum() )
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i+1),
                                        String.format(Locale.US, "%s", mTconEEPRomInfo.GetDescription(i)),
                                        mTextbuttonAdapterClickListener )
                        );
                    else
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 1), "")
                        );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, "mode #%d", i + 1), "")
                    );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( mTconUsbInfo.Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < mTconUsbInfo.GetModeNum() )
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 11),
                                        String.format(Locale.US, "%s", mTconEEPRomInfo.GetDescription(i)),
                                        mTextbuttonAdapterClickListener )
                        );
                    else
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, "mode #%d", i + 11), "")
                        );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, "mode #%d", i + 11), "")
                    );

                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }
    }

    private void UpdateDotCorrection() {
        InitWhiteSeamValue();

        String[] result = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( result == null || result.length == 0 )
            return;

        new AsyncTaskAdapterDotCorrection(mAdapterDotCorrect).execute();
        mBtnDotCorrectCheckAll.setEnabled(true);
        mBtnDotCorrectUnCheckAll.setEnabled(true);
        mBtnDotCorrectApply.setEnabled(true);
    }

    private void InitWhiteSeamValue() {
        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(null);
        mCheckWhiteSeamEmulation.setChecked(false);

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

        byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Log.i(VD_DTAG, String.format(Locale.US, "Change WhiteSeam Emulation. ( %b )", mCheckWhiteSeamEmulation.isChecked()));

                mBtnWhiteSeamApply.setEnabled(b);
                for( VdSpinCtrl spin : mSpinWhiteSeam ) {
                    spin.setEnabled(b);
                }

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

                if( mSpinnerWhiteSeamCabinetId.getSelectedItemPosition() == 0 ) {
                    boolean bValidPort0 = false, bValidPort1 = false;
                    for( byte id : mCabinet ) {
                        if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                        if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                    }

                    byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(mCheckWhiteSeamEmulation.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                }
                else {
                    int pos = mSpinnerWhiteSeamCabinetId.getSelectedItemPosition() - 1;
                    byte slave = ((mCabinet[pos] % 16) < 8) ? (mCabinet[pos]) : (byte)(mCabinet[pos] | 0x80);
                    byte[] inData;
                    inData = new byte[] { slave };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16));
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mCheckWhiteSeamEmulation.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16));

                    ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                }

                new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
            }
        });
    }

    private void UpdateWhiteSeamValue() {
        InitWhiteSeamValue();

        mBtnWhiteSeamApply.setEnabled(mCheckWhiteSeamEmulation.isChecked());
        for( VdSpinCtrl spin : mSpinWhiteSeam )
            spin.setEnabled(mCheckWhiteSeamEmulation.isChecked());

        new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
    }

    private void UpdateDotCorrectionExtract() {
        InitWhiteSeamValue();

        String result = GetExternalStorage();
        if( result != null ) {
            mBtnDotCorrectExtract.setEnabled(true);
        }
        else {
            mBtnDotCorrectExtract.setEnabled(false);
        }
    }

    private void UpdateGlobal() {
        InitWhiteSeamValue();
        new AsyncTaskGlobalRead().execute();
    }

    private void UpdateSetup() {
        InitWhiteSeamValue();

        mEditCabinet.setText(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));

        for( int i = 0; i < CinemaService.OFF_TIME.length; i++ ) {
            if( ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_SAVING).equals(CinemaService.OFF_TIME[i]) ) {
                mSpinnerSuspendTime.setSelection(i);
                break;
            }
        }
    }

    //
    //
    //
    private void WriteHelper(String message) throws IOException {
        LocalSocket sender = new LocalSocket();
        sender.connect(new LocalSocketAddress("cinema.helper"));
        sender.getOutputStream().write(message.getBytes());
        sender.getOutputStream().close();
    }

    private String GetExternalStorage() {
        String[] result = FileManager.GetExternalPath();
        return (result != null && result.length != 0) ? result[0] : null;
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
    //
    //
    private void AddViewMastering( int index, View[] childView, String[][] textMastering ) {
        mParentLayoutMastering.addView(childView[index]);

        final int itemIndex = index;
        final LinearLayout layoutText = (LinearLayout)childView[index].findViewById(R.id.layoutTextMastering0);
        final TextView textView0    = (TextView)childView[index].findViewById(R.id.textMastering0);
        final TextView textView1    = (TextView)childView[index].findViewById(R.id.textMastering1);
        mValueMastering[itemIndex]  = (EditText)childView[index].findViewById(R.id.editMastering);
        mSeekBarMastering[itemIndex]= (SeekBar)childView[index].findViewById(R.id.seekMastering);
        mBtnMastering[itemIndex]    = (Button)childView[index].findViewById(R.id.btnMastering);

        if( textMastering[index][0].equals("CC00") ) {
            textView0.setText("Gamut");
        }
        else if( textMastering[index][0].equals("Global") ) {
            layoutText.setBackgroundResource( R.drawable.draw_line_top_right );
            layoutText.setPadding(0, 15, 0, 15);
            textView0.setText("Brightness");
        }

        textView1.setText( textMastering[index][0] );
        mSeekBarMastering[itemIndex].setMax( Integer.parseInt( textMastering[index][1] ) );

        mSeekBarMastering[itemIndex].setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mValueMastering[itemIndex].setText(String.valueOf(progress));

                int pos = mSpinnerMastering.getSelectedItemPosition();
                if( mDataMastering[pos][itemIndex] == progress && mBtnMastering[itemIndex].isEnabled() ) {
                    mBtnMastering[itemIndex].setEnabled(false);
                }
                if( mDataMastering[pos][itemIndex] != progress && !mBtnMastering[itemIndex].isEnabled() ) {
                    if( IsMasterMode() ) {
                        mBtnMastering[itemIndex].setEnabled(true);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mBtnMastering[itemIndex].setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDataMastering[mMasteringModePos][itemIndex] = Integer.parseInt( mValueMastering[itemIndex].getText().toString() );
                ApplyMasteringMode( itemIndex, mDataMastering[mMasteringModePos][itemIndex] );
                mBtnMastering[itemIndex].setEnabled( false );
            }
        });

        mValueMastering[itemIndex].setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                switch (i) {
                    case EditorInfo.IME_ACTION_DONE:
                        String strValue = mValueMastering[itemIndex].getText().toString();
                        if( strValue.equals("") ) {
                            strValue = "0";
                        }

                        if( Integer.parseInt(strValue, 10) > mSeekBarMastering[itemIndex].getMax() ) {
                            strValue = String.valueOf(mSeekBarMastering[itemIndex].getMax());
                        }

                        mValueMastering[itemIndex].setText(strValue);
                        mSeekBarMastering[itemIndex].setProgress(Integer.parseInt(strValue, 10));

                        int pos = mSpinnerMastering.getSelectedItemPosition();
                        int progress = mSeekBarMastering[itemIndex].getProgress();
                        if( mDataMastering[pos][itemIndex] == progress && mBtnMastering[itemIndex].isEnabled() ) {
                            mBtnMastering[itemIndex].setEnabled(false);
                        }
                        if( mDataMastering[pos][itemIndex] != progress && !mBtnMastering[itemIndex].isEnabled() ) {
                            if( IsMasterMode() ) {
                                mBtnMastering[itemIndex].setEnabled(true);
                            }
                        }
                        break;
                    default:
                        return false;
                }
                return false;
            }
        });
    }

    //
    //
    //
    private void ApplyMasteringMode( int itemIdx, int value ) {
        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

        byte[] reg = ctrl.IntToByteArray(mRegMastering[itemIdx], NxCinemaCtrl.FORMAT_INT16);
        byte[] data = ctrl.IntToByteArray(value, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg ,data);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
    }

    //
    //
    //
    private class AsyncTaskCheckCabinet extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < 0x10 )
                    continue;

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
                if (result == null || result.length == 0)
                    continue;

                if( 0 > result[0] )
                    continue;

                ((CinemaInfo)getApplicationContext()).AddCabinet( (byte)i );
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "AsyncTaskCheckCabinet Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
            ((CinemaInfo)getApplicationContext()).ClearCabinet();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            ((CinemaInfo)getApplicationContext()).SortCabinet();

            byte[] cabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
            int curCabinetNum = Integer.parseInt( ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM ) );

            if( cabinet.length != curCabinetNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinet() ) {
                ShowMessage( String.format(Locale.US, "Please check cabinet number. ( value: %d, detect: %d )", curCabinetNum, cabinet.length) );
            }
            else {
                String targetCabinetNum = mEditCabinet.getText().toString();

                CinemaInfo info = (CinemaInfo)getApplicationContext();
                info.SetValue(CinemaInfo.KEY_CABINET_NUM, targetCabinetNum);

                String strLog = String.format( "Change cabinet number. ( %s -> %s )", curCabinetNum, targetCabinetNum  );
                info.InsertLog(strLog);
                Log.i(VD_DTAG, strLog);
            }

            Log.i(VD_DTAG, "AsyncTaskCheckCabinet Done.");
            CinemaLoading.Hide();
        }
    }

    //
    //
    //
    private class AsyncTaskMasteringWrite extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( IsMasterMode() ) {
                for( int i = 0; i < mRegHideMastering.length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT16 );
                    byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }
                return null;
            }

            for( int i = 0; i < mRegHideMastering.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(reg, data);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
            }

            for( int i = 0; i < mRegMastering.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray( mRegMastering[i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] data = ctrl.IntToByteArray( mDataMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(reg, data);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "AsyncTaskMasteringWrite Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            for( int i = 0; i < mSeekBarMastering.length; i++ ) {
                mSeekBarMastering[i].setProgress( mDataMastering[mMasteringModePos][i] );
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskMasteringWrite Done.");
        }
    }

    private class AsyncTaskMasteringRead extends AsyncTask<Void, Void, Void> {
        int[] mValue;

        @Override
        protected Void doInBackground(Void... voids) {
            if( !IsMasterMode() )
                return null;

            if( mCabinet == null || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for (int i = 0; i < mRegMastering.length; i++) {
                byte[] reg = ctrl.IntToByteArray( mRegMastering[i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(new byte[]{mCabinet[0]}, reg);
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );

                if( result == null || result.length == 0 )
                    continue;

                mValue[i] = ctrl.ByteArrayToInt(result);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "AsyncTaskMasteringRead Start.");
            CinemaLoading.Show( DisplayModeActivity.this );

            mValue = new int[mRegMastering.length];
            for( int i = 0; i < mValue.length; i++ ) {
                mValue[i] = 0;
            }
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            for (int i = 0; i < mSeekBarMastering.length; i++) {
                if( 0 > mValue[i] )
                    continue;

                mSeekBarMastering[i].setProgress(mValue[i]);
                mDataMastering[mMasteringModePos][i] = mValue[i];

                mBtnMastering[i].setEnabled(false);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskMasteringRead Done.");
        }
    }

    private class AsyncTaskUniformityCorrection extends AsyncTask<Void, Void, Void> {
        private int mIndexUniformity = mSpinnerUniformity.getSelectedItemPosition();

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            String[] result;
            int enableUniformity = 0;

            result = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : result ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    enableUniformity = info.GetEnableUpdateUniformity(mIndexUniformity);

                    for( int i = 0; i < info.GetRegister(mIndexUniformity).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mIndexUniformity)[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexUniformity)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            result = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
            for( String file : result ) {
                LedUniformityInfo info = new LedUniformityInfo();
                if( info.Parse(file) ) {
                    if( enableUniformity == 0 ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Uniformity Correction. ( %s )", file ));
                        continue;
                    }

                    byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                    ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                }
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "AsyncTaskUniformityCorrection Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskUniformityCorrection Done.");
        }
    }

    private class AsyncTaskImageQuality extends AsyncTask<Void, Void, Void> {
        private int mModeIndexQuality = 0;
        private int mUpdateGamma[] = new int[4];

        public AsyncTaskImageQuality(int mode) {
            mModeIndexQuality = mode;
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
            int[] enableGamma = {0, 0, 0, 0};

            resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : resultPath ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    for( int i = 0; i < info.GetRegister(mModeIndexQuality).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mModeIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mModeIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            //
            //  Parse T_REG.txt
            //
            if( (10 > mModeIndexQuality) && (mModeIndexQuality < mTconEEPRomInfo.GetModeNum())) {
                enableGamma = mTconEEPRomInfo.GetEnableUpdateGamma(mModeIndexQuality);
                for( int i = 0; i < mTconEEPRomInfo.GetRegister(mModeIndexQuality).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(mTconEEPRomInfo.GetRegister(mModeIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(mTconEEPRomInfo.GetData(mModeIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }
            }

            if( (10 <= mModeIndexQuality) && ((mModeIndexQuality-10) < mTconUsbInfo.GetModeNum())) {
                enableGamma = mTconUsbInfo.GetEnableUpdateGamma(mModeIndexQuality-10);
                for( int i = 0; i < mTconUsbInfo.GetRegister(mModeIndexQuality-10).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(mTconUsbInfo.GetRegister(mModeIndexQuality-10)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(mTconUsbInfo.GetData(mModeIndexQuality-10)[i], NxCinemaCtrl.FORMAT_INT16);
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
            if( (10 > mModeIndexQuality) && (mModeIndexQuality < mTconEEPRomInfo.GetModeNum())) {
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

            if( (10 <= mModeIndexQuality) ) {
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

            Log.i(VD_DTAG, "AsyncTaskImageQuality Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            mTextImageQuality.setText(String.format(Locale.US, "Mode #%d", mModeIndexQuality+1));
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mModeIndexQuality));

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskImageQuality Done.");
        }
    }

    private class AsyncTaskAdapterDotCorrection extends AsyncTask<Void, String, Void> {
        private CheckRunAdapter mAdapter;

        public AsyncTaskAdapterDotCorrection( CheckRunAdapter adapter ) {
            mAdapter = adapter;
            mAdapter.clear();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
            for( String dir : resultDir ) {
                if( isCancelled() ) {
                    return null;
                }
                publishProgress( dir );
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            String[] resultFile = FileManager.CheckFile(values[0], LedDotCorrectInfo.PATTERN_NAME);
            mAdapter.add( new CheckRunInfo(values[0].substring(values[0].lastIndexOf("/") + 1), String.format("total : %s", resultFile.length)) );
            Collections.sort(mAdapter.get(), new Comparator<CheckRunInfo>() {
                @Override
                public int compare(CheckRunInfo t0, CheckRunInfo t1) {
                    return (t0.GetTitle().compareTo(t1.GetTitle()) > 0) ? 1 : -1;
                }
            });

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "AsyncTaskAdapterDotCorrection Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskAdapterDotCorrection Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskDotCorrection extends AsyncTask<Void, Integer, Void> {
        private CheckRunAdapter mAdapter;

        public AsyncTaskDotCorrection( CheckRunAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
            if( resultDir == null || resultDir.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            String topdir = resultDir[0].substring(0, resultDir[0].lastIndexOf("/") + 1);

            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                int success = 0;
                int fail = 0;

                CheckRunInfo item = mAdapter.getItem(i);
                if( !item.GetChecked() )
                    continue;

                String[] result = FileManager.CheckFile(topdir + item.GetTitle(), LedDotCorrectInfo.PATTERN_NAME);
                for( String file : result ) {
                    Log.i(VD_DTAG, "Dot Correct Info : " + file);

                    LedDotCorrectInfo info = new LedDotCorrectInfo();
                    if( info.Parse(file) ) {
                        byte[] sel = ctrl.IntToByteArray( info.GetModule(), NxCinemaCtrl.FORMAT_INT8 );       // size: 1
                        byte[] data = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );    // size: 61440
                        byte[] inData =  ctrl.AppendByteArray( sel, data );

                        byte[] res = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION, ctrl.AppendByteArray( new byte[]{(byte)info.GetIndex()}, inData ) );
                        if( res == null || res.length == 0 ) {
                            publishProgress(i, result.length, success, ++fail);
                            continue;
                        }

                        if( res[0] == (byte)0xFF ) {
                            publishProgress(i, result.length, success, ++fail);
                            continue;
                        }
                        publishProgress(i, result.length, ++success, fail);
                    }
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            CheckRunInfo info = mAdapter.getItem(values[0]);
            info.SetDescription( String.format(Locale.US, "total: %d, success: %d, fail: %d", values[1], values[2], values[3]));
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "Dot Correction Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Dot Correction Done.");
        }
    }

    private class AsyncTaskDotCorrectionExtract extends AsyncTask<Void, Void, Void> {
        private int mId;
        private int mModule;

        public AsyncTaskDotCorrectionExtract( int id, int module ) {
            mId     = id;
            mModule = module;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            int start   = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule     : 0;
            int end     = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule + 1 : LedDotCorrectInfo.MAX_MODULE_NUM;
            int idx     = mId;

            for( int i = start; i < end; i++ ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Dot correction extract. ( slave: 0x%02X, module: %d )", (byte)idx, i) );

                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION_EXTRACT, new byte[]{(byte)idx, (byte)i} );
                if( result == null || result.length == 0)
                    continue;

                String strDir = String.format(Locale.US, "%s/DOT_CORRECTION_ID%03d", GetExternalStorage(), (idx & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                if( !FileManager.MakeDirectory( strDir ) ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Create Directory. ( %s )", strDir));
                    continue;
                }

                new LedDotCorrectInfo().Make(idx, i, result, strDir);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "Dot Correction Extract Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Dot Correction Extract Done. ");
        }
    }

    private class AsyncTaskWhiteSeamRead extends AsyncTask<Void, Integer, Void> {
        private byte[] mCabinet;
        private int mIndexPos;
        private boolean mEmulate;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal = new int[4];

        public AsyncTaskWhiteSeamRead(byte[] cabinet, int indexPos, boolean emulate) {
            Log.i(VD_DTAG, ">>> WhiteSeam Read Start.");

            mCabinet = cabinet;
            mIndexPos = indexPos;
            mEmulate = emulate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Emulate is
            //      true    : Emulate Register --> UI
            //      false   : Flash Register --> Emulate Register --> UI
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            int indexPos = (mIndexPos != 0) ? mIndexPos - 1 : 0;
            byte slave = mCabinet[indexPos];

            //
            //  1. Read White Seam Value in Flash Memory.
            //
            if( !mEmulate ) {
                Log.i(VD_DTAG, ">>> White Seam Read in Flash Memory.");
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_WHITE_SEAM_READ, new byte[] {slave} );
                if( result == null || result.length == 0 || result[0] != 0x01 ) {
                    Log.i(VD_DTAG, "Fail, WhiteSeam Read.");
                    return null;
                }
            }

            //
            //  2. Read White Seam Value in Emulate Register.
            //
            Log.i(VD_DTAG, ">>> White Seam Read in Emulate Register.");
            for( int i = 0; i < mSeamReg.length; i++ ) {
                byte[] result, inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[i], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", slave, mSeamReg[i] ));
                    return null;
                }

                mSeamVal[i] = ctrl.ByteArrayToInt(result);
            }

            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Read Done. ( pos: %d, slave: 0x%02X, emulate: %b, top: %d, bottom: %d, left: %d, right: %d )",
                    indexPos, slave, mEmulate, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));

            publishProgress(mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]);
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            for( int i = 0; i < mSeamVal.length; i++ ) {
                mSpinWhiteSeam[i].SetValue(values[i]);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "WhiteSeam Read Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            Log.i(VD_DTAG, "WhiteSeam Read Done.");
            CinemaLoading.Hide();
        }
    }

    private class AsyncTaskWhiteSeamEmulate extends AsyncTask<Void, Void, Void> {
        private byte[] mCabinet;
        private int mIndexPos;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal = new int[4];

        public AsyncTaskWhiteSeamEmulate(byte[] cabinet, int indexPos) {
            mCabinet = cabinet;
            mIndexPos = indexPos;

            for( int i = 0; i < mSeamVal.length; i++ )
                mSeamVal[i] = mSpinWhiteSeam[i].GetValue();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            Log.i(VD_DTAG, String.format(Locale.US, "pos: %d", mIndexPos));

            if( mIndexPos == 0 ) {
                Log.i(VD_DTAG, "WhiteSeam Emulate. ( index: all )");

                boolean bValidPort0 = false, bValidPort1 = false;
                for( byte id : mCabinet ) {
                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                }

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                    byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                    byte[] inData1 = new byte[] {(byte)0x09};
                    inData1 = ctrl.AppendByteArray(inData1, reg);
                    inData1 = ctrl.AppendByteArray(inData1, val);

                    byte[] inData2 = new byte[] {(byte)0x89};
                    inData2 = ctrl.AppendByteArray(inData2, reg);
                    inData2 = ctrl.AppendByteArray(inData2, val);

                    if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                }
            }
            else {
                int pos = mIndexPos - 1;
                byte slave = mCabinet[pos];
                Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Emulate. ( index: %d, slave: 0x%02x )", pos, slave));

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] inData;
                    inData = new byte[] { slave };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                    ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "WhiteSeam Emulate Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "WhiteSeam Emulation Done.");
        }
    }

    private class AsyncTaskWhiteSeamWrite extends  AsyncTask<Void, Void, Void> {
        private byte[] mCabinet;
        private int mIndexPos;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal = new int[4];

        public AsyncTaskWhiteSeamWrite(byte[] cabinet, int indexPos) {
            mCabinet = cabinet;
            mIndexPos = indexPos;

            for( int i = 0; i < mSeamVal.length; i++ )
                mSeamVal[i] = mSpinWhiteSeam[i].GetValue();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            //
            //  1. Update White Seam in Emulate Register.
            //
            if( mIndexPos == 0 ) {
                boolean bValidPort0 = false, bValidPort1 = false;
                for( byte id : mCabinet ) {
                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                }

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                    byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                    byte[] inData1 = new byte[] {(byte)0x09};
                    inData1 = ctrl.AppendByteArray(inData1, reg);
                    inData1 = ctrl.AppendByteArray(inData1, val);

                    byte[] inData2 = new byte[] {(byte)0x89};
                    inData2 = ctrl.AppendByteArray(inData2, reg);
                    inData2 = ctrl.AppendByteArray(inData2, val);

                    if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                }
            }
            else {
                int pos = mIndexPos - 1;
                byte slave = mCabinet[pos];
                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] inData;
                    inData = new byte[] { slave };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                    ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                }
            }


            //
            //  2. Update White Seam in Flash Data.
            //
            int start   = (mIndexPos != 0) ? mIndexPos - 1  : 0;
            int end     = (mIndexPos != 0) ? mIndexPos      : mCabinet.length;

            for( int i = start; i < end; i++ ) {
                byte slave = mCabinet[i];
                byte[] result, inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[0], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[1], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[2], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[3], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send(NxCinemaCtrl.CMD_TCON_WHITE_SEAM_WRITE, inData);
                if( result == null || result.length == 0 || ctrl.ByteArrayToInt(result) != 0x01 ) {
                    Log.i(VD_DTAG, "Fail, Write WhiteSeam.");
                    return null;
                }

                Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Write. ( pos: %d, slave: 0x%02x, top: %d, bottom: %d, left: %d, right: %d )",
                        i, slave, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "WhiteSeam Write Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "WhiteSeam Write Done.");
        }
    }

    private class AsyncTaskGlobalRead extends AsyncTask<Void, Integer, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            int [] globalReg = { 0x018B, 0x018C, 0x018A, 0x018D, 0x018E, 0x0192, 0x0055, 0x0004, 0x0100, 0x011E };
            int [] globalVal = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

            for( int i = 0; i < globalReg.length; i++ )
            {
                byte[] result, inData;
                inData = new byte[] { mCabinet[0] };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(globalReg[i], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], globalReg[i] ));
                    return null;
                }

                globalVal[i] = ctrl.ByteArrayToInt(result);
            }

            publishProgress( globalVal[0], globalVal[1], globalVal[2], globalVal[3], globalVal[4], globalVal[5], globalVal[6], globalVal[7], globalVal[8], globalVal[9] );
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mSpinSyncWidth.SetValue( values[0] );
            mSpinSyncDelay.SetValue( values[1] );

            mCheckSyncReverse.setChecked( values[2] != 0 ) ;
            mCheckScale.setChecked( values[3] != 0 );
            mCheckZeroScale.setChecked( values[4] != 0 );
            mCheckSeam.setChecked( values[5] != 0 );
            mCheckModule.setChecked( values[6] != 0 );
            mCheckXyzInput.setChecked( values[7] != 0 );
            mCheckLedOpenDetection.setChecked( values[8] != 0 );
            mCheckLodRemoval.setChecked( values[9] != 0 );

            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(values[0]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(values[1]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf(values[2]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018D, String.valueOf(values[3] == 0 ? 0 : 1) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_PREG_0x0199, String.valueOf(values[3] == 0 ? 1 : 0) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018E, String.valueOf(values[4]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0192, String.valueOf(values[5]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0055, String.valueOf(values[6]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0004, String.valueOf(values[7]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0100, String.valueOf(values[8]) );
            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf(values[9]) );
            ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            UnregisterListener();

            Log.i(VD_DTAG, ">>> Global Read Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            RegisterListener();

            Log.i(VD_DTAG, ">>> Global Read Done.");
            CinemaLoading.Hide();
        }
    }

    private class AsyncTaskLodReset extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            byte[] reg, dat, inData, inData0, inData1;
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            Log.i(VD_DTAG, "Lod Reset Start.");
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Lod Reset Done.");
        }
    }

    void ApplySyncWidth() {
        Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_WIDTH. ( %d )", mSpinSyncWidth.GetValue()));

        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] reg = ctrl.IntToByteArray(0x018B, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mSpinSyncWidth.GetValue(), NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncDelay.GetValue()) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplySyncDelay() {
        Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] reg = ctrl.IntToByteArray(0x018C, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mSpinSyncDelay.GetValue(), NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplySyncReverse() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change SYNC_REVERSE. ( %b )", mCheckSyncReverse.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] reg = ctrl.IntToByteArray(0x018A, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mCheckSyncReverse.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf(mCheckSyncReverse.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyScale() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Scale. ( %b )", mCheckScale.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        {
            byte[] reg = ctrl.IntToByteArray(0x0199, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckScale.isChecked() ? 0x0000 : 0x0001, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
        }
        {
            byte[] reg = ctrl.IntToByteArray(0x018D, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckScale.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_PREG_0x0199, String.valueOf(mCheckScale.isChecked() ? 0 : 1) );
        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018D, String.valueOf(mCheckScale.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyZeroScale() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Zero Scale. ( %b )", mCheckZeroScale.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        {
            byte[] reg = ctrl.IntToByteArray(0x018E, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckZeroScale.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018E, String.valueOf(mCheckZeroScale.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplySeam() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Seam. ( %b )", mCheckSeam.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        {
            byte[] reg = ctrl.IntToByteArray(0x0192, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckSeam.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0192, String.valueOf(mCheckSeam.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyModule() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Module. ( %b )", mCheckModule.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        {
            byte[] reg = ctrl.IntToByteArray(0x0055, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckModule.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0055, String.valueOf(mCheckModule.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyXyzInput() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change XyzInput. ( %b )", mCheckXyzInput.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        {
            byte[] reg = ctrl.IntToByteArray(0x0004, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckXyzInput.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0004, String.valueOf(mCheckXyzInput.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyLedOpenDetection() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Led Open Detection. ( %b )", mCheckLedOpenDetection.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        {
            byte[] reg = ctrl.IntToByteArray(0x0100, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckLedOpenDetection.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0100, String.valueOf(mCheckLedOpenDetection.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyLodRemoval() {
        Log.i(VD_DTAG, String.format(Locale.US, "Change Lod Removal. ( %b )", mCheckLodRemoval.isChecked()));
        if( mCabinet.length == 0 )
            return ;

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        {
            byte[] reg = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckLodRemoval.isChecked() ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf(mCheckLodRemoval.isChecked() ? 1 : 0) );
        ((CinemaInfo)getApplicationContext()).UpdateDefaultRegister();
    }

    void ApplyLodReset() {
        new AsyncTaskLodReset().execute();
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
        private WeakReference<DisplayModeActivity> mActivity;

        public UIHandler( DisplayModeActivity activity ) {
            mActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            DisplayModeActivity activity = mActivity.get();
            if( activity != null ) {
                activity.handleMessage(msg);
            }
        }
    }

    private void handleMessage( Message msg ) {
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        int mode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
        mTextImageQuality.setText(String.format(Locale.US, "Mode #%d", mode+1));
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
