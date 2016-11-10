package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DiagnosticsActivity extends AppCompatActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

    private TabHost mTabHost;

    private StatusSimpleAdapter mAdapterTcon;
    private StatusDetailAdapter mAdapterLedOpen;
    private StatusDetailAdapter mAdapterLedShort;
    private StatusSimpleAdapter mAdapterCabinetDoor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

        //
        // Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDiagnostics ));
        titleBar.SetTitle( "Cinema LED Display System - Diagnostics" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });

        //
        // Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutDiagnostics) );

        //
        //  TCON STATUS
        //
        int cabinetNum = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));

        ListView listViewTcon = (ListView)findViewById(R.id.listView_tcon);
        mAdapterTcon = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewTcon.setAdapter( mAdapterTcon );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterTcon.add( new StatusSimpleInfo("Cabinet " + String.valueOf(i + 1)) );
            mAdapterTcon.notifyDataSetChanged();
        }

        //
        //  LED OPEN
        //
        ListView listViewLedOpen = (ListView)findViewById(R.id.listview_led_open);
        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail);
        listViewLedOpen.setAdapter( mAdapterLedOpen );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterLedOpen.add( "Cabinet " + String.valueOf(i + 1) );
        }

        //
        //  LED SHORT
        //
        ListView listViewLedShort = (ListView)findViewById(R.id.listview_led_short);
        mAdapterLedShort = new StatusDetailAdapter(this, R.layout.listview_row_status_detail);
        listViewLedShort.setAdapter( mAdapterLedShort );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterLedShort.add( "Cabinet " + String.valueOf(i + 1) );
        }

        //
        //  CABINET DOOR
        //
        ListView listViewCabinetDoor = (ListView)findViewById(R.id.listView_cabinet_door);
        mAdapterCabinetDoor = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewCabinetDoor.setAdapter( mAdapterCabinetDoor );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterCabinetDoor.add( new StatusSimpleInfo("Cabinet " + String.valueOf(i + 1)) );
            mAdapterCabinetDoor.notifyDataSetChanged();
        }

        //
        //  Initialize Tab
        //
        AddTabs();
        UpdateTconStatus();

        mTabHost.getTabWidget().getChildTabViewAt(2).setEnabled(false);
        ((TextView)mTabHost.getTabWidget().getChildAt(2).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        mTabHost.getTabWidget().getChildTabViewAt(4).setEnabled(false);
        ((TextView)mTabHost.getTabWidget().getChildAt(4).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

        //
        //  Set System Information
        //
        CinemaInfo info = ((CinemaInfo)getApplicationContext());
        Log.i(VD_DTAG, "--> Login Group : " + info.GetUserGroup());

        if( info.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            mTabHost.getTabWidget().getChildTabViewAt(0).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(0).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(1).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(1).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);

            mTabHost.getTabWidget().getChildTabViewAt(3).setEnabled(false);
            ((TextView)mTabHost.getTabWidget().getChildAt(3).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
        }
    }

    private void AddTabs() {
        mTabHost = (TabHost)findViewById( R.id.tabHost );
        mTabHost.setup();

        TabHost.TabSpec tabSpec0 = mTabHost.newTabSpec( "TAB0" );
        tabSpec0.setIndicator("TCON");
        tabSpec0.setContent(R.id.tab_diagnostics_tcon);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("LED OPEN");
        tabSpec1.setContent(R.id.tab_diagnostics_led_open);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("LED SHORT");
        tabSpec2.setContent(R.id.tab_diagnostics_led_short);

        TabHost.TabSpec tabSpec3 = mTabHost.newTabSpec( "TAB3" );
        tabSpec3.setIndicator("CABINET DOOR");
        tabSpec3.setContent(R.id.tab_diagnostics_cabinet_door);

        TabHost.TabSpec tabSpec4 = mTabHost.newTabSpec( "TAB4" );
        tabSpec4.setIndicator("BATTERY");
        tabSpec4.setContent(R.id.tab_diagnostics_battery);

        TabHost.TabSpec tabSpec5 = mTabHost.newTabSpec( "TAB5" );
        tabSpec5.setIndicator("PERIPHERAL");
        tabSpec5.setContent(R.id.tab_diagnostics_peripheral);

        TabHost.TabSpec tabSpec6 = mTabHost.newTabSpec( "TAB6" );
        tabSpec6.setIndicator("VERSION");
        tabSpec6.setContent(R.id.tab_diagnostics_version);

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);
        mTabHost.addTab(tabSpec2);
        mTabHost.addTab(tabSpec3);
        mTabHost.addTab(tabSpec4);
        mTabHost.addTab(tabSpec5);
        mTabHost.addTab(tabSpec6);

        mTabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        mTabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateTconStatus();
            if( tabId.equals("TAB1") ) UpdateLedOpen();
            if( tabId.equals("TAB2") ) UpdateLedShort();
            if( tabId.equals("TAB3") ) UpdateCabinetDoor();
            if( tabId.equals("TAB4") ) UpdateBattery();
            if( tabId.equals("TAB5") ) UpdatePeripheral();
            if( tabId.equals("TAB6") ) UpdateVersion();
        }
    };

    private void UpdateTconStatus() {
        for( int i = 0; i < mAdapterTcon.getCount(); i++ ) {
            StatusSimpleInfo info = mAdapterTcon.getItem(i);

            byte[] result = NxCinemaCtrl.GetInstance().Send( 32 + i, NxCinemaCtrl.CMD_TCON_STATUS, null );
            if(result == null || result.length == 0)
                continue;

            info.SetStatus( (int)result[0] );
            mAdapterTcon.notifyDataSetChanged();
        }
    }

    private void UpdateLedOpen() {

    }

    private void UpdateLedShort() {

    }

    private void UpdateCabinetDoor() {

    }

    private void UpdateBattery() {

    }

    private void UpdatePeripheral() {

    }

    private void UpdateVersion() {

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
