package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
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
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.TextButtonAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.CheckRunInfo;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigPfpgaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedDotCorrectInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedGammaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedUniformityInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.CheckRunAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.TextButtonInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaService;
import com.samsung.vd.cinemacontrolpanel.Utils.FileManager;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.Comparator;
import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeAdapterDotCorrection;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyLedOpenDetection;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyLodRemoval;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyMastering;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyModule;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyScale;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplySeam;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplySyncDelay;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplySyncReverse;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplySyncWidth;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyXyzInput;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeApplyZeroScale;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeCheckCabinet;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeDotCorrection;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeDotCorrectionExtract;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeGlobalRead;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeImageQuality;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeInitWhiteSeam;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeLodReset;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeMasteringRead;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeMasteringWrite;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeUniformityBtn;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeUniformityCorrection;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeUpdateUnif;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeWhiteSeamEmulate;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeWhiteSeamRead;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeWhiteSeamWrite;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayModeonCheckchanged;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends BaseActivity {
    private final String VD_DTAG = "DisplayModeActivity";
    private UIHandler mUIHandler;
    TabHost tabHost;
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

    //tab1  tabMasteringMode
    private Spinner mSpinnerMastering;
    private Button btnMasteringMode;
    private Button[] mBtnMastering = new Button[mTextMastering.length];
    private SeekBar[] mSeekBarMastering = new SeekBar[mTextMastering.length];
    private EditText[] mValueMastering = new EditText[mTextMastering.length];

    private String mMasteringMode = mStrModeMastering[0];
    private int mMasteringModePos = 0;

    //tab2  tabUniformityCorrection
    private Spinner mSpinnerUniformity;
    private Button mBtnEnableUniformity;
    private Button mBtnUpdateUniformity;
    private Button mBtnApplyUniformity;

    ConfigTconInfo sourceTconInfo;

    //tab3  tabImageQuality
    private TextView mTextImageQuality;
    private Button mBtnUpdateImageQuality;
    private TextButtonAdapter mAdapterMode;

    //tab4  tabDotCorrection
    private CheckRunAdapter mAdapterDotCorrect;
    private Button mBtnDotCorrectCheckAll;
    private Button mBtnDotCorrectUnCheckAll;
    private Button mBtnDotCorrectApply;

    //tab5  tabDotCorrectionExtract
    private Spinner mSpinnerDotCorrectExtractId;
    private Spinner mSpinnerDotCorrectExtractModule;
    private Button mBtnDotCorrectExtract;

    //tab6  tabWhiteSeamValue
    private Spinner mSpinnerWhiteSeamCabinetId;
    private VdSpinCtrl[] mSpinWhiteSeam = new VdSpinCtrl[4];
    private CheckBox mCheckWhiteSeamEmulation;
    private Button mBtnWhiteSeamApply;

    //tab7
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

    //tab8  tabSetup
    private Spinner mSpinnerSuspendTime;
    private EditText mEditCabinet;


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

//                String[] resultTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
//                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
//                if( (resultTcon != null && resultTcon.length != 0) || (resultGamma != null && resultGamma.length != 0) ) {
//                    mBtnUpdateImageQuality.setEnabled(true);
//                }

                if(2 == tabHost.getCurrentTab()) {
                    UpdateImageQuality();
                }

                String[] resultDot = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
                if( (resultDot != null && resultDot.length != 0) ) {
                    NXAsync.getInstance().getDisplayModeAsyncParam().setmAdapter(mAdapterDotCorrect);
//                    new AsyncTaskAdapterDotCorrection(mAdapterDotCorrect).execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeAdapterDotCorrection , asyncCallbackAdapterDotCorrection);

                    mBtnDotCorrectCheckAll.setEnabled(true);
                    mBtnDotCorrectUnCheckAll.setEnabled(true);
                    mBtnDotCorrectApply.setEnabled(true);
                }

                if( resultPfpga != null ) for(String file : resultPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                if( resultUniformity != null ) for(String file : resultUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
//                if( resultTcon != null ) for(String file : resultTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
//                if( resultGamma != null ) for(String file : resultGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));

                String strIndex = mSpinnerDotCorrectExtractId.getSelectedItem().toString();
                if( strIndex != null && !strIndex.equals("") ) {
                    mBtnDotCorrectExtract.setEnabled(true);
                }
            }

            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnUpdateUniformity.setEnabled(false);

//                mBtnUpdateImageQuality.setEnabled(false);
                if(2 == tabHost.getCurrentTab()) {
                    UpdateImageQuality();
                }

                mBtnDotCorrectCheckAll.setEnabled(false);
                mBtnDotCorrectUnCheckAll.setEnabled(false);
                mBtnDotCorrectApply.setEnabled(false);

                mBtnDotCorrectExtract.setEnabled(false);

                mAdapterDotCorrect.clear();
            }
        }
    };

    @Override
    protected void onStart() {
        super.onStart();
        //Async
        UpdateMasteringMode();

    }

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

    String[] strCabinetNumber;
    String[] strCabinetNumberWithAll;
    String[] strModuleWithAll;
    View listViewFooter;

    public void VarSetting() {
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();

        //
        //  Cabinet Number & Module Number String
        //
        //mCabinet = new byte[]{0x66, 0x67};

        if( mCabinet.length == 0 ) {
            strCabinetNumber = new String[]{ "" };
        }
        else {
            strCabinetNumber = new String[mCabinet.length];
            for( int i = 0; i < mCabinet.length; i++ ) {
                strCabinetNumber[i] = String.valueOf((mCabinet[i] & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
            }
        }

        strCabinetNumberWithAll = new String[mCabinet.length+1];
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

        strModuleWithAll =  new String[25];
        strModuleWithAll[0] = "ALL";
        for( int i = 0; i < 24; i++ ) {
            strModuleWithAll[i+1] = String.format(Locale.US, "MODULE #%d", i);
        }

        mUIHandler = new UIHandler( this );
    }

    public void CommonUISetting() {
        mServiceCallback = new CinemaService.ChangeContentsCallback() {
            @Override
            public void onChangeContentsCallback(int mode) {
                mUIHandler.sendEmptyMessage(0);
            }
        };

        //
        // Set Title Bar and Status Bar
        //  AsyncTask need in activity change
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Display Mode" , new Intent(getApplicationContext(), TopActivity.class) );
        AddTabs();

        //
        //  Cinema System Information
        //
        listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
    }

    public void UIFirstTabSetting() {
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

        btnMasteringMode = (Button)findViewById(R.id.btnMasteringMode);
        btnMasteringMode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mMasteringMode = mSpinnerMastering.getSelectedItem().toString();
                mMasteringModePos = mSpinnerMastering.getSelectedItemPosition();

                ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_MASTERING_MODE, mMasteringMode);
                if( !IsMasterMode() ) {
                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteMode(IsMasterMode());
                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteModePos(mMasteringModePos);
//                    new AsyncTaskMasteringWrite().execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeMasteringWrite , asyncCallbackMasteringWrite);
                }
                else {
                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteMode(IsMasterMode());
                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteModePos(mMasteringModePos);
//                    new AsyncTaskMasteringWrite().execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeMasteringWrite , asyncCallbackMasteringWrite);

                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteMode(IsMasterMode());
//                    new AsyncTaskMasteringRead().execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeMasteringRead , asyncCallbackMasteringRead);
                }

                UpdateMasteringMode();

                ((CinemaInfo)getApplicationContext()).InsertLog(String.format( Locale.US, "Change Mastering Mode. ( %s Mode )", mMasteringMode ));
            }
        });


        //UISettingMastering();
        mLayoutMastering = new LinearLayout[mTextMastering.length];
        for( int i = 0; i < mTextMastering.length; i++ ) {
            mLayoutMastering[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_mastering, mParentLayoutMastering, false );
            AddViewMastering( i, mLayoutMastering, mTextMastering );
        }
    }

    public void UISeconeTabSetting() {
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
                NXAsync.getInstance().Execute(CMD_DisplayModeUniformityBtn , asyncCallbackUniformityBtn);
            }
        });

        mBtnApplyUniformity = (Button)findViewById(R.id.btnApplyUniformity);
        mBtnApplyUniformity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
