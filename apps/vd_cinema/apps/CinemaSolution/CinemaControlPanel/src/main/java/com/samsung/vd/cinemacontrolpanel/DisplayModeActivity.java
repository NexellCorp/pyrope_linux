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
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
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
import java.util.Collections;
import java.util.Comparator;
import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
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

    private Spinner mSpinnerImageQuality;
    private Button mBtnUpdateImageQuality;
    private Button mBtnApplyImageQuality;

    private CheckRunAdapter mAdapterDotCorrect;

    private EditText mEditCabinet;

    private Button mBtnDotCorrectCheckAll;
    private Button mBtnDotCorrectUnCheckAll;
    private Button mBtnDotCorrectApply;

    private Spinner mSpinnerDotCorrectExtractId;
    private Spinner mSpinnerDotCorrectExtractModule;
    private Button mBtnDotCorrectExtract;

    private Spinner mSpinnerWhiteSeamCabinetId;

    private VdSpinCtrl mSpinWhiteSeamTop;
    private VdSpinCtrl mSpinWhiteSeamBottom;
    private VdSpinCtrl mSpinWhiteSeamLeft;
    private VdSpinCtrl mSpinWhiteSeamRight;

    private Button mBtnWhiteSeamTopApply;
    private Button mBtnWhiteSeamBottomApply;
    private Button mBtnWhiteSeamLeftApply;
    private Button mBtnWhiteSeamRightApply;

    private Button mBtnWhiteSeamEnable;
    private Button mBtnWhiteSeamTest;

    private Spinner mSpinnerSuspendTime;
    private boolean mWhiteSeamTest = false;

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
                startActivity( new Intent(v.getContext(), TopActivity.class) );
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
        //  Cabinet ID & Module Number String
        //
        String [] strCabinetId;
        if( mCabinet.length == 0 ) {
            strCabinetId = new String[]{""};
        }
        else {
            strCabinetId = new String[mCabinet.length];
            for( int i = 0; i < mCabinet.length; i++ ) {
                strCabinetId[i] = String.valueOf(mCabinet[i]&0x7F - CinemaInfo.TCON_ID_OFFSET);
            }
        }

        String [] strModule =  new String[25];
        strModule[0] = "ALL";
        for( int i = 0; i < 24; i++ ) {
            strModule[i+1] = String.format(Locale.US, "MODULE #%d", i);
        }

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
        mSpinnerUniformity.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[]{"1", "2", "3", "4"}));

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
                    ShowMessage( "Update Image Quality File.");
                    UpdateImageQuality();
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

                    byte[] res = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                    mBtnEnableUniformity.setText("DISABLE");
                }
                else {
                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN , NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    byte[] res = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

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
        mSpinnerImageQuality = (Spinner)findViewById(R.id.spinnerImageQuality);
        mSpinnerImageQuality.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, new String[]{"1", "2", "3", "4"}));

        mBtnUpdateImageQuality = (Button)findViewById(R.id.btnUpdateImageQuality);
        mBtnUpdateImageQuality.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String[] resultTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                for( String path : resultTcon ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigTconInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedGammaInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultTcon.length != 0) || (resultGamma.length != 0) ) {
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
        mSpinnerDotCorrectExtractId.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strCabinetId) );

        mSpinnerDotCorrectExtractModule = (Spinner)findViewById(R.id.spinnerDotCorrectionExtractModule);
        mSpinnerDotCorrectExtractModule.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strModule) );

        mBtnDotCorrectExtract = (Button)findViewById(R.id.btnDotCorrectionExtract);
        mBtnDotCorrectExtract.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String strIndex = mSpinnerDotCorrectExtractId.getSelectedItem().toString();
                if( strIndex == null || strIndex.equals("") )
                    return;

                int index = Integer.parseInt(strIndex, 10);
                int module= (mSpinnerDotCorrectExtractModule.getSelectedItemPosition() == 0 ) ?
                        LedDotCorrectInfo.MAX_MODULE_NUM : mSpinnerDotCorrectExtractModule.getSelectedItemPosition() - 1;

                new AsyncTaskDotCorrectionExtract(index, module).execute();
            }
        });


        //
        //  WHITE SEAM VALUE
        //
        mBtnWhiteSeamTopApply = (Button)findViewById(R.id.btnWhiteSeamTopApply);
        mBtnWhiteSeamBottomApply = (Button)findViewById(R.id.btnWhiteSeamBottomApply);
        mBtnWhiteSeamLeftApply = (Button)findViewById(R.id.btnWhiteSeamLeftApply);
        mBtnWhiteSeamRightApply = (Button)findViewById(R.id.btnWhiteSeamRightApply);

        mBtnWhiteSeamEnable = (Button)findViewById(R.id.btnWhiteSeamEnable);
        mBtnWhiteSeamTest = (Button)findViewById(R.id.btnWhiteSeamTest);

        mSpinnerWhiteSeamCabinetId = (Spinner)findViewById(R.id.spinnerWhiteSeamCabinetId);
        mSpinnerWhiteSeamCabinetId.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strCabinetId) );

        mSpinWhiteSeamTop = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamTop);
        mSpinWhiteSeamBottom = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamBottom);
        mSpinWhiteSeamLeft = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamLeft);
        mSpinWhiteSeamRight = (VdSpinCtrl)findViewById(R.id.spinWhiteSeamRight);

        mSpinWhiteSeamTop.SetRange(0, 32767);
        mSpinWhiteSeamBottom.SetRange(0, 32767);
        mSpinWhiteSeamLeft.SetRange(0, 32767);
        mSpinWhiteSeamRight.SetRange(0, 32767);

