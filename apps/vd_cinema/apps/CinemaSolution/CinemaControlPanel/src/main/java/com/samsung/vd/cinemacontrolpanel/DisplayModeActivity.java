package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
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

import com.samsung.vd.baseutils.VdSpinCtrl;
import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.IOException;
import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DisplayModeActivity";

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
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_PIXEL_CORRECTION_ADAPTER,
                            getApplicationContext(),
                            mAdapterDotCorrect,
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {
                                    HideProgress();
                                }

                                @Override
                                public void onPostExecute(int[] values) {

                                }
                            }
                    );

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
        setContentView(R.layout.activity_display_mode);

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
                ClearWhiteSeamValue();
                Launch(v.getContext(), TopActivity.class);
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ClearWhiteSeamValue();
                TurnOff();
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

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        mRegHideMastering,
                        mDataHideMastering[mMasteringModePos],
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                Log.i(VD_DTAG, "Mastering Write Start. ( Hidden Register Section )");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                                Log.i(VD_DTAG, "Mastering Write Done. ( Hidden Register Section )");
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );

                if( !IsMasterMode() ) {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_TCON_REG_WRITE,
                            getApplicationContext(),
                            mRegMastering,
                            mDataMastering[mMasteringModePos],
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    Log.i(VD_DTAG, "Mastering Write Start.");
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {
                                    HideProgress();
                                    Log.i(VD_DTAG, "Mastering Write Done.");
                                }

                                @Override
                                public void onPostExecute(int[] values) {

                                }
                            }
                    );
                }
                else {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_TCON_REG_READ,
                            getApplicationContext(),
                            mRegMastering,
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    Log.i(VD_DTAG, "Mastering Read Start.");
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {

                                }

                                @Override
                                public void onPostExecute(int[] values) {
                                    for( int i = 0; i < values.length; i++ ) {
                                        mSeekBarMastering[i].setProgress(values[i]);
                                        mDataMastering[mMasteringModePos][i] = values[i];
                                        mBtnMastering[i].setEnabled(false);
                                    }

                                    HideProgress();
                                    Log.i(VD_DTAG, "Mastering Read Done.");
                                }
                            }
                    );
                }

                UpdateMasteringMode();
                ((CinemaInfo)getApplicationContext()).InsertLog(String.format( Locale.US, "Change Mastering Mode. ( %s Mode )", mMasteringMode ));
            }
        });


        //
        //
        //
        mLayoutMastering = new LinearLayout[mTextMastering.length];
        for( int i = 0; i < mTextMastering.length; i++ ) {
            mLayoutMastering[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_mastering, mParentLayoutMastering, false );
            AddViewMastering( i, mLayoutMastering, mTextMastering );
        }

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
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_UNIFORMITY,
                        getApplicationContext(),
                        mSpinnerUniformity.getSelectedItemPosition(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
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
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PIXEL_CORRECTION,
                        getApplicationContext(),
                        mAdapterDotCorrect,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
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

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PIXEL_CORRECTION_EXTRACT,
                        getApplicationContext(),
                        index,
                        module,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
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
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_WHITESEAM_READ,
                        getApplicationContext(),
                        mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                        mCheckWhiteSeamEmulation.isChecked(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                for( int i = 0; i < values.length; i++ ) {
                                    mSpinWhiteSeam[i].SetValue(values[i]);
                                }
                                HideProgress();
                            }
                        }
                );
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
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_WHITESEAM_EMULATE,
                            getApplicationContext(),
                            mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                            new int [] {
                                    mSpinWhiteSeam[0].GetValue(),
                                    mSpinWhiteSeam[1].GetValue(),
                                    mSpinWhiteSeam[2].GetValue(),
                                    mSpinWhiteSeam[3].GetValue(),
                            },
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {
                                    HideProgress();
                                }

                                @Override
                                public void onPostExecute(int[] values) {

                                }
                            }
                    );

                }
            });
            spin.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_WHITESEAM_EMULATE,
                            getApplicationContext(),
                            mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                            new int [] {
                                    mSpinWhiteSeam[0].GetValue(),
                                    mSpinWhiteSeam[1].GetValue(),
                                    mSpinWhiteSeam[2].GetValue(),
                                    mSpinWhiteSeam[3].GetValue(),
                            },
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {
                                    HideProgress();
                                }

                                @Override
                                public void onPostExecute(int[] values) {

                                }
                            }
                    );
                    return false;
                }
            });
        }

        mBtnWhiteSeamApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_WHITESEAM_WRITE,
                        getApplicationContext(),
                        mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                        new int [] {
                                mSpinWhiteSeam[0].GetValue(),
                                mSpinWhiteSeam[1].GetValue(),
                                mSpinWhiteSeam[2].GetValue(),
                                mSpinWhiteSeam[3].GetValue(),
                        },
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
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
        String desireCabinetNum = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM);

        mEditCabinet = (EditText)findViewById(R.id.editCabinet);
        mEditCabinet.setText(desireCabinetNum);

        Button btnCabinetNum = (Button)findViewById(R.id.btnCabinetNumApply);
        btnCabinetNum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_CHECK_CABINET_NUM,
                        getApplicationContext(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                int detectCabinetNum = values[0];
                                int desireCabinetNum = Integer.parseInt( mEditCabinet.getText().toString() );
                                int curCabinetNum = Integer.parseInt( ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM ) );

                                if( detectCabinetNum != desireCabinetNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinetNum() ) {
                                    ShowMessage( String.format(Locale.US, "Please check cabinet number. ( desire: %d, detect: %d )", desireCabinetNum, detectCabinetNum) );
                                }
                                else {
                                    CinemaInfo info = (CinemaInfo)getApplicationContext();
                                    info.SetValue(CinemaInfo.KEY_CABINET_NUM, mEditCabinet.getText().toString());

                                    String strLog = String.format( "Change cabinet number. ( %s -> %s )", curCabinetNum, desireCabinetNum  );
                                    info.InsertLog(strLog);
                                    Log.i(VD_DTAG, strLog);
                                }
                                HideProgress();
                            }
                        }
                );
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
                RefreshScreenSaver();
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

        this.RegisterTmsCallback(new CinemaService.TmsEventCallback() {
            @Override
            public void onTmsEventCallback() {
                String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
                int mode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
                mTextImageQuality.setText(String.format(Locale.US, "Mode #%d", mode+1));
            }
        });

        UpdateMasteringMode();
    }

    private void RegisterListener() {
        mBtnApplySyncWidth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mSpinSyncWidth.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mSpinSyncWidth.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
                return false;
            }
        });

        mBtnApplySyncDelay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mSpinSyncDelay.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mSpinSyncDelay.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );

                return false;
            }
        });

        mCheckSyncReverse.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018A},
                        new int[]{mCheckSyncReverse.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change SYNC_REVERSE. ( %b )", mCheckSyncReverse.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf(mCheckSyncReverse.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckScale.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PFPGA_MUTE,
                        getApplicationContext(),
                        true,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        null
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PFPGA_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0199},
                        new int[]{mCheckScale.isChecked() ? 0x0000 : 0x0001},
                        null,
                        null
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018D},
                        new int[]{mCheckScale.isChecked() ? 0x0001 : 0x0000},
                        null,
                        null
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PFPGA_MUTE,
                        getApplicationContext(),
                        false,
                        null,
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_PREG_0x0199, String.valueOf(mCheckScale.isChecked() ? 0 : 1) );
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018D, String.valueOf(mCheckScale.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckZeroScale.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x018E},
                        new int[]{mCheckZeroScale.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Zero Scale. ( %b )", mCheckZeroScale.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x018E, String.valueOf(mCheckZeroScale.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckSeam.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0192},
                        new int[]{mCheckSeam.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Seam. ( %b )", mCheckSeam.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0192, String.valueOf(mCheckSeam.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckModule.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0055},
                        new int[]{mCheckModule.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Module. ( %b )", mCheckModule.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0055, String.valueOf(mCheckModule.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckXyzInput.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0004},
                        new int[]{mCheckXyzInput.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change XYX Input. ( %b )", mCheckXyzInput.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x0004, String.valueOf(mCheckXyzInput.isChecked() ? 1 : 0) );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckLedOpenDetection.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0100},
                        new int[]{mCheckLedOpenDetection.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Led Open Detection. ( %b )", mCheckLedOpenDetection.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                ((CinemaInfo)getApplicationContext()).SetValue(
                                        CinemaInfo.KEY_TREG_0x0100,
                                        String.valueOf(mCheckLedOpenDetection.isChecked() ? 1 : 0)
                                );
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        mCheckLodRemoval.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x011E},
                        new int[]{mCheckLodRemoval.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf(mCheckLodRemoval.isChecked() ? 1 : 0) );
                                HideProgress();
                            }
                        }
                );
            }
        });

        mBtnLodReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_LOD_RESET,
                        getApplicationContext(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf(mCheckLodRemoval.isChecked() ? 1 : 0) );
                                HideProgress();
                            }
                        }
                );
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
            final int mode = position;
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_MODE,
                    getApplicationContext(),
                    position,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute() {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute() {
                            mTextImageQuality.setText(String.format(Locale.US, "Mode #%d", mode+1));
                            ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mode));
                            HideProgress();
                        }

                        @Override
                        public void onPostExecute(int[] values) {

                        }
                    }
            );
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
        ClearWhiteSeamValue();

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

                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_TCON_REG_READ,
                            getApplicationContext(),
                            mRegMastering,
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute() {
                                    Log.i(VD_DTAG, "Mastering Read Start.");
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute() {

                                }

                                @Override
                                public void onPostExecute(int[] values) {
                                    for( int i = 0; i < values.length; i++ ) {
                                        mSeekBarMastering[i].setProgress(values[i]);
                                        mDataMastering[mMasteringModePos][i] = values[i];
                                        mBtnMastering[i].setEnabled(false);
                                    }

                                    HideProgress();
                                    Log.i(VD_DTAG, "Mastering Read Done.");
                                }
                            }
                    );
                }
                break;
            }
        }
    }

    private void UpdateUniformityCorrection() {
        ClearWhiteSeamValue();

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
        ClearWhiteSeamValue();

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
        ClearWhiteSeamValue();

        String[] result = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( result == null || result.length == 0 )
            return;

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PIXEL_CORRECTION_ADAPTER,
                getApplicationContext(),
                mAdapterDotCorrect,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );

        mBtnDotCorrectCheckAll.setEnabled(true);
        mBtnDotCorrectUnCheckAll.setEnabled(true);
        mBtnDotCorrectApply.setEnabled(true);
    }

    private void ClearWhiteSeamValue() {
        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(null);
        mCheckWhiteSeamEmulation.setChecked(false);

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                new int[] { 0x0189 },
                new int[] { 0x0000 },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );

        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Log.i(VD_DTAG, String.format(">>> onCheckedChanged() ( %b )", b));
                Log.i(VD_DTAG, String.format(">>> onCheckedChanged() ( %b )", mCheckWhiteSeamEmulation.isChecked()));
                mBtnWhiteSeamApply.setEnabled(b);
                for( VdSpinCtrl spin : mSpinWhiteSeam ) {
                    spin.setEnabled(b);
                }

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_WHITESEAM_ENABLE,
                        getApplicationContext(),
                        mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                        b,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {

                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_WHITESEAM_READ,
                        getApplicationContext(),
                        mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                        b,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {

                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                for( int i = 0; i < values.length; i++ ) {
                                    mSpinWhiteSeam[i].SetValue(values[i]);
                                }
                            }
                        }
                );
            }
        });
    }

    private void UpdateWhiteSeamValue() {
        ClearWhiteSeamValue();

        mBtnWhiteSeamApply.setEnabled(mCheckWhiteSeamEmulation.isChecked());
        for( VdSpinCtrl spin : mSpinWhiteSeam )
            spin.setEnabled(mCheckWhiteSeamEmulation.isChecked());

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_WHITESEAM_READ,
                getApplicationContext(),
                mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                mCheckWhiteSeamEmulation.isChecked(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {

                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {

                    }

                    @Override
                    public void onPostExecute(int[] values) {
                        for( int i = 0; i < values.length; i++ ) {
                            mSpinWhiteSeam[i].SetValue(values[i]);
                        }
                    }
                }
        );
    }

    private void UpdateDotCorrectionExtract() {
        ClearWhiteSeamValue();

        String result = GetExternalStorage();
        if( result != null ) {
            mBtnDotCorrectExtract.setEnabled(true);
        }
        else {
            mBtnDotCorrectExtract.setEnabled(false);
        }
    }

    private void UpdateGlobal() {
        ClearWhiteSeamValue();

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                new int[] {
                        NxCinemaCtrl.REG_TCON_0x018B,
                        NxCinemaCtrl.REG_TCON_0x018C,
                        NxCinemaCtrl.REG_TCON_0x018A,
                        NxCinemaCtrl.REG_TCON_0x018D,
                        NxCinemaCtrl.REG_TCON_0x018E,
                        NxCinemaCtrl.REG_TCON_0x0192,
                        NxCinemaCtrl.REG_TCON_0x0055,
                        NxCinemaCtrl.REG_TCON_0x0004,
                        NxCinemaCtrl.REG_TCON_0x0100,
                        NxCinemaCtrl.REG_TCON_0x011E
                },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        Log.i(VD_DTAG, ">> Global Register Read Start.");
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {

                    }

                    @Override
                    public void onPostExecute(int[] values) {
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
                        Log.i(VD_DTAG, ">> Global Register Read Done.");
                        HideProgress();
                    }
                }
        );
    }

    private void UpdateSetup() {
        ClearWhiteSeamValue();

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

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{mRegMastering[itemIndex]},
                        new int[]{mDataMastering[mMasteringModePos][itemIndex]},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );

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
}