//                new AsyncTaskUniformityCorrection().execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeUniformityCorrection , asyncCallbackUniformityCorrection);
            }
        });
    }

    public void UIThirdTabSetting() {
        //
        //  IMAGE QUALITY
        //
        mTextImageQuality = (TextView)findViewById(R.id.textModeCurrent);

        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        int mode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
        mTextImageQuality.setText(String.format(Locale.US, "Index #%d", mode+1));

        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );

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
                    ShowMessage( "Update Image Quality File.\nIndex 11~20 will be changed");
                    UpdateImageQuality();
                }
            }
        });
    }

    public void UIFouthTabSetting() {
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

                NXAsync.getInstance().getDisplayModeAsyncParam().setmAdapterDot(mAdapterDotCorrect);
//                new AsyncTaskDotCorrection(mAdapterDotCorrect).execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeDotCorrection , asyncCallbackDotCorrection);
            }
        });
    }

    public void UIFifthTabSetting() {
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

                NXAsync.getInstance().getDisplayModeAsyncParam().setmId(index);
                NXAsync.getInstance().getDisplayModeAsyncParam().setmModule(module);
                NXAsync.getInstance().getDisplayModeAsyncParam().setExternalStorage(GetExternalStorage());
//                new AsyncTaskDotCorrectionExtract(index, module).execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeDotCorrectionExtract , baseAsyncCallback);
            }
        });
    }

    public void UISixthTabSetting() {
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
                NXAsync.getInstance().getDisplayModeAsyncParam().setIdxPosbEmulate(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition() , mCheckWhiteSeamEmulation.isChecked());
//        new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamRead , asyncCallbackWhiteSeamRead);
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
                    NXAsync.getInstance().getDisplayModeAsyncParam().setIndexPos(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition());
//                    new AsyncTaskWhiteSeamEmulate(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamEmulate , asyncCallbackWhiteSeamEmulate);
                }
            });
            spin.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                    NXAsync.getInstance().getDisplayModeAsyncParam().setIndexPos(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition());
