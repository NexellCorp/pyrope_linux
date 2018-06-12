package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DisplayCheckActivity";

    private String[] mFuncName = {
            "DCI",
            "Color Bar",
            "Full Screen Color",
            "Gary Scale ( Horizontal )",
            "Dot Pattern / Checker Board",
            "Diagonal Pattern",
            "Cabinet ID",
            "Dot Correction",
            "Gamut Mapping",
            "XYZ to RGB",
            "White Seam Value",
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
    };

    private int[] mPatternReg = {
            NxCinemaCtrl.REG_TCON_FLASH_CC,     // restore default value
            NxCinemaCtrl.REG_TCON_CC_MODULE,    // restore default value
            NxCinemaCtrl.REG_TCON_XYZ_TO_RGB,   // restore default value
            NxCinemaCtrl.REG_TCON_SEAM_ON,      // restore default value
    };

    private int[] mPatternDat = {
            0x0000,
            0x0000,
            0x0000,
            0x0000,
    };

    private SelectRunAdapter mAdapterTestPattern;
    private StatusDescribeExpandableAdapter mAdapterAccumulation;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

        //
        // Configuration TitleBar
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

                StopTestPattern(
                        null,
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                Launch(view.getContext(), TopActivity.class, 100);
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                StopTestPattern();
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
        // Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        AddTabs();

        //
        //
        //
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //
        //  TEST PATTERN
        //
        ListView listViewTestPattern = (ListView)findViewById(R.id.listview_test_pattern);
        listViewTestPattern.addFooterView( listViewFooter );

        mAdapterTestPattern = new SelectRunAdapter(this, R.layout.listview_row_select_run);
        listViewTestPattern.setAdapter( mAdapterTestPattern );

        UpdateTestPattern();

        for(int i = 0; i < mFuncName.length; i++ ) {
            boolean toggle = (i >= 5);
            String[] btnText = (i >= 7) ? new String[]{"ENABLE", "DISABLE"} : new String[]{"RUN", "STOP"};
            boolean status = (i>=7) && (mPatternDat[i-7] == 0x0001);

            mAdapterTestPattern.add( new SelectRunInfo(mFuncName[i], mPatternName[i], btnText, toggle, status, new SelectRunAdapter.OnClickListener() {
                @Override
                public void onClickListener(int index, int spinnerIndex, boolean status ) {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_TEST_PATTERN,
                            getApplicationContext(),
                            mAdapterTestPattern,
                            index,
                            spinnerIndex,
                            status,
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
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec0 = tabHost.newTabSpec("TAB0");
        tabSpec0.setIndicator("Test Pattern");
        tabSpec0.setContent(R.id.tabTestPattern);

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Led Accumulation time");
        tabSpec1.setContent(R.id.tabLedAccumulation);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Additional Information");
        tabSpec2.setContent(R.id.tabAdditionalInformation);

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);

        tabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        tabHost.setCurrentTab(0);

        tabHost.getTabWidget().getChildTabViewAt(1).setEnabled(false);
        ((TextView)tabHost.getTabWidget().getChildAt(1).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        tabHost.getTabWidget().getChildTabViewAt(2).setEnabled(false);
        ((TextView)tabHost.getTabWidget().getChildAt(2).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
    }

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateTestPattern();
            if( tabId.equals("TAB1") ) UpdateAccumulation();
            if( tabId.equals("TAB2") ) UpdateAdditionalInformation();
        }
    };

    private void UpdateTestPattern() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                mPatternReg,
                null,
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        ShowProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {
                        mPatternDat = values;
                        for( int i = 0 ; i < mPatternReg.length; i++ )
                        {
                            Log.i(VD_DTAG, String.format(">>> read default pattern register. ( reg: 0x%04X, dat: 0x%04X )", mPatternReg[i], mPatternDat[i]) );
                        }
                        HideProgress();
                    }
                }
        );
    }

    private void UpdateAccumulation() {
        StopTestPattern();

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_ACCUMULATION,
                getApplicationContext(),
                mAdapterTestPattern,
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

    private void UpdateAdditionalInformation() {
        StopTestPattern();
    }

    private void StopTestPattern() {
        StopTestPattern(null, null);
    }

    private void StopTestPattern(final CinemaTask.PreExecuteCallback preExecute, final CinemaTask.PostExecuteCallback postExecute) {
        for( int i = 0; i < mFuncName.length - mPatternReg.length + 1; i++ ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_TEST_PATTERN,
                    getApplicationContext(),
                    mAdapterTestPattern,
                    i,
                    0,
                    false,
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

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_WRITE,
                getApplicationContext(),
                mPatternReg,
                mPatternDat,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        if( null != preExecute )
                            preExecute.onPreExecute();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        if( null != postExecute )
                            postExecute.onPostExecute();

                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {
                        if( null != postExecute )
                            postExecute.onPostExecute(values);

                        HideProgress();
                    }
                }
        );
    }
}
