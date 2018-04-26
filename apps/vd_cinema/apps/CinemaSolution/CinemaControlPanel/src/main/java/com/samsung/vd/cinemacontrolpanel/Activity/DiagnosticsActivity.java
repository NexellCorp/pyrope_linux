package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.LedPosAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDetailAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusSimpleAdapter;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusDescribeInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusDetailInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusSimpleInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.AccountPreference;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.LedPosDialog;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeAdapter;

import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsCabinetDoor;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsFullWhite;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsFullWhiteBackup;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsLedOpenDetect;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsLedOpenNum;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsPeriPheral;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsTconLvds;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsTconStatus;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_DiagnosticsVersion;

/**
 * Created by doriya on 8/16/16.
 */
public class DiagnosticsActivity extends BaseActivity {
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

    @Override
    protected void onStart() {
        super.onStart();
        //Async
        NXAsync.getInstance().Execute(CMD_DiagnosticsFullWhiteBackup , baseAsyncCallback);
        UpdateTconStatus();
    }


    View.OnClickListener onRefreshClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            bRefreshFlag = true;
            int tabIdx = mTabHost.getCurrentTab();
            NXAsync.getInstance().Execute(CMD_DiagnosticsFullWhiteBackup , baseAsyncCallback);
            if( 0 == tabIdx ) {
                UpdateTconStatus();
            }
            else if( 1 == tabIdx ) {
                UpdateTconLvds();
            }
            else if( 2 == tabIdx ) {
                UpdateLedOpen();
            }
            else if( 3 == tabIdx ) {
                UpdateCabinetDoor();
            }
            else if( 4 == tabIdx ) {
                UpdatePeripheral();
            }
            else if( 5 == tabIdx ) {
                UpdateVersion();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

        contentParent = (RelativeLayout)findViewById(R.id.contentParentDiagnostic);
        bI2CFailFlag = false;
        bRefreshFlag = false;
        setOnRefreshClickListener(onRefreshClickListener);

        // Set Title Bar and Status Bar
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Diagnostics" , new Intent(getApplicationContext(), TopActivity.class) );

        //  Cinema System Information
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //  TCON STATUS
        ListView listViewTconStatus = (ListView)findViewById(R.id.listView_tcon_status);
        listViewTconStatus.addFooterView(listViewFooter);

        mAdapterTconStatus = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewTconStatus.setAdapter( mAdapterTconStatus );

        mTextTconStatus = (TextView)findViewById(R.id.listview_tcon_status_text);
        mTextTconStatus.setVisibility(View.GONE);

        //  TCON LVDS
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

        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail , asyncCallbackLedPos);
        listViewLedOpen.setAdapter( mAdapterLedOpen );

        mTextLedOpen = (TextView)findViewById(R.id.listview_led_open_text);
        mTextLedOpen.setVisibility(View.GONE);

//        new AsyncTaskFullWhiteBackup().execute();
//        NXAsync.getInstance().Execute(CMD_DiagnosticsFullWhiteBackup , baseAsyncCallback);    >>> onStart

        CheckBox checkFullWhite = (CheckBox)findViewById(R.id.checkFullWhite);
        checkFullWhite.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.i(VD_DTAG, String.format("Full White Value : %s", !((CheckBox)view).isChecked() ? "Restore" : "Full White"));


                NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setFullWhiteRestore( !((CheckBox)view).isChecked() );
//                new AsyncTaskFullWhite( !((CheckBox)view).isChecked() ).execute();
                NXAsync.getInstance().Execute(CMD_DiagnosticsFullWhite , baseAsyncCallback);
            }
        });

        CheckBox checkLedOpenDetect = (CheckBox)findViewById(R.id.checkLedOpenDetect);
        checkLedOpenDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setLedOpenDetect(((CheckBox)view).isChecked());
//                new AsyncTaskLedOpenDetect(((CheckBox)view).isChecked()).execute();
                NXAsync.getInstance().Execute(CMD_DiagnosticsLedOpenDetect , baseAsyncCallback);
            }
        });

        CheckBox checkLodRemoval1 = (CheckBox)findViewById(R.id.checkLodRemoval1);
        CheckBox checkLodRemoval2 = (CheckBox)findViewById(R.id.checkLodRemoval2);

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
        }
        mAdapterPeripheral.notifyDataSetChanged();

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
        //UpdateTconStatus();    >>> onStart

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
    } //onCreate

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
        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setTCONStatusAdapter(mAdapterTconStatus);