//                    new AsyncTaskWhiteSeamEmulate(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamEmulate , asyncCallbackWhiteSeamEmulate);
                    return false;
                }
            });
        }

        mBtnWhiteSeamApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NXAsync.getInstance().getDisplayModeAsyncParam().setPos(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition());
//                new AsyncTaskWhiteSeamWrite(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition()).execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamWrite , asyncCallbackWhiteSeamWrite);
            }
        });

    }

    public void UISeventhTabSetting() {
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
    }

    public void UIEightthTabSetting() {
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
//                new AsyncTaskCheckCabinet().execute();
                NXAsync.getInstance().Execute(CMD_DisplayModeCheckCabinet , asyncCallbackCheckCabinet);
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
    }

    public void UISetting() {
        CommonUISetting();

        UIFirstTabSetting();
        UISeconeTabSetting();
        UIThirdTabSetting();
        UIFouthTabSetting();
        UIFifthTabSetting();
        UISixthTabSetting();
        UISeventhTabSetting();
        UIEightthTabSetting();

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


    View.OnClickListener onRefreshClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            bRefreshFlag = true;
            int tabIdx = tabHost.getCurrentTab();
            if( 0 == tabIdx ) {
                UpdateMasteringMode();
            }
            if( 1 == tabIdx ) {
                UpdateUniformityCorrection();
            }
            if( 2 == tabIdx ) {
                UpdateImageQuality();
            }
            if( 3 == tabIdx ) {
                UpdateDotCorrection();
            }
            if( 4 == tabIdx ) {
                UpdateDotCorrectionExtract();
            }
            if( 5 == tabIdx ) {
                mCheckWhiteSeamEmulation.setChecked(false);
                UpdateWhiteSeamValue();
            }
            if( 6 == tabIdx ) {
                UpdateSetup();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_mode);

        bI2CFailFlag = false;
        bRefreshFlag = false;
        contentParent = (RelativeLayout)findViewById(R.id.contentParent);
        setOnRefreshClickListener(onRefreshClickListener);

        VarSetting();
        UISetting();
    } //onCreate

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

            NXAsync.getInstance().getTconInfoCarrier().setiMode(position);
