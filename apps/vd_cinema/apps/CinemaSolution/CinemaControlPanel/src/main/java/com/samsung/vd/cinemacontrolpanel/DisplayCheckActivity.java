package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Chronometer;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayCheckActivity";

    private String[] mFuncName = {
            "Full Screen Color",
            "Gary Scale ( Horizontal )",
            "Dot Pattern / Checker Board",
            "Diagonal Pattern",
            "Cabinet ID",
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

    private String[] mDotPattern = {
    };

    private String[] mDiagonalPattern = {
            "Right Down ( ↘ )", "Right Up ( ↗ )",
    };

    private String[] mCabinetId = {
    };

    private String[][] mPatternName = {
            mFullScreenColor,
            mGrayScale,
            mDotPattern,
            mDiagonalPattern,
            mCabinetId,
    };

    private LayoutInflater  mInflater;
    private LinearLayout    mParentLayoutTestPattern;
    private LinearLayout[]  mLayoutTestPattern;

    @Override
    protected void onResume() {
        super.onResume();

        mLayoutTestPattern = new LinearLayout[mFuncName.length];
        for( int i = 0; i < mFuncName.length; i++ ) {
            mLayoutTestPattern[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_test_pattern, mParentLayoutTestPattern, false );
            AddViewTestPattern( i, mLayoutTestPattern, mFuncName, mPatternName[i] );
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Display Check" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });

        // Configuration StatusBar
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        AddTabs();

        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayoutTestPattern = (LinearLayout)findViewById(R.id.layoutTestPattern);
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Test Pattern");
        tabSpec1.setContent(R.id.tabTestPattern);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Led Accumulation time");
        tabSpec2.setContent(R.id.tabLedAccumulation);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);

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
    //  For Internal Toast Message
    //
    private static Toast mToast;

    private void ShowMessage( String strMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_SHORT);

        mToast.setText(strMsg);
        mToast.show();
    }

    private void AddViewTestPattern( int index, View[] childView, String[] strName, String[] spinnerItem  ) {
        mParentLayoutTestPattern.addView(childView[index]);

        final int funcIndex = index;
        final TextView textViewTestPattern = (TextView)childView[index].findViewById(R.id.textViewTestPattern);
        final Spinner spinnerTestPattern = (Spinner)childView[index].findViewById(R.id.spinnerTestPattern);

        textViewTestPattern.setText( strName[index] );

        ArrayAdapter<String> arrayAdapter;
        if(0 == spinnerItem.length) {
            String[] strDummy = { "" };
            arrayAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strDummy);
            spinnerTestPattern.setEnabled(false);
        }
        else {
            arrayAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, spinnerItem);
        }

        spinnerTestPattern.setAdapter( arrayAdapter );

        Button btnTestPattern = (Button)childView[index].findViewById(R.id.btnTestPattern);
        btnTestPattern.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                RunTestPattern( funcIndex, spinnerTestPattern.getSelectedItemPosition() );
            }
        });
    }

    private void RunTestPattern( int funcIndex, int patternIndex ) {
        Log.i(VD_DTAG, String.format( "funcIndex(%d), patternIndex(%d)", funcIndex, patternIndex) );

        byte[] data = { (byte)funcIndex, (byte)patternIndex };
        NxCinemaCtrl.GetInstance().Send( 0x09, NxCinemaCtrl.CMD_TCON_PATTERN, data );

        String strLog;
        if( mPatternName[funcIndex].length == 0 ) {
            strLog = String.format( "Run pattern test. ( %s )", mFuncName[funcIndex] );
        }
        else {
            strLog = String.format( "Run pattern test. ( %s / %s )", mFuncName[funcIndex], mPatternName[funcIndex][patternIndex] );
        }

        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
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