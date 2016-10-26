package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdLoginDatabase;
import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.ArrayList;

/**
 * Created by doriya on 8/16/16.
 */
public class DiagnosticsActivity extends AppCompatActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

    private ListView mListViewTcon;
    private ListView mListViewLedOpen;
    private ListView mListViewLedShort;
    private ListView mListViewCabinetDoor;

    private StatusSimpleAdapter mAdapterTcon;
    private StatusDetailAdapter mAdapterLedOpen;
    private StatusDetailAdapter mAdapterLedShort;
    private StatusSimpleAdapter mAdapterCabinetDoor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDiagnostics ));
        titleBar.SetTitle( "Cinema LED Display System Diagnostics" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), TopActivity.class);
                startActivity(intent);
            }
        });

        // Configuration StatusBar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutDiagnostics) );

        AddTabs();

        //
        //  TCON
        //
        mListViewTcon  = (ListView)findViewById(R.id.listView_tcon);
        mAdapterTcon = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        mListViewTcon.setAdapter( mAdapterTcon);

        for(int i = 0; i < 69; i++ )
        {
            mAdapterTcon.add( "Cabinet " + String.valueOf(i + 1) );
        }

        //
        //  LED OPEN
        //
        mListViewLedOpen = (ListView)findViewById(R.id.listview_led_open);
        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail);
        mListViewLedOpen.setAdapter( mAdapterLedOpen );

        for(int i = 0; i < 69; i++ )
        {
            mAdapterLedOpen.add( "Cabinet " + String.valueOf(i + 1) );
        }

        //
        //  LED SHORT
        //
        mListViewLedShort = (ListView)findViewById(R.id.listview_led_short);
        mAdapterLedShort = new StatusDetailAdapter(this, R.layout.listview_row_status_detail);
        mListViewLedShort.setAdapter( mAdapterLedShort );

        for(int i = 0; i < 69; i++ )
        {
            mAdapterLedShort.add( "Cabinet " + String.valueOf(i + 1) );
        }

        //
        //  CABINET DOOR
        //
        mListViewCabinetDoor = (ListView)findViewById(R.id.listView_cabinet_door);
        mAdapterCabinetDoor = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        mListViewCabinetDoor.setAdapter( mAdapterCabinetDoor );

        for(int i = 0; i < 69; i++ )
        {
            mAdapterCabinetDoor.add( "Cabinet " + String.valueOf(i + 1) );
        }
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost)findViewById( R.id.tabHost );
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("TCON");
        tabSpec1.setContent(R.id.tab_diagnostics_tcon);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("LED OPEN");
        tabSpec2.setContent(R.id.tab_diagnostics_led_open);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec( "TAB3" );
        tabSpec3.setIndicator("LED SHORT");
        tabSpec3.setContent(R.id.tab_diagnostics_led_short);

        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec( "TAB4" );
        tabSpec4.setIndicator("CABINET DOOR");
        tabSpec4.setContent(R.id.tab_diagnostics_cabinet_door);

        TabHost.TabSpec tabSpec5 = tabHost.newTabSpec( "TAB5" );
        tabSpec5.setIndicator("BATTERY");
        tabSpec5.setContent(R.id.tab_diagnostics_battery);

        TabHost.TabSpec tabSpec6 = tabHost.newTabSpec( "TAB6" );
        tabSpec6.setIndicator("PERIPHERAL");
        tabSpec6.setContent(R.id.tab_diagnostics_peripheral);

        TabHost.TabSpec tabSpec7 = tabHost.newTabSpec( "TAB7" );
        tabSpec7.setIndicator("VERSION");
        tabSpec7.setContent(R.id.tab_diagnostics_version);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);
        tabHost.addTab(tabSpec4);
        tabHost.addTab(tabSpec5);
        tabHost.addTab(tabSpec6);
        tabHost.addTab(tabSpec7);

        tabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            Log.i(VD_DTAG, "Tab ID : " + tabId);
        }
    };

    //
    //  For ScreenSaver
    //
    private ScreenSaverService mService = null;
    private boolean mServiceRun = false;

    @Override
    protected void onStart() {
        super.onStart();

        Intent intent = new Intent(this, ScreenSaverService.class);
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
        mService.RefreshScreenSaver();

        return super.dispatchTouchEvent(ev);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            ScreenSaverService.LocalBinder binder = (ScreenSaverService.LocalBinder)service;
            mService = binder.GetService();
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };

}