//            new AsyncTaskImageQuality(position).execute();
            NXAsync.getInstance().Execute(CMD_DisplayModeImageQuality , asyncCallbackImageQuality);
        }
    };

    private TextButtonAdapter.OnClickListener mRemovebuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            int iMode = Integer.parseInt(mAdapterMode.getItem(position).GetMode());
            int eepromModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum();

            if(10 > position && position < eepromModeNum) {
                NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Remove(iMode , DisplayModeActivity.this);
            }

            int usbModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum();
            if(10 <= position && (position-10) < usbModeNum ) {
                NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Remove(iMode , DisplayModeActivity.this);
            }
        }
    };
    private TextButtonAdapter.OnClickListener mUpdatebuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            int iMode = Integer.parseInt(mAdapterMode.getItem(position).GetMode());
            int eepromModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum();

            if(10 > position && position < eepromModeNum) {
                NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Update(iMode , DisplayModeActivity.this , sourceTconInfo);
            }

            int usbModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum();
            if(10 <= position && (position-10) < usbModeNum ) {
                NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Update(iMode , DisplayModeActivity.this , sourceTconInfo);
            }
        }
    };

    private void AddTabs() {
        tabHost = (TabHost) findViewById(R.id.tabHost);
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
            if( tabId.equals("TAB0") ) {
                UpdateMasteringMode();
            }
            if( tabId.equals("TAB1") ) {
                UpdateUniformityCorrection();
            }
            if( tabId.equals("TAB2") ) {
                UpdateImageQuality();
            }
            if( tabId.equals("TAB3") ) {
                UpdateDotCorrection();
            }
            if( tabId.equals("TAB4") ) {
                UpdateDotCorrectionExtract();
            }
            if( tabId.equals("TAB5") ) {
                mCheckWhiteSeamEmulation.setChecked(false);
                UpdateWhiteSeamValue();
            }
            if( tabId.equals("TAB6") ) {
                UpdateGlobal();
            }
            if( tabId.equals("TAB7") ) {
                UpdateSetup();
            }
        }
    };

    private void UpdateGlobal() {
        InitWhiteSeamValue();

        NXAsync.getInstance().Execute(CMD_DisplayModeGlobalRead , asyncCallbackGlobalRead);
//        new AsyncTaskGlobalRead().execute();
    }

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
                    NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringWriteMode(IsMasterMode());
//                    new AsyncTaskMasteringRead().execute();
                    NXAsync.getInstance().Execute(CMD_DisplayModeMasteringRead , asyncCallbackMasteringRead);
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

        NXAsync.getInstance().Execute(CMD_DisplayModeUpdateUnif , asyncCallbackUpdateUnif);
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
        if( usbTcon != null ) for(String file : usbTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        if( usbGamma != null ) for(String file : usbGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));

