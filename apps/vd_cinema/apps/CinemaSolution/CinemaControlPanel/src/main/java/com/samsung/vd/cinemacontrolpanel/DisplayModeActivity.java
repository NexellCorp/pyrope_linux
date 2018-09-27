package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
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
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdSpinCtrl;
import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private CinemaInfo mCinemaInfo;
    private byte[]  mCabinet;

    private TabHost mTabHost;

    private Spinner mSpinnerUniformity;
    private Button mBtnEnableUniformity;
    private Button mBtnUpdateUniformity;
    private Button mBtnApplyUniformity;

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
                if( (usbPfpga != null && usbPfpga.length != 0) ||
                    (usbUniformity != null && usbUniformity.length != 0 ) ) {
                    mBtnUpdateUniformity.setEnabled(true);

                    if( usbPfpga != null ) for(String file : usbPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                    if( usbUniformity != null ) for(String file : usbUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
                }

                String[] usbDot = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
                if( (usbDot != null && usbDot.length != 0) ) {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_PIXEL_CORRECTION_ADAPTER,
                            getApplicationContext(),
                            mAdapterDotCorrect,
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    HideProgress();
                                }
                            },
                            null
                    );

                    mBtnDotCorrectCheckAll.setEnabled(true);
                    mBtnDotCorrectUnCheckAll.setEnabled(true);
                    mBtnDotCorrectApply.setEnabled(true);
                }

                String strIndex = mSpinnerDotCorrectExtractId.getSelectedItem().toString();
                if( strIndex != null && !strIndex.equals("") ) {
                    mBtnDotCorrectExtract.setEnabled(true);
                }
            }

            if( action.equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnUpdateUniformity.setEnabled(false);

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
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();
        mCabinet = mCinemaInfo.GetCabinet();

        final ViewGroup rootGroup = null;
        View listViewFooter = null;
        LayoutInflater inflater = (LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        if( inflater != null ) listViewFooter = inflater.inflate(R.layout.listview_footer_blank, rootGroup, false);

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
                if( mTabPrevious.equals("TAB5") ) ClearWhiteSeamValue();
                Launch(v.getContext(), TopActivity.class);
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mTabPrevious.equals("TAB5") ) ClearWhiteSeamValue();
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
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //  Cabinet Number String
        String[] strCabinetNumber;
        if( mCabinet.length == 0 ) {
            strCabinetNumber = new String[]{ "" };
        }
        else {
            strCabinetNumber = new String[mCabinet.length];
            for( int i = 0; i < mCabinet.length; i++ ) {
                strCabinetNumber[i] = String.valueOf(mCinemaInfo.GetCabinetNumber(mCabinet[i]));
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
                strCabinetNumberWithAll[i+1] = String.valueOf(mCinemaInfo.GetCabinetNumber(mCabinet[i]));
            }
        }

        //  Module Number String
        String [] strModuleWithAll =  new String[25];
        strModuleWithAll[0] = "ALL";
        for( int i = 0; i < 24; i++ ) {
            strModuleWithAll[i+1] = String.format(Locale.US, "MODULE #%d", i);
        }

        //
        //  UNIFORMITY CORRECTION
        //
        mSpinnerUniformity = (Spinner)findViewById(R.id.spinnerUniformity);
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
                final boolean bEnable = mBtnEnableUniformity.getText().toString().equals("DISABLE");
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_PFPGA_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_PFPGA_NUC_EN},
                        new int[]{bEnable ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mBtnEnableUniformity.setText( bEnable ? "ENABLE" : "DISABLE" );
                                HideProgress();
                            }
                        },
                        null
                );
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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                HideProgress();
                            }
                        },
                        null
                );
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
                    if( info != null ) info.SetChecked( true );
                }
                mAdapterDotCorrect.notifyDataSetChanged();
            }
        });

        mBtnDotCorrectUnCheckAll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                for( int i = 0; i < mAdapterDotCorrect.getCount(); i++ ) {
                    CheckRunInfo info = mAdapterDotCorrect.getItem(i);
                    if( info != null ) info.SetChecked( false );
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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                HideProgress();
                            }
                        },
                        null
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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                HideProgress();
                            }
                        },
                        null
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
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    HideProgress();
                                }
                            },
                            null
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
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    HideProgress();
                                }
                            },
                            null
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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        //
        //  GLOBAL REGISTER
        //
        mSpinSyncWidth = (VdSpinCtrl)findViewById(R.id.spinSyncWidth);
        mBtnApplySyncWidth = (Button)findViewById(R.id.btnApplySyncWidth);
        mSpinSyncDelay = (VdSpinCtrl)findViewById(R.id.spinSyncDelay);
        mBtnApplySyncDelay = (Button)findViewById(R.id.btnApplySyncDelay);
        mCheckSyncReverse = (CheckBox)findViewById(R.id.checkSyncReverse);
        mCheckZeroScale = (CheckBox)findViewById(R.id.checkZeroScale);
        mCheckSeam = (CheckBox)findViewById(R.id.checkSeam);
        mCheckModule = (CheckBox)findViewById(R.id.checkModule);
        mCheckXyzInput = (CheckBox)findViewById(R.id.checkXyzInput);
        mCheckLedOpenDetection = (CheckBox)findViewById(R.id.checkLedOpenDetect);
        mCheckLodRemoval = (CheckBox)findViewById(R.id.checkLodRemoval);
        mBtnLodReset = (Button)findViewById(R.id.btnLodReset);

        mSpinSyncWidth.SetRange(0, 4095);
        mSpinSyncDelay.SetRange(0, 4095);

        //
        //  SETUP
        //
        String desireCabinetNum = mCinemaInfo.GetValue(CinemaInfo.KEY_CABINET_NUM);

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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                int detectCabinetNum = (Integer)values[0];
                                int desireCabinetNum = Integer.parseInt( mEditCabinet.getText().toString() );
                                int curCabinetNum = Integer.parseInt( mCinemaInfo.GetValue(CinemaInfo.KEY_CABINET_NUM ) );

                                if( detectCabinetNum != desireCabinetNum && mCinemaInfo.IsCheckCabinetNum() ) {
                                    ShowMessage( String.format(Locale.US, "Please check cabinet number. ( desire: %d, detect: %d )", desireCabinetNum, detectCabinetNum) );
                                }
                                else {
                                    mCinemaInfo.SetValue(CinemaInfo.KEY_CABINET_NUM, mEditCabinet.getText().toString());
                                    mCinemaInfo.InsertLog(
                                        String.format( "Change cabinet number. ( %s -> %s )", curCabinetNum, desireCabinetNum )
                                    );
                                }
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

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
                mCinemaInfo.SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[mSpinnerSuspendTime.getSelectedItemPosition()]);
                RefreshScreenSaver();
            }
        });

        //
        //  IMM Handler
        //
        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                if( imm == null )
                    return;

                imm.hideSoftInputFromWindow(mEditCabinet.getWindowToken(), 0);
            }
        });

        //
        //  Initialize Tab
        //
        AddTabs();
        RegisterListener();
        UpdateUniformityCorrection();
    }

    private void AddTabs() {
        mTabHost = (TabHost) findViewById(R.id.tabHost);
        mTabHost.setup();

        TabHost.TabSpec tabSpec0 = mTabHost.newTabSpec("TAB");
        tabSpec0.setIndicator("Uniformity");
        tabSpec0.setContent(R.id.tabUniformityCorrection);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Pixel Correction");
        tabSpec1.setContent(R.id.tabDotCorrection);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Pixel Correction Extract");
        tabSpec2.setContent(R.id.tabDotCorrectionExtract);

        TabHost.TabSpec tabSpec3 = mTabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("White Seam Value");
        tabSpec3.setContent(R.id.tabWhiteSeamValue);

        TabHost.TabSpec tabSpec4 = mTabHost.newTabSpec("TAB4");
        tabSpec4.setIndicator("Global");
        tabSpec4.setContent(R.id.tabGlobal);

        TabHost.TabSpec tabSpec5 = mTabHost.newTabSpec("TAB5");
        tabSpec5.setIndicator("Set up");
        tabSpec5.setContent(R.id.tabSetup);

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);
        mTabHost.addTab(tabSpec2);
        mTabHost.addTab(tabSpec3);
        mTabHost.addTab(tabSpec4);
        mTabHost.addTab(tabSpec5);

        mTabHost.setOnTabChangedListener(mTabChange);
        mTabHost.setCurrentTab(0);
    }

    private String mTabPrevious = "TAB0";
    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( mTabPrevious.equals("TAB3") ) ClearWhiteSeamValue();

            if( tabId.equals("TAB0") ) UpdateUniformityCorrection();
            if( tabId.equals("TAB1") ) UpdateDotCorrection();
            if( tabId.equals("TAB2") ) UpdateDotCorrectionExtract();
            if( tabId.equals("TAB3") ) UpdateWhiteSeamValue();
            if( tabId.equals("TAB4") ) UpdateGlobal();
            if( tabId.equals("TAB5") ) UpdateSetup();

            mTabPrevious = tabId;
        }
    };

    private void UpdateUniformityCorrection() {
        //  Update Button
        String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
        String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
        if( (usbPfpga != null && usbPfpga.length != 0) || (usbUniformity != null && usbUniformity.length != 0) ) {
            mBtnUpdateUniformity.setEnabled(true);

            if( usbPfpga != null ) for(String file : usbPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
            if( usbUniformity != null ) for(String file : usbUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        }

        //  Apply Button
        String[] internalPfpga = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        String[] internalUniformity =FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
        if( (internalPfpga != null && internalPfpga.length != 0) || (internalUniformity != null && internalUniformity.length != 0) ) {
            mBtnApplyUniformity.setEnabled(true);

            if( internalPfpga != null ) for(String file : internalPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
            if( internalUniformity != null ) for(String file : internalUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        }

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PFPGA_REG_READ,
                getApplicationContext(),
                new int[]{CinemaInfo.REG_PFPGA_NUC_EN},
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

                        mBtnEnableUniformity.setText( ((Integer)values[0] == 1) ? "ENABLE" : "DISABLE" );
                        HideProgress();
                    }
                },
                null
        );

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

    private void UpdateDotCorrection() {
        String[] usbDot = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( usbDot == null || usbDot.length == 0 )
            return;

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PIXEL_CORRECTION_ADAPTER,
                getApplicationContext(),
                mAdapterDotCorrect,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        HideProgress();
                    }
                },
                null
        );

        mBtnDotCorrectCheckAll.setEnabled(true);
        mBtnDotCorrectUnCheckAll.setEnabled(true);
        mBtnDotCorrectApply.setEnabled(true);
    }

    private void ClearWhiteSeamValue() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                new int[] { 0x0189 },
                new int[] { 0x0000 },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        UnregisterListener();
                        mCheckWhiteSeamEmulation.setChecked(false);
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        RegisterListener();
                        HideProgress();
                    }
                },
                null
        );
    }

    private void UpdateWhiteSeamValue() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_WHITESEAM_READ,
                getApplicationContext(),
                mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                mCheckWhiteSeamEmulation.isChecked(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        UnregisterListener();

                        mBtnWhiteSeamApply.setEnabled(mCheckWhiteSeamEmulation.isChecked());
                        for( VdSpinCtrl spin : mSpinWhiteSeam )
                            spin.setEnabled(mCheckWhiteSeamEmulation.isChecked());
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if (!(values instanceof Integer[]))
                            return;

                        for (int i = 0; i < values.length; i++) {
                            mSpinWhiteSeam[i].SetValue((Integer) values[i]);
                        }

                        RegisterListener();
                        HideProgress();
                    }
                },
                null
        );
    }

    private void UpdateDotCorrectionExtract() {
        String result = GetExternalStorage();
        if( result != null ) {
            mBtnDotCorrectExtract.setEnabled(true);
        }
        else {
            mBtnDotCorrectExtract.setEnabled(false);
        }
    }

    private void UpdateGlobal() {
        mSpinSyncWidth.setEnabled(mCinemaInfo.IsMode3D());
        mSpinSyncDelay.setEnabled(mCinemaInfo.IsMode3D());
        mCheckSyncReverse.setEnabled(mCinemaInfo.IsMode3D());
        mBtnApplySyncWidth.setEnabled(mCinemaInfo.IsMode3D());
        mBtnApplySyncDelay.setEnabled(mCinemaInfo.IsMode3D());

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                new int[] {
                        CinemaInfo.REG_TCON_0x018B,     // Sync Width
                        CinemaInfo.REG_TCON_0x018C,     // Sync Delay
                        CinemaInfo.REG_TCON_0x018A,     // Sync Reverse
                        CinemaInfo.REG_TCON_0x018E,     // Zero Scale
                        CinemaInfo.REG_TCON_0x0192,     // Seam
                        CinemaInfo.REG_TCON_0x0055,     // Module
                        CinemaInfo.REG_TCON_0x0004,     // Xyz
                        CinemaInfo.REG_TCON_0x0100,     // LedOpenDetection
                        CinemaInfo.REG_TCON_0x011E      // Lod Removal
                },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        Log.i(VD_DTAG, ">> Global Register Read Start.");
                        ShowProgress();
                        UnregisterListener();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        mSpinSyncWidth.SetValue( (Integer)values[0] );
                        mSpinSyncDelay.SetValue( (Integer)values[1] );

                        mCheckSyncReverse.setChecked( (Integer)values[2] != 0 ) ;
                        mCheckZeroScale.setChecked( (Integer)values[3] != 0 );
                        mCheckSeam.setChecked( (Integer)values[4] != 0 );
                        mCheckModule.setChecked( (Integer)values[5] != 0 );
                        mCheckXyzInput.setChecked( (Integer)values[6] != 0 );
                        mCheckLedOpenDetection.setChecked( (Integer)values[7] != 0 );
                        mCheckLodRemoval.setChecked( (Integer)values[8] != 0 );

                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf( values[0]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf( values[1]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf( values[2]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018E, String.valueOf( values[3]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x0192, String.valueOf( values[4]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x0055, String.valueOf( values[5]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x0004, String.valueOf( values[6]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x0100, String.valueOf( values[7]) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf( values[8]) );
                        mCinemaInfo.UpdateDefaultRegister();

                        Log.i(VD_DTAG, ">> Global Register Read Done.");

                        RegisterListener();
                        HideProgress();
                    }
                },
                null
        );
    }

    private void UpdateSetup() {
        String cabinetNum = mCinemaInfo.GetValue(CinemaInfo.KEY_CABINET_NUM);
        if( cabinetNum == null ) {
            cabinetNum = String.valueOf(0);
            mCinemaInfo.SetValue(CinemaInfo.KEY_CABINET_NUM, cabinetNum);
        }
        mEditCabinet.setText(cabinetNum);

        for( int i = 0; i < CinemaService.OFF_TIME.length; i++ ) {
            String offTime = mCinemaInfo.GetValue(CinemaInfo.KEY_SCREEN_SAVING);
            if( offTime == null ) {
                offTime = String.valueOf(CinemaService.OFF_TIME[0]);
                mCinemaInfo.SetValue(CinemaInfo.KEY_SCREEN_SAVING, offTime);
            }

            if( offTime.equals(CinemaService.OFF_TIME[i]) ) {
                mSpinnerSuspendTime.setSelection(i);
                break;
            }
        }
    }

    private String GetExternalStorage() {
        String[] result = FileManager.GetExternalPath();
        return (result != null && result.length != 0) ? result[0] : null;
    }

    private void RegisterListener() {
        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
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
                        null,
                        null,
                        null
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_WHITESEAM_READ,
                        getApplicationContext(),
                        mSpinnerWhiteSeamCabinetId.getSelectedItemPosition(),
                        b,
                        null,
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                for( int i = 0; i < values.length; i++ ) {
                                    mSpinWhiteSeam[i].SetValue((Integer)values[i]);
                                }
                            }
                        },
                        null
                );
            }
        });

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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return ;

                                for( int i = 0; i < values.length; i++ ) {
                                    mSpinWhiteSeam[i].SetValue((Integer)values[i]);
                                }
                                HideProgress();
                            }
                        },
                        null
                );
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        mBtnApplySyncWidth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_WIDTH. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncWidth.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mSpinSyncWidth.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_WIDTH. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncWidth.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mSpinSyncWidth.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018B},
                        new int[]{mSpinSyncWidth.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_WIDTH. ( %d )", mSpinSyncWidth.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(mSpinSyncWidth.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
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
                        new int[]{CinemaInfo.REG_TCON_0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mSpinSyncDelay.SetOnChangeListener(new VdSpinCtrl.OnChangeListener() {
            @Override
            public void onChange(int value) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mSpinSyncDelay.SetOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018C},
                        new int[]{mSpinSyncDelay.GetValue()},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelay.GetValue()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelay.GetValue()) );
                                HideProgress();
                            }
                        },
                        null
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
                        new int[]{CinemaInfo.REG_TCON_0x018A},
                        new int[]{mCheckSyncReverse.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change SYNC_REVERSE. ( %b )", mCheckSyncReverse.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf(mCheckSyncReverse.isChecked() ? 1 : 0) );
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckZeroScale.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x018E},
                        new int[]{mCheckZeroScale.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Zero Scale. ( %b )", mCheckZeroScale.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x018E,
                                        String.valueOf(mCheckZeroScale.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckSeam.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x0192},
                        new int[]{mCheckSeam.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Seam. ( %b )", mCheckSeam.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x0192,
                                        String.valueOf(mCheckSeam.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckModule.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x0055},
                        new int[]{mCheckModule.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Module. ( %b )", mCheckModule.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x0055,
                                        String.valueOf(mCheckModule.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckXyzInput.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x0004},
                        new int[]{mCheckXyzInput.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change XYX Input. ( %b )", mCheckXyzInput.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x0004,
                                        String.valueOf(mCheckXyzInput.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckLedOpenDetection.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x0100},
                        new int[]{mCheckLedOpenDetection.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change Led Open Detection. ( %b )", mCheckLedOpenDetection.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x0100,
                                        String.valueOf(mCheckLedOpenDetection.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        mCheckLodRemoval.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{CinemaInfo.REG_TCON_0x011E},
                        new int[]{mCheckLodRemoval.isChecked() ? 0x0001 : 0x0000},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Change LOD Removal. ( %b )", mCheckLodRemoval.isChecked()));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mCinemaInfo.SetValue(
                                        CinemaInfo.KEY_TREG_0x011E,
                                        String.valueOf(mCheckLodRemoval.isChecked() ? 1 : 0)
                                );
                                mCinemaInfo.UpdateDefaultRegister();
                                HideProgress();
                            }
                        },
                        null
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
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                                Log.i(VD_DTAG, String.format(Locale.US, "Apply LOD Reset."));
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });
    }

    private void UnregisterListener() {
        mCheckWhiteSeamEmulation.setOnCheckedChangeListener(null);
        mSpinnerWhiteSeamCabinetId.setOnItemSelectedListener(null);
        mBtnApplySyncWidth.setOnClickListener(null);
        mSpinSyncWidth.SetOnChangeListener(null);
        mSpinSyncWidth.SetOnEditorActionListener(null);
        mBtnApplySyncDelay.setOnClickListener(null);
        mSpinSyncDelay.SetOnChangeListener(null);
        mSpinSyncDelay.SetOnEditorActionListener(null);
        mCheckSyncReverse.setOnCheckedChangeListener(null);
        mCheckZeroScale.setOnCheckedChangeListener(null);
        mCheckSeam.setOnCheckedChangeListener(null);
        mCheckModule.setOnCheckedChangeListener(null);
        mCheckXyzInput.setOnCheckedChangeListener(null);
        mCheckLedOpenDetection.setOnCheckedChangeListener(null);
        mCheckLodRemoval.setOnCheckedChangeListener(null);
        mBtnLodReset.setOnClickListener(null);
    }
}
