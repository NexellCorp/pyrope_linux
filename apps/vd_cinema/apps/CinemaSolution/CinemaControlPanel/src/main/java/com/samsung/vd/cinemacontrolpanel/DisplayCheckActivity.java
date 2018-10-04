package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ExpandableListView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DisplayCheckActivity";

    private String[] mFuncName = {
            "DCI",                          //  0, Run/Stop
            "Color Bar",                    //  1, Run/Stop
            "Full Screen Color",            //  2, Run/Stop
            "Gary Scale ( Horizontal )",    //  3, Run/Stop
            "Dot Pattern / Checker Board",  //  4, Run/Stop
            "Diagonal Pattern",             //  5, Run/Stop,   Toggle Menu
            "Cabinet ID",                   //  6, Run/Stop,   Toggle Menu
            "Bypass",                       //  7, En/Disable, Toggle Menu
            "XYZ to RGB Conversion",        //  8, En/Disable, Toggle Menu
            "Gamut Mapping",                //  9, En/Disable, Toggle Menu
            "Pixel Correction",             // 10, En/Disable, Toggle Menu
            "Edge Correction",              // 11, En/Disable, Toggle Menu
            "Module Correction",            // 12, En/Disable, Toggle Menu
    };

    private String[] mDci = {
            "Red-1",            "Green-1",          "Blue-1",
            "Cyan-1",           "Magenta-1",        "Yellow-1",
            "Red-2",            "Green-2",          "Blue-2",
            "Cyan-2",           "Magenta-2",        "Yellow-2",
            "White-1",          "White-2",          "White-3",
    };

    private String[] mFullScreenColor = {
            "White 100%",       "Gray 90%",         "Gray 80%",
            "Gray 70%",         "Gray 60%",         "Gray 50%",
            "Gray 40%",         "Gray 30%",         "Gray 20%",
            "Gray 10%",         "Black 0%",         "Red 100%",
            "Green 100%",       "Blue 100%",        "Magenta 100%",
            "Cyan 100%",        "Yellow 100%",
    };

    private String[] mGrayScale = {
            "Gray 16-Step",     "Gray 32-Step",     "Gray 64-Step",
            "Gray 128-Step",    "Gray 256-Step",    "Gray 512-Step",
            "Gray 2048-Step",   "Red 2048-Step",    "Green 2048-Step",
            "Blue 2048-Step",
    };

    private String[] mDiagonalPattern = {
            "Right Down ( ↘ )", "Right Up ( ↗ )",
    };

    private String[][] mPatternName = {
            mDci,
            new String[0],
            mFullScreenColor,
            mGrayScale,
            new String[0],
            mDiagonalPattern,
            new String[0],
            new String[0],
            new String[0],
            new String[0],
            new String[0],
            new String[0],
            new String[0],
    };

    private int[] mTestPatternReg = {
            CinemaInfo.REG_TCON_BOX_R,
            CinemaInfo.REG_TCON_BOX_G,
            CinemaInfo.REG_TCON_BOX_B,
    };

    private int[] mTestPatternDat = {
            0x0000,
            0x0000,
            0x0000,
    };

    private int[] mBypassReg = {
            CinemaInfo.REG_TCON_XYZ_TO_RGB,   //  8, XYZ to RGB Conversion
            CinemaInfo.REG_TCON_CC_CABINET,   //  9, Gamut Mapping
            CinemaInfo.REG_TCON_CC_PIXEL,     // 10, Pixel Correction
            CinemaInfo.REG_TCON_SEAM_ON,      // 11, Edge Correction
            CinemaInfo.REG_TCON_CC_MODULE,    // 12, Module Correction
            CinemaInfo.REG_TCON_PATTERN,      // This is not menu. Just background setting.
    };

    private int[] mBypassDat = {
            0x0000,
            0x0000,
            0x0000,
            0x0000,
            0x0000,
            0x0000,
    };

    private int[] mScanModeReg = {
            CinemaInfo.REG_TCON_SCAN_MODE,
    };

    private int[] mScanModeDat = {
            0x0000,
    };

    private CinemaInfo mCinemaInfo;
    private byte[]  mCabinet;

    private TabHost mTabHost;

    private SelectRunAdapter mAdapterTestPattern;
    private StatusDescribeExpandableAdapter mAdapterAccumulation;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

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
        titleBar.SetTitle( "Cinema LED Display System - Display Check" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final View view = v;
                if( mTabPrevious.equals("TAB0") ) {
                    ClearTestPattern(
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    Launch(view.getContext(), TopActivity.class, 100);
                                }
                            }
                    );
                }
                else {
                    Launch(view.getContext(), TopActivity.class);
                }
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mTabPrevious.equals("TAB0") ) ClearTestPattern();
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

        //
        //  TEST PATTERN
        //
        ListView listViewTestPattern = (ListView)findViewById(R.id.listview_test_pattern);
        listViewTestPattern.addFooterView( listViewFooter );

        mAdapterTestPattern = new SelectRunAdapter(this, R.layout.listview_row_select_run);
        listViewTestPattern.setAdapter( mAdapterTestPattern );

        for(int i = 0; i < mFuncName.length; i++ ) {
            mAdapterTestPattern.add( new SelectRunInfo(
                    mFuncName[i],
                    mPatternName[i],
                    (i >= 7) ? new String[]{"ENABLE", "DISABLE"} : new String[]{"RUN", "STOP"},
                    (i >= 5),
                    false,
                    new SelectRunAdapter.OnClickListener() {
                        @Override
                        public void onClickListener(int index, int spinnerIndex, boolean status ) {
                            if( status && mFuncName[index].equals("Diagonal Pattern") ) {
                                CinemaTask.GetInstance().Run(
                                        CinemaTask.CMD_TCON_REG_WRITE,
                                        getApplicationContext(),
                                        mScanModeReg,
                                        new int[]{0x0000},
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

                            {
                                CinemaTask.GetInstance().Run(
                                        CinemaTask.CMD_TEST_PATTERN,
                                        getApplicationContext(),
                                        index,
                                        spinnerIndex,
                                        status,
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
                                        new CinemaTask.ProgressUpdateCallback() {
                                            @Override
                                            public void onProgressUpdate(Object[] values) {
                                                if( !(values instanceof Integer[]) )
                                                    return;

                                                SelectRunInfo info = mAdapterTestPattern.getItem((Integer)values[0] + 8);
                                                if( info != null ) {
                                                    info.SetStatus((Integer)values[1] == 0x0001);
                                                    mAdapterTestPattern.notifyDataSetChanged();
                                                }
                                            }
                                        }
                                );
                            }

                            if( !status && mFuncName[index].equals("Diagonal Pattern") ) {
                                CinemaTask.GetInstance().Run(
                                        CinemaTask.CMD_TCON_REG_WRITE,
                                        getApplicationContext(),
                                        mScanModeReg,
                                        mScanModeDat,
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
                    }
            }));

            mAdapterTestPattern.notifyDataSetChanged();
        }

        //
        //  ACCUMULATION TIME
        //
        ExpandableListView listViewAccumulation = (ExpandableListView)findViewById(R.id.listview_accumulation);
        listViewAccumulation.addFooterView( listViewFooter );

        mAdapterAccumulation = new StatusDescribeExpandableAdapter(this, R.layout.listview_row_status_describe, R.layout.listview_row_status_describe);
        listViewAccumulation.setAdapter( mAdapterAccumulation );

        //
        //  Initialize Tab
        //
        AddTabs();
        UpdateTestPattern();

        //
        //
        //
        mTabHost.getTabWidget().getChildTabViewAt(1).setEnabled(false);
        ((TextView) mTabHost.getTabWidget().getChildAt(1).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        mTabHost.getTabWidget().getChildTabViewAt(2).setEnabled(false);
        ((TextView) mTabHost.getTabWidget().getChildAt(2).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        //
        //
        //
        this.RegisterScreenSaverCallback(new CinemaService.ScreenSaverCallback() {
            @Override
            public void onPrepare() {
                CinemaTask.GetInstance().ClearTestPattern(getApplicationContext());
                CinemaTask.GetInstance().TconRegWrite(getApplicationContext(), mTestPatternReg, mTestPatternDat);
                CinemaTask.GetInstance().TconRegWrite(getApplicationContext(), mBypassReg, mBypassDat);
                CinemaTask.GetInstance().TconRegWrite(getApplicationContext(), mScanModeReg, mScanModeDat);
            }
        });
    }

    private void AddTabs() {
        mTabHost = (TabHost) findViewById(R.id.tabHost);
        mTabHost.setup();

        TabHost.TabSpec tabSpec0 = mTabHost.newTabSpec("TAB0");
        tabSpec0.setIndicator("Test Pattern");
        tabSpec0.setContent(R.id.tabTestPattern);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Led Accumulation time");
        tabSpec1.setContent(R.id.tabLedAccumulation);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Additional Information");
        tabSpec2.setContent(R.id.tabAdditionalInformation);

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);
        mTabHost.addTab(tabSpec2);

        mTabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        mTabHost.setCurrentTab(0);
    }

    private String mTabPrevious = "TAB0";
    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( mTabPrevious.equals("TAB0") ) ClearTestPattern();

            if( tabId.equals("TAB0") ) UpdateTestPattern();
            if( tabId.equals("TAB1") ) UpdateAccumulation();
            if( tabId.equals("TAB2") ) UpdateAdditionalInformation();

            mTabPrevious = tabId;
        }
    };

    private void UpdateTestPattern() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                mTestPatternReg,
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

                        for( int i = 0 ; i < values.length; i++ )
                        {
                            mTestPatternDat[i] = (Integer)values[i];
                            Log.i(VD_DTAG, String.format(">>> backup test pattern register. ( reg: 0x%04X, dat: 0x%04X )", mTestPatternReg[i], mTestPatternDat[i]) );
                        }
                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                mBypassReg,
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

                        for( int i = 0 ; i < values.length; i++ )
                        {
                            mBypassDat[i] = (Integer)values[i];
                            Log.i(VD_DTAG, String.format(">>> backup bypass register. ( reg: 0x%04X, dat: 0x%04X )", mBypassReg[i], mBypassDat[i]) );

                            SelectRunInfo info = mAdapterTestPattern.getItem(i+8);
                            if( info != null ) {
                                info.SetStatus((Integer)values[1] == 0x0001 );
                                mAdapterTestPattern.notifyDataSetChanged();
                            }
                        }
                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                mScanModeReg,
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

                        for( int i = 0 ; i < values.length; i++ )
                        {
                            mScanModeDat[i] = (Integer)values[i];
                            Log.i(VD_DTAG, String.format(">>> backup scan mode register. ( reg: 0x%04X, dat: 0x%04X )", mScanModeReg[i], mScanModeDat[i]) );
                        }
                        HideProgress();
                    }
                },
                null
        );
    }


    private void ClearTestPattern() {
        ClearTestPattern(null);
    }

    private void ClearTestPattern(final CinemaTask.PostExecuteCallback postExecute) {
        for( int i = 0; i < 7; i++ ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_TEST_PATTERN,
                    getApplicationContext(),
                    i,
                    0,
                    false,
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

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                mTestPatternReg,
                mTestPatternDat,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        for( int i = 0; i < mTestPatternReg.length; i++ ) {
                            Log.i(VD_DTAG, String.format(">>> restore test pattern register. ( reg: 0x%04X, dat: 0x%04X )", mTestPatternReg[i], mTestPatternDat[i]) );
                        }

                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                mBypassReg,
                mBypassDat,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        for( int i = 0; i < mBypassReg.length; i++ ) {
                            Log.i(VD_DTAG, String.format(">>> restore bypass register. ( reg: 0x%04X, dat: 0x%04X )", mBypassReg[i], mBypassDat[i]) );
                        }

                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                mScanModeReg,
                mScanModeDat,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        for( int i = 0; i < mScanModeReg.length; i++ ) {
                            Log.i(VD_DTAG, String.format(">>> restore scan mode register. ( reg: 0x%04X, dat: 0x%04X )", mScanModeReg[i], mScanModeDat[i]) );
                        }

                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_POST_EXECUTE,
                getApplicationContext(),
                0,
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

                        if( null != postExecute )
                            postExecute.onPostExecute(values);
                    }
                },
                null
        );
    }

    private void UpdateAccumulation() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_ACCUMULATION,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mAdapterAccumulation.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        if( mAdapterAccumulation.getGroupCount() == (Integer)values[0] ) {
                            mAdapterAccumulation.addGroup( new StatusDescribeExpandableInfo(
                                    String.format( Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])
                            )));
                        }

                        mAdapterAccumulation.addChild(
                                (Integer)values[0],
                                new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", (Integer)values[1]),
                                        (Integer)values[2] == CinemaInfo.RET_ERROR ? "Error" :
                                        String.format( Locale.US, "%d mSec", (Integer)values[2] )
                                )
                        );

                        mAdapterAccumulation.notifyDataSetChanged();
                    }
                }
        );
    }

    private void UpdateAdditionalInformation() {
        Log.i(VD_DTAG, "Not Implementation.");
    }
}
