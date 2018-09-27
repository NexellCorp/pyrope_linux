package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
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
public class DiagnosticsActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

    private CinemaInfo mCinemaInfo;
    private byte[]  mCabinet;

    private TabHost mTabHost;

    private TextView mTextTconStatus;
    private TextView mTextTconLvds;
    private TextView mTextLedOpen;
    private TextView mTextCabinetDoor;

    private StatusSimpleAdapter mAdapterTconStatus;
    private StatusSimpleAdapter mAdapterTconLvds;
    private StatusDetailAdapter mAdapterLedOpen;
    private StatusSimpleAdapter mAdapterCabinetDoor;
    private StatusSimpleAdapter mAdapterPeripheral;
    private StatusDescribeAdapter mAdapterVersion;

    int[] mRegFullWhite = {
            0x0004, // TCON_REG_XYZ_TO_RGB
            0x0024, // TCON_REG_PATTERN
            0x0025, // TCON_REG_LEFT
            0x0026, // TCON_REG_RIGHT
            0x0027, // TCON_REG_TOP
            0x0028, // TCON_REG_DOWN
            0x0029, // TCON_REG_BOX_R
            0x002A, // TCON_REG_BOX_G
            0x002B, // TCON_REG_BOX_B
    };

    int[] mDefFullWhite = {
            0,      // TCON_REG_XYZ_TO_RGB
            10,     // TCON_REG_PATTERN
            0,      // TCON_REG_LEFT
            1024,   // TCON_REG_RIGHT
            0,      // TCON_REG_TOP
            2160,   // TCON_REG_DOWN
            4095,   // TCON_REG_BOX_R
            4095,   // TCON_REG_BOX_G
            4095,   // TCON_REG_BOX_B
    };

    int[] mDatFullWhite = new int[mRegFullWhite.length];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

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
        titleBar.SetTitle( "Cinema LED Display System - Diagnostics" );
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
                if( mTabPrevious.equals("TAB3") ) {
                    ClearCabinetDoor(
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    Launch(view.getContext(), TopActivity.class, 100);
                                }
                            }
                    );
                }
                else {
                    Launch(v.getContext(), TopActivity.class);
                }
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mTabPrevious.equals("TAB3") ) ClearCabinetDoor();
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
        //  TCON STATUS
        //
        ListView listViewTconStatus = (ListView)findViewById(R.id.listView_tcon_status);
        listViewTconStatus.addFooterView(listViewFooter);

        mAdapterTconStatus = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewTconStatus.setAdapter( mAdapterTconStatus );

        mTextTconStatus = (TextView)findViewById(R.id.listview_tcon_status_text);
        mTextTconStatus.setVisibility(View.GONE);

        //
        //  TCON LVDS
        //
        ListView listViewTconLvds = (ListView)findViewById(R.id.listView_tcon_lvds);
        listViewTconLvds.addFooterView(listViewFooter);

        mAdapterTconLvds = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewTconLvds.setAdapter( mAdapterTconLvds );

        mTextTconLvds = (TextView)findViewById(R.id.listview_tcon_lvds_text);
        mTextTconLvds.setVisibility(View.GONE);

        //
        //  LED OPEN
        //
        ListView listViewLedOpen = (ListView)findViewById(R.id.listview_led_open);
        listViewLedOpen.addFooterView(listViewFooter);

        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail);
        listViewLedOpen.setAdapter( mAdapterLedOpen );

        mTextLedOpen = (TextView)findViewById(R.id.listview_led_open_text);
        mTextLedOpen.setVisibility(View.GONE);

        //  This is not Tested.
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                mRegFullWhite,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        Log.i(VD_DTAG, "Full White Backup Start.");
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        for( int i = 0; i < values.length; i++ ) {
                            mDatFullWhite[i] = (Integer)values[i];
                        }

                        HideProgress();
                        Log.i(VD_DTAG, "Full White Backup Done.");
                    }
                },
                null
        );

        //  This is not Tested.
        CheckBox checkFullWhite = (CheckBox)findViewById(R.id.checkFullWhite);
        checkFullWhite.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        mRegFullWhite,
                        !((CheckBox) view).isChecked() ? mDatFullWhite : mDefFullWhite,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                Log.i(VD_DTAG, "Full White Write Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                Log.i(VD_DTAG, "Full white Write Done.");
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        //  This is not Tested.
        CheckBox checkLedOpenDetect = (CheckBox)findViewById(R.id.checkLedOpenDetect);
        checkLedOpenDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x0100},
                        new int[]{((CheckBox) view).isChecked() ? 1 : 0},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                Log.i(VD_DTAG, "Led Open Detect Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                Log.i(VD_DTAG, "Led Open Detect Done.");
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        //  This is not Tested.
        final CheckBox checkLodRemoval1 = (CheckBox)findViewById(R.id.checkLodRemoval1);
        final CheckBox checkLodRemoval2 = (CheckBox)findViewById(R.id.checkLodRemoval2);

        checkLodRemoval1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_WRITE,
                        getApplicationContext(),
                        new int[]{0x011E, 0x00E6},
                        new int[]{checkLodRemoval1.isChecked() ? 1 : 0, checkLodRemoval2.isChecked() ? 1 : 0},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                Log.i(VD_DTAG, "LOD Removal Write Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                Log.i(VD_DTAG, "LOD Removal Write Done.");
                                HideProgress();
                            }
                        },
                        null
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_READ,
                        getApplicationContext(),
                        new int[]{0x011E, 0x00E6},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                Log.i(VD_DTAG, "LOD Removal Read Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                checkLodRemoval1.setChecked((Integer)values[0]==1);
                                checkLodRemoval2.setChecked((Integer)values[1]==1);
                                Log.i(VD_DTAG, "LOD Removal Read Done.");
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        checkLodRemoval2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        checkFullWhite.setVisibility(View.GONE);
        checkLedOpenDetect.setVisibility(View.GONE);
        checkLodRemoval1.setVisibility(View.GONE);
        checkLodRemoval2.setVisibility(View.GONE);

        //
        //  CABINET DOOR
        //
        ListView listViewCabinetDoor = (ListView)findViewById(R.id.listView_cabinet_door);
        listViewCabinetDoor.addFooterView(listViewFooter);

        mAdapterCabinetDoor = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewCabinetDoor.setAdapter( mAdapterCabinetDoor );

        mTextCabinetDoor = (TextView)findViewById(R.id.listview_cabinet_door_text);
        mTextCabinetDoor.setVisibility(View.GONE);

        //
        //  PERIPHERAL
        //
        ListView listViewPeripheral = (ListView)findViewById(R.id.listView_peripheral);
        listViewPeripheral.addFooterView(listViewFooter);

        mAdapterPeripheral = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewPeripheral.setAdapter( mAdapterPeripheral );

        final String[] strPeripheral = {
            "Secure AP",
            "P.FPGA",
            "IMB",
        };

        for(final String str : strPeripheral ) {
            mAdapterPeripheral.add( new StatusSimpleInfo( str ) );
            mAdapterPeripheral.notifyDataSetChanged();
        }

        //
        //  VERSION
        //
        ListView listViewVersion = (ListView)findViewById(R.id.listview_version);
        listViewVersion.addFooterView(listViewFooter);

        mAdapterVersion = new StatusDescribeAdapter(this, R.layout.listview_row_status_describe);
        listViewVersion.setAdapter( mAdapterVersion );

        //
        //  Initialize Tab
        //
        AddTabs();
        UpdateTconStatus();

        //
        //
        //

        Log.i(VD_DTAG, "--> Login Group : " + mCinemaInfo.GetUserGroup());

        if( mCinemaInfo.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            mTabHost.getTabWidget().getChildTabViewAt(0).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(0).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(2).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(2).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(4).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(4).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
        }
    }

    private void AddTabs() {
        mTabHost = (TabHost)findViewById( R.id.tabHost );
        mTabHost.setup();

        TabHost.TabSpec tabSpec0 = mTabHost.newTabSpec( "TAB0" );
        tabSpec0.setIndicator("TCON STATUS");
        tabSpec0.setContent(R.id.tabTcon);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("TCON LVDS");
        tabSpec1.setContent(R.id.tabTconLvds);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("LED OPEN");
        tabSpec2.setContent(R.id.tabLedOpen);

        TabHost.TabSpec tabSpec3 = mTabHost.newTabSpec( "TAB3" );
        tabSpec3.setIndicator("CABINET DOOR");
        tabSpec3.setContent(R.id.tabCabinetDoor);

        TabHost.TabSpec tabSpec4 = mTabHost.newTabSpec( "TAB4" );
        tabSpec4.setIndicator("PERIPHERAL");
        tabSpec4.setContent(R.id.tabPeripheral);

        TabHost.TabSpec tabSpec5 = mTabHost.newTabSpec( "TAB5" );
        tabSpec5.setIndicator("VERSION");
        tabSpec5.setContent(R.id.tabVersion);

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);
        mTabHost.addTab(tabSpec2);
        mTabHost.addTab(tabSpec3);
        mTabHost.addTab(tabSpec4);
        mTabHost.addTab(tabSpec5);

        mTabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        mTabHost.setCurrentTab(0);
    }

    private String mTabPrevious = "TAB0";
    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( mTabPrevious.equals("TAB3") ) ClearCabinetDoor();

            if( tabId.equals("TAB0") ) UpdateTconStatus();
            if( tabId.equals("TAB1") ) UpdateTconLvds();
            if( tabId.equals("TAB2") ) UpdateLedOpen();
            if( tabId.equals("TAB3") ) UpdateCabinetDoor();
            if( tabId.equals("TAB4") ) UpdatePeripheral();
            if( tabId.equals("TAB5") ) UpdateVersion();

            mTabPrevious = tabId;
        }
    };

    private void UpdateTconStatus() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_STATUS,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mTextTconStatus.setVisibility(View.GONE);
                        mAdapterTconStatus.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( mAdapterTconStatus.getCount() == 0 ) {
                            mTextTconStatus.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextTconStatus.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        if( (Integer)values[1] != CinemaInfo.RET_PASS ) {
                            mAdapterTconStatus.add( new StatusSimpleInfo(
                                    String.format( Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])),
                                    (Integer)values[1]
                            ));
                            mAdapterTconStatus.notifyDataSetChanged();
                        }
                    }
                }
        );
    }

    private void UpdateTconLvds() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_LVDS,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mTextTconLvds.setVisibility(View.GONE);
                        mAdapterTconLvds.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( mAdapterTconLvds.getCount() == 0 ) {
                            mTextTconLvds.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextTconLvds.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        if( (Integer)values[1] != CinemaInfo.RET_PASS ) {
                            mAdapterTconLvds.add( new StatusSimpleInfo(
                                    String.format( Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])),
                                    (Integer)values[1]
                            ));
                            mAdapterTconLvds.notifyDataSetChanged();
                        }
                    }
                }
        );
    }

    private void UpdateLedOpen() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_LED_OPEN_NUM,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mTextLedOpen.setVisibility(View.GONE);
                        mAdapterLedOpen.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if (mAdapterLedOpen.getCount() == 0) {
                            mTextLedOpen.setVisibility(View.VISIBLE);
                        } else {
                            mTextLedOpen.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        if( (Integer)values[1] != 0 ) {
                            mAdapterLedOpen.add( new StatusDetailInfo(
                                    String.format(Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])),
                                    mCabinet[(Integer)values[0]],
                                    0,
                                    (0 > (Integer)values[1]) ? "-" : String.valueOf(values[1])
                            ));
                            mAdapterLedOpen.notifyDataSetChanged();
                        }
                    }
                }
        );
    }

    private void UpdateCabinetDoor() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_CABINET_DOOR,
                getApplicationContext(),
                true,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mTextCabinetDoor.setVisibility(View.GONE);
                        mAdapterCabinetDoor.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( mAdapterCabinetDoor.getCount() == 0 ) {
                            mTextCabinetDoor.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextCabinetDoor.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        if( (Integer)values[1] != CinemaInfo.RET_PASS ) {
                            mAdapterCabinetDoor.add( new StatusSimpleInfo(
                                    String.format( Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])),
                                    (Integer)values[1]
                            ));
                            mAdapterCabinetDoor.notifyDataSetChanged();
                        }
                    }
                }
        );
    }

    private void ClearCabinetDoor() {
        ClearCabinetDoor( null );
    }

    private void ClearCabinetDoor(final CinemaTask.PostExecuteCallback postExecute) {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_CABINET_DOOR,
                getApplicationContext(),
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

                        if( null != postExecute )
                            postExecute.onPostExecute(values);
                    }
                },
                null
        );
    }

    private void UpdatePeripheral() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PERIPHERAL,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();

                        for( int i = 0; i < 3; i++ ) {
                            StatusSimpleInfo info = mAdapterPeripheral.getItem(i);
                            if( info != null ) {
                                info.SetStatus(CinemaInfo.RET_ERROR);
                            }
                        }
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

                        StatusSimpleInfo info = mAdapterPeripheral.getItem((Integer)values[0]);
                        if( info != null ) info.SetStatus((Integer)values[1]);
                        mAdapterPeripheral.notifyDataSetChanged();
                    }
                }
        );
    }

    private void UpdateVersion() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_VERSION,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                        mAdapterVersion.clear();
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
                        if( !(values[0] instanceof Integer) || !(values[1] instanceof String) )
                            return;

                        mAdapterVersion.add( new StatusDescribeInfo(
                                String.format( Locale.US, "Cabinet %02d", mCinemaInfo.GetCabinetNumber(mCabinet[(Integer)values[0]])),
                                (String)values[1]
                        ));
                        mAdapterVersion.notifyDataSetChanged();
                    }
                }
        );
    }
}
