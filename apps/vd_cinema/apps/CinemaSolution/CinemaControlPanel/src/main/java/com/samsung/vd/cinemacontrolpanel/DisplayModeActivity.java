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
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TabHost;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private final String[] INPUT_SOURCE = {
        "DCP (24Hz, 4K)",
        "HD-SDI (120Hz, 4K)",
        "HDMI (60Hz, 4K)",
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_mode);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDisplayMode ));
        titleBar.SetTitle( "Cinema LED Display System Display Mode" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), TopActivity.class);
                startActivity(intent);
            }
        });

        // Configuration StatusBar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutDisplayMode) );

        ArrayAdapter<String> adapterInputSource = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, INPUT_SOURCE);
        Spinner spinnerInputSource = (Spinner)findViewById(R.id.spinnerInputSource);
        spinnerInputSource.setAdapter(adapterInputSource );

        AddTabs();

        //
        //  Spinner Time
        //
        String[] strYear = new String[100];
        String[] strDay = new String[31];
        String[] strHour = new String[24];
        String[] strMin = new String[60];

        String[] strMonth = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        for( int i = 0; i < strYear.length; i++ )   strYear[i] = String.valueOf(2000 + i);
        for( int i = 0; i < strDay.length; i++ )    strDay[i] = String.valueOf(1 + i);
        for( int i = 0; i < strHour.length; i++ )   strHour[i] = String.valueOf(i);
        for( int i = 0; i < strMin.length; i++ )    strMin[i] = String.valueOf(i);

        ArrayAdapter<String> adapterYear = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strYear);
        Spinner spinnerYear = (Spinner)findViewById(R.id.spinnerYear);
        spinnerYear.setAdapter(adapterYear);

        ArrayAdapter<String> adapterMonth = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strMonth);
        Spinner spinnerMonth = (Spinner)findViewById(R.id.spinnerMonth);
        spinnerMonth.setAdapter(adapterMonth);

        ArrayAdapter<String> adapterDay = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strDay);
        Spinner spinnerDay = (Spinner)findViewById(R.id.spinnerDay);
        spinnerDay.setAdapter(adapterDay);

        ArrayAdapter<String> adapterHour = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strHour);
        Spinner spinnerHour = (Spinner)findViewById(R.id.spinnerHour);
        spinnerHour.setAdapter(adapterHour);

        ArrayAdapter<String> adapterMin = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strMin);
        Spinner spinnerMin = (Spinner)findViewById(R.id.spinnerMinute);
        spinnerMin.setAdapter(adapterMin);

        //
        //  Spinner Suspend Time
        //
        String[] strSuspendTime = {
            "Disable", "1 min", "2 min", "5 min", "10 min", "20 min", "30 min"
        };

        ArrayAdapter<String> adapterSuspendTime = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, strSuspendTime);
        Spinner spinnerSuspendTime = (Spinner)findViewById(R.id.spinnerSuspendTime);
        spinnerSuspendTime.setAdapter(adapterSuspendTime);
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Brightness");
        tabSpec1.setContent(R.id.tab_display_mode_brightness);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Input Source");
        tabSpec2.setContent(R.id.tab_display_mode_input);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Set up");
        tabSpec3.setContent(R.id.tab_display_mode_setup);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
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