//        mSpinWhiteSeamTop.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
//            @Override
//            public void onChange(int value) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                if( mWhiteSeamTest )
//                    new AsyncTaskRegisterWrite( id, 0x0180, value ).execute();
//            }
//        });
//
//        mSpinWhiteSeamBottom.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
//            @Override
//            public void onChange(int value) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                if( mWhiteSeamTest )
//                    new AsyncTaskRegisterWrite( id, 0x0181, value ).execute();
//            }
//        });
//
//        mSpinWhiteSeamLeft.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
//            @Override
//            public void onChange(int value) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                if( mWhiteSeamTest )
//                    new AsyncTaskRegisterWrite( id, 0x0182, value ).execute();
//            }
//        });
//
//        mSpinWhiteSeamRight.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
//            @Override
//            public void onChange(int value) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                if( mWhiteSeamTest )
//                    new AsyncTaskRegisterWrite( id, 0x0183, value ).execute();
//            }
//        });
//
//        mBtnWhiteSeamTopApply.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                int value = mSpinWhiteSeamTop.GetValue();
//
//                Log.i(VD_DTAG, String.format("id( %d ), value( %d )", id, value));
//                new AsyncTaskWhiteSeam(id, AsyncTaskWhiteSeam.TCON_SEAM_TOP, value).execute();
//            }
//        });
//
//        mBtnWhiteSeamBottomApply.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                int value = mSpinWhiteSeamBottom.GetValue();
//
//                Log.i(VD_DTAG, String.format("id( %d ), value( %d )", id, value));
//                new AsyncTaskWhiteSeam(id, AsyncTaskWhiteSeam.TCON_SEAM_BOTTOM, value).execute();
//            }
//        });
//
//        mBtnWhiteSeamLeftApply.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                int value = mSpinWhiteSeamLeft.GetValue();
//
//                Log.i(VD_DTAG, String.format("id( %d ), value( %d )", id, value));
//                new AsyncTaskWhiteSeam(id, AsyncTaskWhiteSeam.TCON_SEAM_LEFT, value).execute();
//            }
//        });
//
//        mBtnWhiteSeamRightApply.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//                if( strId == null || strId.equals("") ) {
//                    return;
//                }
//
//                int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                int value = mSpinWhiteSeamRight.GetValue();
//
//                Log.i(VD_DTAG, String.format("id( %d ), value( %d )", id, value));
//                new AsyncTaskWhiteSeam(id, AsyncTaskWhiteSeam.TCON_SEAM_RIGHT, value).execute();
//            }
//        });
//
//        mBtnWhiteSeamEnable.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
//
//                boolean bValidPort0 = false, bValidPort1 = false;
//                for( byte id : mCabinet ) {
//                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
//                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
//                }
//
//                if( mBtnWhiteSeamEnable.getText().toString().equals("ENABLE") ) {     // disable status
//                    byte[] data0, data1;
//                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_TCON_SEAM_EN , NxCinemaCtrl.FORMAT_INT16);
//                    byte[] val = ctrl.IntToByteArray(1, NxCinemaCtrl.FORMAT_INT16);
//
//                    data0 = ctrl.AppendByteArray(new byte[] {(byte)0x09}, reg);
//                    data0 = ctrl.AppendByteArray(data0, val);
//
//                    data1 = ctrl.AppendByteArray(new byte[] {(byte)0x89}, reg);
//                    data1 = ctrl.AppendByteArray(data1, val);
//
//                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
//                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
//
//                    mBtnWhiteSeamEnable.setText("DISABLE");
//                    mBtnWhiteSeamTest.setEnabled(true);
//
//                    mSpinWhiteSeamTop.setEnabled(true);
//                    mSpinWhiteSeamBottom.setEnabled(true);
//                    mSpinWhiteSeamLeft.setEnabled(true);
//                    mSpinWhiteSeamRight.setEnabled(true);
//
//                    mBtnWhiteSeamTopApply.setEnabled(true);
//                    mBtnWhiteSeamBottomApply.setEnabled(true);
//                    mBtnWhiteSeamLeftApply.setEnabled(true);
//                    mBtnWhiteSeamRightApply.setEnabled(true);
//                }
//                else {
//                    byte[] data0, data1;
//                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_TCON_SEAM_EN , NxCinemaCtrl.FORMAT_INT16);
//                    byte[] val = ctrl.IntToByteArray(0, NxCinemaCtrl.FORMAT_INT16);
//
//                    data0 = ctrl.AppendByteArray(new byte[] {(byte)0x09}, reg);
//                    data0 = ctrl.AppendByteArray(data0, val);
//
//                    data1 = ctrl.AppendByteArray(new byte[] {(byte)0x89}, reg);
//                    data1 = ctrl.AppendByteArray(data1, val);
//
//                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
//                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
//
//                    mBtnWhiteSeamEnable.setText("ENABLE");
//                    mBtnWhiteSeamTest.setEnabled(false);
//
//                    mSpinWhiteSeamTop.setEnabled(false);
//                    mSpinWhiteSeamBottom.setEnabled(false);
//                    mSpinWhiteSeamLeft.setEnabled(false);
//                    mSpinWhiteSeamRight.setEnabled(false);
//
//                    mBtnWhiteSeamTopApply.setEnabled(false);
//                    mBtnWhiteSeamBottomApply.setEnabled(false);
//                    mBtnWhiteSeamLeftApply.setEnabled(false);
//                    mBtnWhiteSeamRightApply.setEnabled(false);
//                }
//            }
//        });
//
//        mBtnWhiteSeamTest.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
//
//                boolean bValidPort0 = false, bValidPort1 = false;
//                for( byte id : mCabinet ) {
//                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
//                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
//                }
//
//                if( mBtnWhiteSeamTest.getText().toString().equals("TEST") ) {     // disable status
//                    byte[] data0, data1;
//                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_TCON_SEAM_TEST, NxCinemaCtrl.FORMAT_INT16);
//                    byte[] val = ctrl.IntToByteArray(1, NxCinemaCtrl.FORMAT_INT16);
//
//                    data0 = ctrl.AppendByteArray(new byte[] {(byte)0x09}, reg);
//                    data0 = ctrl.AppendByteArray(data0, val);
//
//                    data1 = ctrl.AppendByteArray(new byte[] {(byte)0x89}, reg);
//                    data1 = ctrl.AppendByteArray(data1, val);
//
//                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
//                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
//
//                    mBtnWhiteSeamTest.setText("NOT TEST");
//                    mWhiteSeamTest = true;
//
//                    int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//                    new AsyncTaskRegisterWrite( id, 0x0180, mSpinWhiteSeamTop.GetValue() ).execute();
//                    new AsyncTaskRegisterWrite( id, 0x0181, mSpinWhiteSeamBottom.GetValue() ).execute();
//                    new AsyncTaskRegisterWrite( id, 0x0182, mSpinWhiteSeamLeft.GetValue() ).execute();
//                    new AsyncTaskRegisterWrite( id, 0x0183, mSpinWhiteSeamRight.GetValue() ).execute();
//                }
//                else {
//                    byte[] data0, data1;
//                    byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_TCON_SEAM_TEST , NxCinemaCtrl.FORMAT_INT16);
//                    byte[] val = ctrl.IntToByteArray(0, NxCinemaCtrl.FORMAT_INT16);
//
//                    data0 = ctrl.AppendByteArray(new byte[] {(byte)0x09}, reg);
//                    data0 = ctrl.AppendByteArray(data0, val);
//
//                    data1 = ctrl.AppendByteArray(new byte[] {(byte)0x89}, reg);
//                    data1 = ctrl.AppendByteArray(data1, val);
//
//                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
//                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
//
//                    mBtnWhiteSeamTest.setText("TEST");
//                    mWhiteSeamTest = false;
//                }
//            }
//        });

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
        tabSpec3.setIndicator("Dot Correction");
        tabSpec3.setContent(R.id.tabDotCorrection);

        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec("TAB4");
        tabSpec4.setIndicator("Dot Correction Extract");
        tabSpec4.setContent(R.id.tabDotCorrectionExtract);

        TabHost.TabSpec tabSpec5 = tabHost.newTabSpec("TAB5");
        tabSpec5.setIndicator("White Seam Value");
        tabSpec5.setContent(R.id.tabWhiteSeamValue);

        TabHost.TabSpec tabSpec6 = tabHost.newTabSpec("TAB6");
        tabSpec6.setIndicator("Set up");
        tabSpec6.setContent(R.id.tabSetup);

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);
        tabHost.addTab(tabSpec4);
        tabHost.addTab(tabSpec5);
        tabHost.addTab(tabSpec6);

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);

        tabHost.getTabWidget().getChildTabViewAt(3).setEnabled(false);
        ((TextView)tabHost.getTabWidget().getChildAt(3).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        tabHost.getTabWidget().getChildTabViewAt(5).setEnabled(false);
        ((TextView)tabHost.getTabWidget().getChildAt(5).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
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
            if( tabId.equals("TAB6") ) UpdateSetup();
        }
    };

    private boolean IsMasterMode() {
        return (mMasteringMode != null) && mMasteringMode.equals("MASTER");
    }

    private void UpdateMasteringMode() {
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
    }

    private void UpdateImageQuality() {
        //
        //  Update Button
        //
        String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
        String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
        String[] internalQuality = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET, ConfigTconInfo.NAME);
        String[] internalGamma = FileManager.CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);

        if( (usbTcon != null && usbTcon.length != 0) || (usbGamma != null && usbGamma.length != 0) ) {
            mBtnUpdateImageQuality.setEnabled(true);
        }

        if( (internalQuality != null && internalQuality.length != 0) || (internalGamma != null && internalGamma.length != 0) ) {
            mBtnApplyImageQuality.setEnabled(true);
        }
    }

    private void UpdateDotCorrection() {
        String[] result = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( result == null || result.length == 0 )
            return;

        new AsyncTaskAdapterDotCorrection(mAdapterDotCorrect).execute();
        mBtnDotCorrectCheckAll.setEnabled(true);
        mBtnDotCorrectUnCheckAll.setEnabled(true);
        mBtnDotCorrectApply.setEnabled(true);
    }

    private void UpdateWhiteSeamValue() {
//        String strId = mSpinnerWhiteSeamCabinetId.getSelectedItem().toString();
//        if( strId == null || strId.equals("") ) {
//            return;
//        }
//
//        int id = Integer.parseInt(mSpinnerWhiteSeamCabinetId.getSelectedItem().toString(), 10);
//        new AsyncTaskWhiteSeamInit(id).execute();
    }

    private void UpdateDotCorrectionExtract()
    {
        String result = GetExternalStorage();
        if( result != null ) {
            mBtnDotCorrectExtract.setEnabled(true);
        }
        else {
            mBtnDotCorrectExtract.setEnabled(false);
        }
    }

    private void UpdateSetup() {
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

            CinemaLoading.Show( DisplayModeActivity.this );
            ((CinemaInfo)getApplicationContext()).ClearCabinet();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            ((CinemaInfo)getApplicationContext()).SortCabinet();
            CinemaLoading.Hide();

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
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            for( int i = 0; i < mSeekBarMastering.length; i++ ) {
                mSeekBarMastering[i].setProgress( mDataMastering[mMasteringModePos][i] );
            }
            CinemaLoading.Hide();
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
            CinemaLoading.Show(DisplayModeActivity.this);

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
        }
    }

    private class AsyncTaskUniformityCorrection extends AsyncTask<Void, Void, Void> {
        private int mIndexUniformity = mSpinnerUniformity.getSelectedItemPosition();

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            String[] result;
            boolean enableUniformity = false;

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
                    if( !enableUniformity ) {
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
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
        }
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

            //
            //  Check TCON Booting Status
            //
            if( ((CinemaInfo)getApplicationContext()).IsCheckTconBooting() ) {
                boolean bTconBooting = true;
                for( byte id : mCabinet ) {
                    byte[] result;
                    result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                    if (result == null || result.length == 0) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, id));
                        continue;
                    }

                    if( result[0] == 0 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, id, result[0] ));
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
            boolean[] enableGamma = {false, };

            resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : resultPath ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    for( int i = 0; i < info.GetRegister(mIndexQuality).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            //
            //  Parse T_REG.txt
            //
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                ConfigTconInfo info = new ConfigTconInfo();
                if( info.Parse( file ) ) {
                    enableGamma = info.GetEnableUpdateGamma(mIndexQuality);

                    for( int i = 0; i < info.GetRegister(mIndexQuality).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
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
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
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
            CinemaLoading.Show( DisplayModeActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
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
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
            Log.i(VD_DTAG, ">>> Dot Correction Done. ");
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
            int idx     = ((mId % 16) < 8 ) ? (mId) : (mId | 0x80);

            for( int i = start; i < end; i++ ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Dot correction extract. ( id: %d, module: %d )", mId, i) );

                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION_EXTRACT, new byte[]{(byte)idx, (byte)i} );
                if( result == null || result.length == 0)
                    continue;

                String strDir = String.format(Locale.US, "%s/DOT_CORRECTION_ID%03d", GetExternalStorage(), mId);
                if( !FileManager.MakeDirectory( strDir ) ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Create Directory. ( %s )", strDir));
                    continue;
                }

                new LedDotCorrectInfo().Make( idx, i, result, strDir);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
            Log.i(VD_DTAG, ">>> Dot Correction Extract Done. ");
        }
    }

    private class AsyncTaskRegisterWrite extends AsyncTask<Void, Void, Void> {
        private int mId;
        private int mReg;
        private int mVal;

        public AsyncTaskRegisterWrite( int id, int reg, int val ) {
            mId  = id;
            mReg = reg;
            mVal = val;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            byte[] inData = new byte[] { (byte)mId };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray( mReg, NxCinemaCtrl.FORMAT_INT16));
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray( mVal, NxCinemaCtrl.FORMAT_INT16));

            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
            if( result == null || result.length == 0 || ctrl.ByteArrayToInt(result) == 0xFFFF ) {
                Log.i(VD_DTAG, String.format(Locale.US, "i2c write fail.( id: %d, reg: %d, val: %d )", mId, mReg, mVal ));
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
        }
    }

//    private class AsyncTaskWhiteSeamInit extends AsyncTask<Void, Void, Void> {
//        private int mId;
//
//        public AsyncTaskWhiteSeamInit( int id ) {
//            mId = id;
//        }
//
//        @Override
//        protected Void doInBackground(Void... voids) {
//            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
//
//            byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_TCON_SEAM_EN, NxCinemaCtrl.FORMAT_INT16);
//            byte[] inData = ctrl.AppendByteArray(new byte[]{(byte)mId}, reg);
//            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
//
//            if( result == null || result.length == 0 )
//                return null;
//
//
//
//            return null;
//        }
//
//        @Override
//        protected void onPreExecute() {
//            super.onPreExecute();
//            CinemaLoading.Show( DisplayModeActivity.this );
//        }
//
//        @Override
//        protected void onPostExecute(Void aVoid) {
//            super.onPostExecute(aVoid);
//            CinemaLoading.Hide();
//            Log.i(VD_DTAG, ">>> WhiteSeam Init Done.");
//        }
//    }
//
//    private class AsyncTaskWhiteSeam extends AsyncTask<Void, Void, Void> {
//        public static final int TCON_SEAM_TOP   = 0;
//        public static final int TCON_SEAM_BOTTOM= 1;
//        public static final int TCON_SEAM_LEFT  = 2;
//        public static final int TCON_SEAM_RIGHT = 3;
//
//        private int mId;
//        private int mPos;
//        private int mValue;
//
//        public AsyncTaskWhiteSeam( int id, int pos, int value ) {
//            mId     = id;
//            mPos    = pos;
//            mValue  = value;
//        }
//
//        @Override
//        protected Void doInBackground(Void... voids) {
//            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
//
//            byte[] index = new byte[] { (byte)mId, (byte)mPos };
//            byte[] value = ctrl.IntToByteArray( mValue, NxCinemaCtrl.FORMAT_INT16);
//
//            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_WHITE_SEAM_WRITE, ctrl.AppendByteArray(index, value) );
//            if( result == null || result.length == 0 ) {
//                Log.i(VD_DTAG, String.format(Locale.US, "Seam data write failed. ( id: %d, pos: %d )", mId, mPos));
//            }
//
//            return null;
//        }
//
//        @Override
//        protected void onPreExecute() {
//            super.onPreExecute();
//            CinemaLoading.Show( DisplayModeActivity.this );
//        }
//
//        @Override
//        protected void onPostExecute(Void aVoid) {
//            super.onPostExecute(aVoid);
//            CinemaLoading.Hide();
//            Log.i(VD_DTAG, ">>> White Seam Done.");
//        }
//    }

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
