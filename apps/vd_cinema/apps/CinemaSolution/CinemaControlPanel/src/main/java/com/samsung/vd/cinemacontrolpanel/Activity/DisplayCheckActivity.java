package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeExpandableAdapter;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.SelectRunInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.SelectRunAdapter;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayCheckAccumulation;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayCheckStopTestPattern;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayCheckTestPattern;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DisplayCheckUpdateTestPattern;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends BaseActivity {
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
            0x0044,     // REG_FLASH_CC     :: restore default value
            0x0052,     // REG_CC_MODULE    :: restore default value
            0x0004,     // REG_XYZ_TO_RGB   :: restore default value
            0x0192,     // REG_SEAM_ON      :: restore default value
    };

    private int[] mPatternDat = {
            0x0000,
            0x0000,
            0x0000,
            0x0000,
    };

    private SelectRunAdapter mAdapterTestPattern;
    private StatusDescribeExpandableAdapter mAdapterAccumulation;
    private TabHost tabHost;
    private byte[]  mCabinet;

    View.OnClickListener onRefreshClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            bRefreshFlag = true;
            int tabIdx = tabHost.getCurrentTab();
            if( 0 == tabIdx ) {
                UpdateTestPattern();
            }
            else if( 1 == tabIdx ) {
                UpdateAccumulation();
            }
            else if( 2 == tabIdx ) {
                UpdateAdditionalInformation();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

        contentParent = (RelativeLayout)findViewById(R.id.contentParentDisplayCheck);
        bI2CFailFlag = false;
        bRefreshFlag = false;
        setOnRefreshClickListener(onRefreshClickListener);

        // Set Title Bar and Status Bar
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Display Check" , new Intent(getApplicationContext(), TopActivity.class) );

        AddTabs();

        //
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();

        //  TEST PATTERN
        ListView listViewTestPattern = (ListView)findViewById(R.id.listview_test_pattern);
        listViewTestPattern.addFooterView( listViewFooter );

        mAdapterTestPattern = new SelectRunAdapter(this, R.layout.listview_row_select_run);
        listViewTestPattern.setAdapter( mAdapterTestPattern );

        UpdateTestPattern();
        //


        //  ACCUMULATION TIME
        ExpandableListView listViewAccumulation = (ExpandableListView)findViewById(R.id.listview_accumulation);
        listViewAccumulation.addFooterView( listViewFooter );

        mAdapterAccumulation = new StatusDescribeExpandableAdapter(this, R.layout.listview_row_status_describe, R.layout.listview_row_status_describe);
        listViewAccumulation.setAdapter( mAdapterAccumulation );
    }

    private void AddTabs() {
        tabHost = (TabHost) findViewById(R.id.tabHost);
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
            if( tabId.equals("TAB0") ) {
                contentParent.setVisibility(View.VISIBLE);
                UpdateTestPattern();
            }
            if( tabId.equals("TAB1") ) {
                contentParent.setVisibility(View.VISIBLE);
                UpdateAccumulation();
            }
            if( tabId.equals("TAB2") ) {
                contentParent.setVisibility(View.VISIBLE);
                UpdateAdditionalInformation();
            }
        }
    };

    private void UpdateTestPattern() {
        NXAsync.getInstance().getDisplayCheckAsyncParam().setmPatternDat(mPatternDat);
        NXAsync.getInstance().Execute(CMD_DisplayCheckUpdateTestPattern ,asyncCallbackUpdateTestPattern );
    }

    private void UpdateAccumulation() {
        StopTestPattern();

        NXAsync.getInstance().getDisplayCheckAsyncParam().setDisplayCheckAdapter(mAdapterAccumulation);
//        new AsyncTaskAccumulation(mAdapterAccumulation).execute();
        NXAsync.getInstance().Execute(CMD_DisplayCheckAccumulation , asyncCallbackAccumulation);
    }

    private void UpdateAdditionalInformation() {
        StopTestPattern();
    }

    private void StopTestPattern() {
        NXAsync.getInstance().getDisplayCheckAsyncParam().setmAdapterTestPattern(mAdapterTestPattern);
        NXAsync.getInstance().Execute(CMD_DisplayCheckStopTestPattern , asyncCallback);
    }


    NXAsync.AsyncCallback asyncCallbackTestPattern = new NXAsync.AsyncCallback(){
        @Override
        public void onPreExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show(DisplayCheckActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    int[] values = NXAsync.getInstance().getDisplayCheckAsyncParam().getResDat();
                    //
                    //
                    //
                    for( int i = 0; i < mPatternReg.length; i++ )
                    {
                        SelectRunInfo info = mAdapterTestPattern.getItem(i+7);
                        if( info == null )
                            continue;

                        info.SetStatus(values[i] == 0x0001);
                    }

                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(NXAsync.getInstance().isI2CFailed()) {
                        ShowMessage("I2C Failed.. try again later");
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackTestPattern

    NXAsync.AsyncCallback asyncCallback = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show(DisplayCheckActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(NXAsync.getInstance().isI2CFailed()) {
                        ShowMessage("I2C Failed.. try again later");
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallback


    NXAsync.AsyncCallback asyncCallbackAccumulation = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show(DisplayCheckActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDisplayCheckAsyncParam().getDisplayCheckAdapter().notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
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
    }; //asyncCallbackAccumulation

    NXAsync.AsyncCallback asyncCallbackUpdateTestPattern = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show(DisplayCheckActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DisplayCheckActivity.this.runOnUiThread(new Runnable() {
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
                    mPatternDat = NXAsync.getInstance().getDisplayCheckAsyncParam().getmPatternDat();

                    for(int i = 0; i < mFuncName.length; i++ ) {
                        boolean toggle = (i >= 5);
                        String[] btnText = (i >= 7) ? new String[]{"ENABLE", "DISABLE"} : new String[]{"RUN", "STOP"};
                        boolean status = (i>=7) && (mPatternDat[i-7] == 0x0001);

                        mAdapterTestPattern.add( new SelectRunInfo(mFuncName[i], mPatternName[i], btnText, toggle, status, new SelectRunAdapter.OnClickListener() {
                            @Override
                            public void onClickListener(int index, int spinnerIndex, boolean status ) {
//                    RunTestPattern( index, spinnerIndex, status );
                                NXAsync.getInstance().getDisplayCheckAsyncParam().setDisplayCheckFuncIndex(index);
                                NXAsync.getInstance().getDisplayCheckAsyncParam().setDisplayCheckPatternIndex(spinnerIndex);
                                NXAsync.getInstance().getDisplayCheckAsyncParam().setDisplayCheckStatus(status);
//                    new AsyncTaskTestPattern( index, spinnerIndex, status ).execute();
                                NXAsync.getInstance().Execute(CMD_DisplayCheckTestPattern , asyncCallbackTestPattern);
                            }
                        }));

                        mAdapterTestPattern.notifyDataSetChanged();
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackUpdateTestPattern

} // DisplayCheckActivity
