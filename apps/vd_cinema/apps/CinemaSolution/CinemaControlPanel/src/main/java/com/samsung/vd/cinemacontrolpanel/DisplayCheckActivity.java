package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayCheckActivity";

    private String[] mFuncName = {
            "DCI",
            "Color Bar",
            "Full Screen Color",
            "Gary Scale ( Horizontal )",
            "Dot Pattern / Checker Board",
            "Diagonal Pattern",
            "Cabinet ID",
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
    };

    private SelectRunAdapter mAdapterTestPattern;
    private StatusDescribeExpandableAdapter mAdapterAccumulation;

    private byte[]  mCabinet;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

        //
        // Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Display Check" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                StopTestPattern();

                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                StopTestPattern();

                mService.TurnOff();
            }
        });

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
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();

        //
        //  TEST PATTERN
        //
        ListView listViewTestPattern = (ListView)findViewById(R.id.listview_test_pattern);
        listViewTestPattern.addFooterView( listViewFooter );

        mAdapterTestPattern = new SelectRunAdapter(this, R.layout.listview_row_select_run);
        listViewTestPattern.setAdapter( mAdapterTestPattern );

        //
        //
        //
        for(int i = 0; i < mFuncName.length; i++ ) {
            boolean isToggle = mFuncName[i].equals("Cabinet ID");
            mAdapterTestPattern.add( new SelectRunInfo(mFuncName[i], mPatternName[i], isToggle, new SelectRunAdapter.OnClickListener() {
                @Override
                public void onClickListener(int index, int spinnerIndex, boolean status ) {
                    RunTestPattern( index, spinnerIndex, status );
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

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);

        tabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateTestPattern();
            if( tabId.equals("TAB1") ) UpdateAccumulation();
        }
    };

    private void UpdateTestPattern() {

    }

    private void UpdateAccumulation() {
        StopTestPattern();

        new AsyncTaskAccumulation(mAdapterAccumulation).execute();
    }

    private void RunTestPattern( int funcIndex, int patternIndex, boolean status ) {
        Log.i(VD_DTAG, String.format( "funcIndex(%d), patternIndex(%d), status(%s)", funcIndex, patternIndex, String.valueOf(status)) );

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] data = { (byte)funcIndex, (byte)patternIndex };

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        if( bValidPort0 ) ctrl.Send( 0x09, status ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data );
        if( bValidPort1 ) ctrl.Send( 0x89, status ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data );

        String strLog;
        if( mPatternName[funcIndex].length == 0 ) {
            strLog = String.format( "%s pattern test. ( %s )", status ? "Run" : "Stop", mFuncName[funcIndex] );
        }
        else {
            strLog = String.format( "%s pattern test. ( %s / %s )", status ? "Run" : "Stop", mFuncName[funcIndex], mPatternName[funcIndex][patternIndex] );
        }

        ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
    }

    private void StopTestPattern() {
        for( int i = 0; i < mAdapterTestPattern.getCount(); i++ ) {
            mAdapterTestPattern.reset(i);
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        for( int i = 0; i < mFuncName.length; i++ ) {
            byte[] data = { (byte)i, (byte)0x00 };
            if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data );
            if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data );
        }
    }

    private class AsyncTaskAccumulation extends AsyncTask<Void, Void, Void> {
        private StatusDescribeExpandableAdapter mAdapter;

        public AsyncTaskAccumulation( StatusDescribeExpandableAdapter adapter) {
            mAdapter = adapter;
            mAdapter.clear();
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mCabinet == null )
                return null;

            for( int i = 0; i < mCabinet.length; i++ ) {
                mAdapter.addGroup( new StatusDescribeExpandableInfo(String.format( Locale.US, "Cabinet %02d", (mCabinet[i] & 0x7F) - CinemaInfo.OFFSET_TCON )) );

                for( int j = 0; j < 24; j++ ) {
                    byte[] module = ctrl.IntToByteArray( j, NxCinemaCtrl.FORMAT_INT8 );
                    byte[] inData = new byte[32];
                    inData[0] = module[0];

                    byte[] result = ctrl.Send( mCabinet[i], NxCinemaCtrl.CMD_TCON_ACCUMULATE_TIME, inData );
                    if( result == null || result.length == 0 ) {
                        mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), String.valueOf("Error")) );
                        continue;
                    }
                    mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), new String(result) ));
                }
	            publishProgress();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show(DisplayCheckActivity.this);
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
        }
    }

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