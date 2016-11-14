package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.TabHost;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.lang.reflect.Array;

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
    private StatusSimpleAdapter mAdapterPeripheral;
    private StatusDescribeAdapter mAdapterVersion;

    private AsyncTaskStatus     mAsyncTaskStatus;
    private AsyncTaskLedOpenNum mAsyncTaskLedOpenNum;
    private AsyncTaskCabinetDoor mAsyncTaskCabinetDoor;
    private AsyncTaskPeripheral mAsyncTaskPeripheral;
    private AsyncTaskVersion    mAsyncTaskVersion;

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
        //  Cinema System Information
        //
        int cabinetNum = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));

        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //
        //
        //  TCON STATUS
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
        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail, LedPosDialog.TYPE_LED_OPEN_POS);
        listViewLedOpen.setAdapter( mAdapterLedOpen );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterLedOpen.add( new StatusDetailInfo("Cabinet " + String.valueOf(i + 1)) );
            mAdapterLedOpen.notifyDataSetChanged();
        }

        //
        //  LED SHORT
        //
        ListView listViewLedShort = (ListView)findViewById(R.id.listview_led_short);
        mAdapterLedShort = new StatusDetailAdapter(this, R.layout.listview_row_status_detail, LedPosDialog.TYPE_LED_SHORT_POS);
        listViewLedShort.setAdapter( mAdapterLedShort );

        for(int i = 0; i < cabinetNum; i++ )
        {
            mAdapterLedShort.add( new StatusDetailInfo("Cabinet " + String.valueOf(i + 1)) );
            mAdapterLedShort.notifyDataSetChanged();
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

        final String[] strVersion = {
            "TCON Version",
            "TCON LED Version",
        };

        for( final String str : strVersion ) {
            mAdapterVersion.add( new StatusDescribeInfo( str, "xx.xx.xx" ) );
            mAdapterVersion.notifyDataSetChanged();
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
        //  System Information
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

    @Override
    protected void onDestroy() {
        StopTask();
        super.onDestroy();
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
        StopTask();

        mAsyncTaskStatus = new AsyncTaskStatus( mAdapterTcon );
        mAsyncTaskStatus.execute();
    }


    private void UpdateLedOpen() {
        StopTask();

        mAsyncTaskLedOpenNum = new AsyncTaskLedOpenNum( mAdapterLedOpen );
        mAsyncTaskLedOpenNum.execute();
    }

    private void UpdateLedShort() {
        Log.i(VD_DTAG, "Not Implementation.");
    }

    private void UpdateCabinetDoor() {
        StopTask();

        mAsyncTaskCabinetDoor = new AsyncTaskCabinetDoor( mAdapterCabinetDoor );
        mAsyncTaskCabinetDoor.execute();
    }

    private void UpdateBattery() {
        Log.i(VD_DTAG, "Not Implementation.");
    }

    private void UpdatePeripheral() {
        StopTask();

        mAsyncTaskPeripheral = new AsyncTaskPeripheral( mAdapterPeripheral );
        mAsyncTaskPeripheral.execute();
    }

    private void UpdateVersion() {
        StopTask();

        mAsyncTaskVersion = new AsyncTaskVersion( mAdapterVersion );
        mAsyncTaskVersion.execute();
    }

    //
    //
    //
    private class AsyncTaskStatus extends AsyncTask<Void, Void, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskStatus( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                if (isCancelled()) {
                    return null;
                }

                byte[] result = NxCinemaCtrl.GetInstance().Send(32 + i, NxCinemaCtrl.CMD_TCON_STATUS, null);
                if (result == null || result.length == 0)
                    continue;

                StatusSimpleInfo info = mAdapter.getItem(i);
                info.SetStatus((int)result[0]);

                publishProgress();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private class AsyncTaskLedOpenNum extends AsyncTask<Void, Void, Void> {
        private StatusDetailAdapter mAdapter;

        public AsyncTaskLedOpenNum( StatusDetailAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MODE_LOD, null);

            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                if( isCancelled() ) {
                    return null;
                }

                //
                //  Implementation Led Open Num!!
                //
                byte[] result = ctrl.Send(32 + i, NxCinemaCtrl.CMD_TCON_OPEN_NUM, null);
                if (result == null || result.length == 0)
                    continue;

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                StatusDetailInfo info = mAdapter.getItem(i);
                info.SetDescription("LED OPEN TEST OK!!!");

                publishProgress();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private class AsyncTaskCabinetDoor extends AsyncTask<Void, Void, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskCabinetDoor( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                if (isCancelled()) {
                    return null;
                }

                //
                //  Implementation Cabinet Door Check!!
                //
//                byte[] result = NxCinemaCtrl.GetInstance().Send(32 + i, NxCinemaCtrl.CMD_TCON_STATUS, null);
//                if (result == null || result.length == 0)
//                    continue;
//
//                StatusSimpleInfo info = mAdapter.getItem(i);
//                info.SetStatus((int)result[0]);

                publishProgress();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private class AsyncTaskPeripheral extends AsyncTask<Void, Void, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskPeripheral( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            //
            //  Security AP
            //
            if (isCancelled()) return null;

            {
                StatusSimpleInfo info = mAdapter.getItem(0);
                info.SetStatus(0);

                ((CinemaInfo)getApplicationContext()).SetSecureAlive("false");
                for( int i = 0; i < 64; i++ ) {
                    if (isCancelled()) return null;

                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    if( ((CinemaInfo)getApplicationContext()).GetSecureAlive().equals("true") ) {
                        info.SetStatus(1);
                        break;
                    }
                }

                publishProgress();
            }

            //
            //  P.FPGA
            //
            if (isCancelled()) return null;

            {

            }

            //
            //  IMB
            //
            if (isCancelled()) return null;

            {
                NetworkTools tools = new NetworkTools();
                StatusSimpleInfo info = mAdapter.getItem(2);

                try {
                    if( tools.GetEthLink().equals("true") && tools.Ping("192.168.10.1") ) {
                        info.SetStatus(1);
                    }
                    else
                    {
                        info.SetStatus(0);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            publishProgress();
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private class AsyncTaskVersion extends AsyncTask<Void, Void, Void> {
        private StatusDescribeAdapter mAdapter;

        public AsyncTaskVersion( StatusDescribeAdapter adapter) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            //
            //  TCON Version
            //
            if(isCancelled()) return null;

            {

            }

            publishProgress();

            //
            //  TCON Led Version
            //
            if(isCancelled()) return null;

            {

            }

            publishProgress();
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private void StopTask() {
        if( mAsyncTaskStatus != null && mAsyncTaskStatus.getStatus() == AsyncTask.Status.RUNNING ) {
            mAsyncTaskStatus.cancel( true );
        }

        if( mAsyncTaskLedOpenNum != null && mAsyncTaskLedOpenNum.getStatus() == AsyncTask.Status.RUNNING ) {
            mAsyncTaskLedOpenNum.cancel( true );

            byte[] result = NxCinemaCtrl.GetInstance().Send(0x09, NxCinemaCtrl.CMD_TCON_MODE_NORMAL, null);
            if( result == null || result.length == 0 || (int)result[0] == 0xFF) {
                Log.i(VD_DTAG, "Error, CMD_TCON_MODE_NORMAL.");
            }
        }

        if( mAsyncTaskCabinetDoor != null && mAsyncTaskCabinetDoor.getStatus() == AsyncTask.Status.RUNNING ) {
            mAsyncTaskCabinetDoor.cancel( true );
        }

        if( mAsyncTaskPeripheral != null && mAsyncTaskPeripheral.getStatus() == AsyncTask.Status.RUNNING ) {
            mAsyncTaskPeripheral.cancel( true );
        }

        if( mAsyncTaskVersion != null && mAsyncTaskVersion.getStatus() == AsyncTask.Status.RUNNING ) {
            mAsyncTaskVersion.cancel( true );
        }
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