//        if( (internalQuality != null && internalQuality.length != 0) || (internalGamma != null && internalGamma.length != 0) ) {
//            mBtnApplyImageQuality.setEnabled(true);
//        }

        //
        //
        //
        sourceTconInfo = new ConfigTconInfo();
        int[] mMode = null;
        if(usbTcon != null && usbTcon.length != 0) {
            for( String file : usbTcon ) {
                if(sourceTconInfo.Parse(file)) {
                    mMode = sourceTconInfo.GetModeArray();
                }
            }
        }

        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        mAdapterMode.clear();
        for( String file : resultPath ) {
            if( NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum() ) {

                        if( null != mMode ){
                            boolean bFound = false;
                            for (int k = 0 ; k < mMode.length ; k++) {
                                if (NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i) == mMode[k]) {
                                    bFound = true;
                                    mAdapterMode.add(
                                            new TextButtonInfo(
                                                    String.format(Locale.US, "%d", i+1),
                                                    String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                                    String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                                    mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, mUpdatebuttonAdapterClickListener)
                                    );
                                    break;
                                }
                            }
                            if (!bFound) {
                                mAdapterMode.add(
                                        new TextButtonInfo(
                                                String.format(Locale.US, "%d", i+1),
                                                String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                                String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                                mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                                );
                            }
                        }else {
                            mAdapterMode.add(
                                    new TextButtonInfo(
                                            String.format(Locale.US, "%d", i+1),
                                            String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                            String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                            mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                            );
                        }

                    }
                    else {
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, ""), "")
                        );
                    }
                    mAdapterMode.notifyDataSetChanged();
                }

            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, ""), "")
                    );
                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum() ) {
                        if( null != mMode ){
                            boolean bFound = false;
                            for (int k = 0 ; k < mMode.length ; k++) {
                                if (NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i) == mMode[k]) {
                                    bFound = true;
                                    mAdapterMode.add(
                                            new TextButtonInfo(
                                                    String.format(Locale.US, "%d", i+11),
                                                    String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                                    String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                                    mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, mUpdatebuttonAdapterClickListener)
                                    );
                                    break;
                                }
                            }
                            if(!bFound) {
                                mAdapterMode.add(
                                        new TextButtonInfo(
                                                String.format(Locale.US, "%d", i+11),
                                                String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                                String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                                mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                                );
                            }
                        }else {
                            mAdapterMode.add(
                                    new TextButtonInfo(
                                            String.format(Locale.US, "%d", i+11),
                                            String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                            String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                            mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                            );
                        }
                    }
                    else {
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, ""), "")
                        );
                    }
                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, ""), "")
                    );
                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }
    } //UpdateImageQuality

    private void UpdateDotCorrection() {
        InitWhiteSeamValue();

        String[] result = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( result == null || result.length == 0 )
            return;

        NXAsync.getInstance().getDisplayModeAsyncParam().setmAdapter(mAdapterDotCorrect);
//                    new AsyncTaskAdapterDotCorrection(mAdapterDotCorrect).execute();
        NXAsync.getInstance().Execute(CMD_DisplayModeAdapterDotCorrection, asyncCallbackAdapterDotCorrection);

        mBtnDotCorrectCheckAll.setEnabled(true);
        mBtnDotCorrectUnCheckAll.setEnabled(true);
        mBtnDotCorrectApply.setEnabled(true);
    }

    private void InitWhiteSeamValue() {
        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(null);
        NXAsync.getInstance().Execute(CMD_DisplayModeInitWhiteSeam , asyncCallbackInitWhiteSeam);
    }

    private void UpdateWhiteSeamValue() {
        InitWhiteSeamValue();

        mBtnWhiteSeamApply.setEnabled(mCheckWhiteSeamEmulation.isChecked());
        for( VdSpinCtrl spin : mSpinWhiteSeam )
            spin.setEnabled(mCheckWhiteSeamEmulation.isChecked());

        NXAsync.getInstance().getDisplayModeAsyncParam().setIdxPosbEmulate(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition() , mCheckWhiteSeamEmulation.isChecked());
//        new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
        NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamRead , asyncCallbackWhiteSeamRead);
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

    private void UpdateSetup() {
        InitWhiteSeamValue(); //need?

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

        NXAsync.getInstance().getDisplayModeAsyncParam().setItemIdx(itemIdx);
        NXAsync.getInstance().getDisplayModeAsyncParam().setValue(value);
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyMastering , asyncCallbackUniformityCorrection);
    }


    void ApplySyncWidth() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setmSpinSyncWidthValue(mSpinSyncWidth.GetValue());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplySyncWidth , baseAsyncCallback);
    }

    void ApplySyncDelay() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setmSpinSyncDelayValue(mSpinSyncDelay.GetValue());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplySyncDelay , baseAsyncCallback);
    }

    void ApplySyncReverse() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckSyncReverse.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplySyncReverse , baseAsyncCallback);
    }

    void ApplyScale() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckScale.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyScale , baseAsyncCallback);
    }

    void ApplyZeroScale() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckZeroScale.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyZeroScale , baseAsyncCallback);
    }

    void ApplySeam() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckSeam.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplySeam , baseAsyncCallback);
    }

    void ApplyModule() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckModule.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyModule , baseAsyncCallback);
    }

    void ApplyXyzInput() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckXyzInput.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyXyzInput , baseAsyncCallback);
    }

    void ApplyLedOpenDetection() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckLedOpenDetection.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyLedOpenDetection , baseAsyncCallback);
    }

    void ApplyLodRemoval() {
        NXAsync.getInstance().getDisplayModeAsyncParam().setUIChecked(mCheckLodRemoval.isChecked());
        NXAsync.getInstance().Execute(CMD_DisplayModeApplyLodRemoval , baseAsyncCallback);
    }

    void ApplyLodReset() {

//        new AsyncTaskLodReset().execute();
        NXAsync.getInstance().Execute(CMD_DisplayModeLodReset , baseAsyncCallback);
    }

    //
    //
    //
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
        mTextImageQuality.setText(String.format(Locale.US, "Index #%d", mode+1));
    }



    NXAsync.AsyncCallback asyncCallbackInitWhiteSeam = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );

                    mCheckWhiteSeamEmulation.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                            Log.i(VD_DTAG, String.format(Locale.US, "Change WhiteSeam Emulation. ( %b )", mCheckWhiteSeamEmulation.isChecked()));

                            mBtnWhiteSeamApply.setEnabled(b);
                            for( VdSpinCtrl spin : mSpinWhiteSeam ) {
                                spin.setEnabled(b);
                            }

                            NXAsync.getInstance().getDisplayModeAsyncParam().setmSpinnerWhiteSeamCabinetIdSelected(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition());
                            NXAsync.getInstance().getDisplayModeAsyncParam().setChecked(mCheckWhiteSeamEmulation.isChecked());
                            NXAsync.getInstance().Execute(CMD_DisplayModeonCheckchanged , asyncCallbackCheckChanged);

    //                        new AsyncTaskWhiteSeamRead(mCabinet, mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(), mCheckWhiteSeamEmulation.isChecked()).execute();
                            NXAsync.getInstance().Execute(CMD_DisplayModeWhiteSeamRead , asyncCallbackWhiteSeamRead);
                        }
                    });

                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackInitWhiteSeam

    NXAsync.AsyncCallback asyncCallbackCheckCabinet = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    ((CinemaInfo)getApplicationContext()).ClearCabinet();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

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

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackCheckCabinet

    NXAsync.AsyncCallback asyncCallbackMasteringWrite = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    for( int i = 0; i < mSeekBarMastering.length; i++ ) {
                        mSeekBarMastering[i].setProgress( mDataMastering[mMasteringModePos][i] );
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackMasteringWrite

    NXAsync.AsyncCallback asyncCallbackMasteringRead = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );

                    NXAsync.getInstance().getDisplayModeAsyncParam().allocMasteringReadValue(mRegMastering.length);

                    for( int i = 0; i < NXAsync.getInstance().getDisplayModeAsyncParam().getMasteringReadValue().length; i++ ) {
                        NXAsync.getInstance().getDisplayModeAsyncParam().setMasteringReadValue(i,0);
                    }
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    for (int i = 0; i < mSeekBarMastering.length; i++) {
                        if( 0 > NXAsync.getInstance().getDisplayModeAsyncParam().getMasteringReadValueinPos(i) )
                            continue;

                        mSeekBarMastering[i].setProgress(NXAsync.getInstance().getDisplayModeAsyncParam().getMasteringReadValueinPos(i));
                        mDataMastering[mMasteringModePos][i] = NXAsync.getInstance().getDisplayModeAsyncParam().getMasteringReadValueinPos(i);

                        mBtnMastering[i].setEnabled(false);
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackMasteringRead

    NXAsync.AsyncCallback asyncCallbackUniformityCorrection = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDisplayModeAsyncParam().setmIndexUniformity(mSpinnerUniformity.getSelectedItemPosition());
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackUniformityCorrection

    NXAsync.AsyncCallback asyncCallbackImageQuality = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    int mModeIndexQuality = NXAsync.getInstance().getTconInfoCarrier().getiMode();
                    mTextImageQuality.setText(String.format(Locale.US, "Index #%d", mModeIndexQuality+1));
                    ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mModeIndexQuality));

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackImageQuality

    NXAsync.AsyncCallback asyncCallbackAdapterDotCorrection = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDisplayModeAsyncParam().getmAdapter().clear();
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CheckRunAdapter mAdapter = NXAsync.getInstance().getDisplayModeAsyncParam().getmAdapter();
                    String dir = NXAsync.getInstance().getDisplayModeAsyncParam().getDir();
                    String[] resultFile = FileManager.CheckFile(dir, LedDotCorrectInfo.PATTERN_NAME);
                    mAdapter.add( new CheckRunInfo(dir.substring(dir.lastIndexOf("/") + 1), String.format("total : %s", resultFile.length)) );
                    Collections.sort(mAdapter.get(), new Comparator<CheckRunInfo>() {
                        @Override
                        public int compare(CheckRunInfo t0, CheckRunInfo t1) {
                            return (t0.GetTitle().compareTo(t1.GetTitle()) > 0) ? 1 : -1;
                        }
                    });

                    mAdapter.notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackAdapterDotCorrection

    NXAsync.AsyncCallback asyncCallbackDotCorrection = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    int idx = NXAsync.getInstance().getDisplayModeAsyncParam().getIdx();
                    int len = NXAsync.getInstance().getDisplayModeAsyncParam().getLen();
                    int success = NXAsync.getInstance().getDisplayModeAsyncParam().getSuccess();
                    int fail = NXAsync.getInstance().getDisplayModeAsyncParam().getFail();
                    CheckRunAdapter mAdapter = NXAsync.getInstance().getDisplayModeAsyncParam().getmAdapterDot();
                    CheckRunInfo info = mAdapter.getItem(idx);
                    info.SetDescription( String.format(Locale.US, "total: %d, success: %d, fail: %d", len, success, fail));
                    mAdapter.notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackDotCorrection

    NXAsync.AsyncCallback asyncCallbackWhiteSeamRead = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    int[] mSeamVal = NXAsync.getInstance().getDisplayModeAsyncParam().getmSeamVal();
                    for( int i = 0; i < mSeamVal.length; i++ ) {
                        mSpinWhiteSeam[i].SetValue(mSeamVal[i]);
                    }
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackWhiteSeamRead

    NXAsync.AsyncCallback asyncCallbackWhiteSeamEmulate = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    int[] mSeamVal = new int[4];
                    for( int i = 0; i < mSeamVal.length; i++ )
                        mSeamVal[i] = mSpinWhiteSeam[i].GetValue();

                    NXAsync.getInstance().getDisplayModeAsyncParam().setSeamVal(mSeamVal);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackWhiteSeamEmulate

    NXAsync.AsyncCallback asyncCallbackWhiteSeamWrite = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    int[] mSeamVal = new int[4];
                    for( int i = 0; i < mSeamVal.length; i++ )
                        mSeamVal[i] = mSpinWhiteSeam[i].GetValue();

                    NXAsync.getInstance().getDisplayModeAsyncParam().setSeamvalue(mSeamVal);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackWhiteSeamWrite

    NXAsync.AsyncCallback asyncCallbackUniformityBtn = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    boolean bEnable = mBtnEnableUniformity.getText().toString().equals("ENABLE");
                    NXAsync.getInstance().getDisplayModeAsyncParam().setbEnable(bEnable);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    if( mBtnEnableUniformity.getText().toString().equals("ENABLE") ) {
                        mBtnEnableUniformity.setText("DISABLE");
                    }
                    else{
                        mBtnEnableUniformity.setText("ENABLE");
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackUniformityBtn

    NXAsync.AsyncCallback asyncCallbackUpdateUnif = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    byte[] resultEnable = NXAsync.getInstance().getDisplayModeAsyncParam().getResultEnable();
                    if( (resultEnable != null) && resultEnable[3] == (byte)1 ) {
                        mBtnEnableUniformity.setText("ENABLE");
                    }
                    else {
                        mBtnEnableUniformity.setText("DISABLE");
                    }

                    mSpinnerUniformity.setAdapter( new ArrayAdapter<>(DisplayModeActivity.this, android.R.layout.simple_spinner_dropdown_item, new String[]{ " " }));
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
                                mSpinnerUniformity.setAdapter( new ArrayAdapter<>(DisplayModeActivity.this, android.R.layout.simple_spinner_dropdown_item, strTemp));
                                mSpinnerUniformity.setEnabled(true);
                            }
                        }
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackUpdateUnif

    NXAsync.AsyncCallback asyncCallbackCheckChanged = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }
                    NXAsync.getInstance().getDisplayModeAsyncParam().setIdxPosbEmulate(mSpinnerWhiteSeamCabinetId.getSelectedItemPosition() , mCheckWhiteSeamEmulation.isChecked());
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackCheckChanged

    NXAsync.AsyncCallback asyncCallbackGlobalRead = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DisplayModeActivity.this );
                    UnregisterListener();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    int[] values = NXAsync.getInstance().getDisplayModeAsyncParam().getGlobalVal();

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

                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayModeActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    RegisterListener();
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackGlobalRead


} //DisplayModeActivity