//        new AsyncTaskTconStatus(mAdapterTconStatus).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsTconStatus , asyncCallbackTconStatus);
    }

    private void UpdateTconLvds() {
        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setTconLvdsAdapter(mAdapterTconLvds);
//        new AsyncTaskTconLvds(mAdapterTconLvds).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsTconLvds , asyncCallbackTconLvds);
    }

    private void UpdateLedOpen() {
//        new AsyncTaskFullWhiteBackup().execute();
//        NXAsync.getInstance().Execute(CMD_DiagnosticsFullWhiteBackup , baseAsyncCallback);

        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setLedOpenNumAdapter(mAdapterLedOpen);
//        new AsyncTaskLedOpenNum(mAdapterLedOpen).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsLedOpenNum , asyncCallbackLedOpenNum);
    }

    private void UpdateCabinetDoor() {
        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setCabinetDoorAdapter(mAdapterCabinetDoor);
//        new AsyncTaskCabinetDoor(mAdapterCabinetDoor).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsCabinetDoor , asyncCallbackCabinetDoor);
    }

    private void UpdatePeripheral() {
        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setPeripheralAdapter(mAdapterPeripheral);
//        new AsyncTaskPeripheral(mAdapterPeripheral).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsPeriPheral , asyncCallbackPeriPheral);
    }

    private void UpdateVersion() {
        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().setVersionAdapter(mAdapterVersion);
//        new AsyncTaskVersion(mAdapterVersion).execute();
        NXAsync.getInstance().Execute(CMD_DiagnosticsVersion , asyncCallbackVersion);
    }

    //
    // async callbacks
    //

    NXAsync.AsyncCallback asyncCallbackTconStatus = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);

                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusAdapter().clear();

                    mTextTconStatus.setVisibility(View.GONE);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusAdapter().add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCON() ), NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusResult() ));
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusAdapter().notifyDataSetChanged();

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusResult() == StatusSimpleInfo.ERROR ) {
                        String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Error.", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCON() );
                        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
                    }
                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusResult() == StatusSimpleInfo.FAIL ) {
                        String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Failed.", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCON() );
                        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
                    }
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONStatusAdapter().getCount() == 0 ) {
                        mTextTconStatus.setVisibility(View.VISIBLE);
                    }
                    else {
                        mTextTconStatus.setVisibility(View.GONE);
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackTconStatus

    NXAsync.AsyncCallback asyncCallbackTconLvds = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTconLvdsAdapter().clear();
                    mTextTconLvds.setVisibility(View.GONE);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTconLvdsAdapter().add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvds() ), NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvdsResult() ));
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTconLvdsAdapter().notifyDataSetChanged();

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvdsResult() == StatusSimpleInfo.ERROR ) {
                        String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Error.", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvds() );
                        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
                    }
                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvdsResult() == StatusSimpleInfo.FAIL ) {
                        String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Failed.", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTCONLvds() );
                        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
                    }
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getTconLvdsAdapter().getCount() == 0 ) {
                        mTextTconLvds.setVisibility(View.VISIBLE);
                    }
                    else {
                        mTextTconLvds.setVisibility(View.GONE);
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackTconLvds
    
    NXAsync.AsyncCallback asyncCallbackLedOpenNum = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumAdapter().clear();
                    mTextLedOpen.setVisibility(View.GONE);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    //
                    //  values[0] : Cabinet ID, values[1] : Slave Address, values[2] : Result
                    //
                    int LedOpenNumCabinetID = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumCabinetID();
                    int LedOpenNumSlaveAddr = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumSlaveAddr();
                    int LedOpenNumResult = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumResult();
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumAdapter().add(new StatusDetailInfo(String.format(Locale.US, "Cabinet %02d", LedOpenNumSlaveAddr), LedOpenNumCabinetID, 0, (0 > LedOpenNumResult) ? "-" : String.valueOf(LedOpenNumResult)));
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumAdapter().notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLedOpenNumAdapter().getCount() == 0 ) {
                        mTextLedOpen.setVisibility(View.VISIBLE);
                    }
                    else {
                        mTextLedOpen.setVisibility(View.GONE);
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackLedOpenNum

    NXAsync.AsyncCallback asyncCallbackLodRemovalRead = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLodRemovalRead1().setChecked(NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().isLodRemovalReadFlag1());
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getLodRemovalRead2().setChecked(NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().isLodRemovalReadFlag2());
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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
    }; //asyncCallbackLodRemovalRead

    NXAsync.AsyncCallback asyncCallbackCabinetDoor = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorAdapter().clear();
                    mTextCabinetDoor.setVisibility(View.GONE);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorAdapter().add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorCabinet() ), NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorResult() ));
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorAdapter().notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getCabinetDoorAdapter().getCount() == 0 ) {
                        mTextCabinetDoor.setVisibility(View.VISIBLE);
                    }
                    else {
                        mTextCabinetDoor.setVisibility(View.GONE);
                    }

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackCabinetDoor

    NXAsync.AsyncCallback asyncCallbackPeriPheral = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getPeripheralAdapter().notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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
    }; //asyncCallbackPeriPheral

    NXAsync.AsyncCallback asyncCallbackVersion = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersionAdapter().clear();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onProgress() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    int cabinet = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersionCabinet();
                    int version1 = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersion1();
                    int version2 = NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersion2();

                    if( NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersion1() == -1 || NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersion2() == -1 ) {
                        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersionAdapter().add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", cabinet ), "Unknown Version") );
                    }
                    else {
                        NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersionAdapter().add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", cabinet ), String.format( Locale.US, "%05d-%05d", version1, version2 )));                    }

                    NXAsync.getInstance().getDiagnosticsTconStatusAsyncParam().getVersionAdapter().notifyDataSetChanged();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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
    }; //asyncCallbackVersion

    NXAsync.AsyncCallback asyncCallbackLedPos = new NXAsync.AsyncCallback(){
        @Override
        public void onPreExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( DiagnosticsActivity.this);
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            DiagnosticsActivity.this.runOnUiThread(new Runnable() {
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
                    LedPosAdapter mAdapter = NXAsync.getInstance().getStatusDetailAdapterAsyncParam().getmAdapter();

                    new LedPosDialog(DiagnosticsActivity.this, mAdapter).show();

                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackLedPos

} //DiagnosticsActivity
