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
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.lang.reflect.Array;
import java.util.Locale;

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

    private byte[]  mCabinet;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

        //
        // Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Diagnostics" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mService.TurnOff();
            }
        });

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
        ListView listViewTcon = (ListView)findViewById(R.id.listView_tcon);
        listViewTcon.addFooterView(listViewFooter);

        mAdapterTcon = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewTcon.setAdapter( mAdapterTcon );

        for(int i = 0; i < mCabinet.length; i++ )
        {
            mAdapterTcon.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", mCabinet[i] - CinemaInfo.OFFSET_TCON ) ));
            mAdapterTcon.notifyDataSetChanged();
        }

        //
        //  LED OPEN
        //
        ListView listViewLedOpen = (ListView)findViewById(R.id.listview_led_open);
        listViewLedOpen.addFooterView(listViewFooter);

        mAdapterLedOpen = new StatusDetailAdapter(this, R.layout.listview_row_status_detail, NxCinemaCtrl.CMD_TCON_OPEN_NUM);
        listViewLedOpen.setAdapter( mAdapterLedOpen );

        for(int i = 0; i < mCabinet.length; i++ )
        {
            mAdapterLedOpen.add( new StatusDetailInfo( String.format( Locale.US, "Cabinet %02d", mCabinet[i] - CinemaInfo.OFFSET_TCON ) ));
            mAdapterLedOpen.notifyDataSetChanged();
        }

        //
        //  LED SHORT
        //
        ListView listViewLedShort = (ListView)findViewById(R.id.listview_led_short);
        listViewLedShort.addFooterView(listViewFooter);

        mAdapterLedShort = new StatusDetailAdapter(this, R.layout.listview_row_status_detail, NxCinemaCtrl.CMD_TCON_SHORT_NUM);
        listViewLedShort.setAdapter( mAdapterLedShort );

        for(int i = 0; i < mCabinet.length; i++ )
        {
            mAdapterLedShort.add( new StatusDetailInfo( String.format( Locale.US, "Cabinet %02d", mCabinet[i] - CinemaInfo.OFFSET_TCON ) ));
            mAdapterLedShort.notifyDataSetChanged();
        }

        //
        //  CABINET DOOR
        //
        ListView listViewCabinetDoor = (ListView)findViewById(R.id.listView_cabinet_door);
        listViewCabinetDoor.addFooterView(listViewFooter);

        mAdapterCabinetDoor = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewCabinetDoor.setAdapter( mAdapterCabinetDoor );

        for(int i = 0; i < mCabinet.length; i++ )
        {
            mAdapterCabinetDoor.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", mCabinet[i] - CinemaInfo.OFFSET_TCON ) ));
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
        tabSpec0.setContent(R.id.tabTcon);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("LED OPEN");
        tabSpec1.setContent(R.id.tabLedOpen);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("LED SHORT");
        tabSpec2.setContent(R.id.tabLedShort);

        TabHost.TabSpec tabSpec3 = mTabHost.newTabSpec( "TAB3" );
        tabSpec3.setIndicator("CABINET DOOR");
        tabSpec3.setContent(R.id.tabCabinetDoor);

        TabHost.TabSpec tabSpec4 = mTabHost.newTabSpec( "TAB4" );
        tabSpec4.setIndicator("BATTERY");
        tabSpec4.setContent(R.id.tabBattery);

        TabHost.TabSpec tabSpec5 = mTabHost.newTabSpec( "TAB5" );
        tabSpec5.setIndicator("PERIPHERAL");
        tabSpec5.setContent(R.id.tabPeripheral);

        TabHost.TabSpec tabSpec6 = mTabHost.newTabSpec( "TAB6" );
        tabSpec6.setIndicator("VERSION");
        tabSpec6.setContent(R.id.tabVersion);

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

                byte[] result = NxCinemaCtrl.GetInstance().Send(mCabinet[i], NxCinemaCtrl.CMD_TCON_STATUS, null);
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

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( bValidPort0 )   ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MODE_LOD, null);
            if( bValidPort1 )   ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MODE_LOD, null);

            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                if( isCancelled() ) {
                    return null;
                }

                byte[] result = ctrl.Send( mCabinet[i], NxCinemaCtrl.CMD_TCON_OPEN_NUM, null);
                if (result == null || result.length == 0)
                    continue;

                StatusDetailInfo info = mAdapter.getItem(i);
                int value = ctrl.ByteArrayToInt16(result, NxCinemaCtrl.FORMAT_INT16);

                if( value == 0xFFFF ) {
                    info.SetStatus(-1);
                    info.SetDescription( "-" );
                }
                else {
                    if( value == 0 )
                        info.SetStatus(1);
                    else
                        info.SetStatus(0);

                    info.SetDescription(String.valueOf( String.valueOf(value) ) );
                }

                publishProgress();
            }

            if( bValidPort0 )   ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MODE_NORMAL, null);
            if( bValidPort1 )   ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MODE_NORMAL, null);

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
                byte[] result = NxCinemaCtrl.GetInstance().Send(mCabinet[i], NxCinemaCtrl.CMD_TCON_DOOR_STATUS, null);
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
                StatusSimpleInfo info = mAdapter.getItem(1);
                info.SetStatus(0);

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

                if (isCancelled()) return null;

                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_STATUS, null);
                if (result != null || result.length > 0 ) {
                    info.SetStatus((int)result[0]);
                    publishProgress();
                }
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

        if( mAsyncTaskLedOpenNum != null ) {
            if( mAsyncTaskLedOpenNum.getStatus() == AsyncTask.Status.RUNNING ) {
                mAsyncTaskLedOpenNum.cancel(true);
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

        mAsyncTaskStatus = null;
        mAsyncTaskLedOpenNum = null;
        mAsyncTaskCabinetDoor = null;
        mAsyncTaskPeripheral = null;
        mAsyncTaskVersion = null;

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
