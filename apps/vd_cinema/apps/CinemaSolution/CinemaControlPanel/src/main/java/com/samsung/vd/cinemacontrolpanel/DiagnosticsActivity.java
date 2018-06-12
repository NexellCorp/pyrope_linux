package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DiagnosticsActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

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

    private byte[]  mCabinet;

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
        // Configuration TitleBar
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
                Launch(v.getContext(), TopActivity.class);
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
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

        //
        //  Cinema System Information
        //
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();

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
                    public void onPreExecute() {
                        Log.i(VD_DTAG, "Full White Backup Start.");
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {

                    }

                    @Override
                    public void onPostExecute(int[] values) {
                        mDatFullWhite = values;
                        HideProgress();
                        Log.i(VD_DTAG, "Full White Backup Done.");
                    }
                }

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
                            public void onPreExecute() {
                                Log.i(VD_DTAG, "Full White Write Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                Log.i(VD_DTAG, "Full white Write Done.");
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
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
                            public void onPreExecute() {
                                Log.i(VD_DTAG, "Led Open Detect Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                Log.i(VD_DTAG, "Led Open Detect Done.");
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
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
                            public void onPreExecute() {
                                Log.i(VD_DTAG, "LOD Removal Write Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {
                                Log.i(VD_DTAG, "LOD Removal Write Done.");
                                HideProgress();
                            }

                            @Override
                            public void onPostExecute(int[] values) {

                            }
                        }
                );

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_TCON_REG_READ,
                        getApplicationContext(),
                        new int[]{0x011E, 0x00E6},
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute() {
                                Log.i(VD_DTAG, "LOD Removal Read Start.");
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute() {

                            }

                            @Override
                            public void onPostExecute(int[] values) {
                                checkLodRemoval1.setChecked(values[0]==1);
                                checkLodRemoval2.setChecked(values[1]==1);
                                Log.i(VD_DTAG, "LOD Removal Read Done.");
                                HideProgress();
                            }
                        }
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
        //  System Information
        //
        CinemaInfo info = ((CinemaInfo)getApplicationContext());
        Log.i(VD_DTAG, "--> Login Group : " + info.GetUserGroup());

        if( info.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            mTabHost.getTabWidget().getChildTabViewAt(0).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(0).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(2).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(2).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(4).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(4).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
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

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateTconStatus();
            if( tabId.equals("TAB1") ) UpdateTconLvds();
            if( tabId.equals("TAB2") ) UpdateLedOpen();
            if( tabId.equals("TAB3") ) UpdateCabinetDoor();
            if( tabId.equals("TAB4") ) UpdatePeripheral();
            if( tabId.equals("TAB5") ) UpdateVersion();
        }
    };

    private void UpdateTconStatus() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_STATUS,
                getApplicationContext(),
                mAdapterTconStatus,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        mTextTconStatus.setVisibility(View.GONE);
                        mAdapterTconStatus.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        if( mAdapterTconStatus.getCount() == 0 ) {
                            mTextTconStatus.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextTconStatus.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );
    }

    private void UpdateTconLvds() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_LVDS,
                getApplicationContext(),
                mAdapterTconLvds,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        mTextTconLvds.setVisibility(View.GONE);
                        mAdapterTconLvds.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        if( mAdapterTconLvds.getCount() == 0 ) {
                            mTextTconLvds.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextTconLvds.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );
    }

    private void UpdateLedOpen() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_LED_OPEN_NUM,
                getApplicationContext(),
                mAdapterLedOpen,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        mTextLedOpen.setVisibility(View.GONE);
                        mAdapterLedOpen.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        if( mAdapterLedOpen.getCount() == 0 ) {
                            mTextLedOpen.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextLedOpen.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );
    }

    private void UpdateCabinetDoor() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_CABINET_DOOR,
                getApplicationContext(),
                mAdapterCabinetDoor,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        mTextCabinetDoor.setVisibility(View.GONE);
                        mAdapterCabinetDoor.clear();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute() {
                        if( mAdapterCabinetDoor.getCount() == 0 ) {
                            mTextCabinetDoor.setVisibility(View.VISIBLE);
                        }
                        else {
                            mTextCabinetDoor.setVisibility(View.GONE);
                        }
                        HideProgress();
                    }

                    @Override
                    public void onPostExecute(int[] values) {

                    }
                }
        );
    }

    private void UpdatePeripheral() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PERIPHERAL,
                getApplicationContext(),
                mAdapterPeripheral,
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

    private void UpdateVersion() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_VERSION,
                getApplicationContext(),
                mAdapterVersion,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute() {
                        ShowProgress();
                        mAdapterVersion.clear();
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
}
