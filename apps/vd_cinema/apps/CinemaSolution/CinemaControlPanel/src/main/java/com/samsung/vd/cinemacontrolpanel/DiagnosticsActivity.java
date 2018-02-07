package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ExpandableListView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Locale;

/**
 * Created by doriya on 8/16/16.
 */
public class DiagnosticsActivity extends AppCompatActivity {
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

    int[][] mRegDataFullWhite = {
            {0x0004, 0},	// TCON_REG_XYZ_TO_RGB
            {0x0024, 10},   // TCON_REG_PATTERN
            {0x0025, 0},    // TCON_REG_LEFT
            {0x0026, 1024}, // TCON_REG_RIGHT
            {0x0027, 0},    // TCON_REG_TOP
            {0x0028, 2160}, // TCON_REG_DOWN
            {0x0029, 4095}, // TCON_REG_BOX_R
            {0x002A, 4095}, // TCON_REG_BOX_G
            {0x002B, 4095}, // TCON_REG_BOX_B
    };

    int[] mDatFullWhite = new int[mRegDataFullWhite.length];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
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

        new AsyncTaskFullWhiteBackup().execute();

        CheckBox checkFullWhite = (CheckBox)findViewById(R.id.checkFullWhite);
        checkFullWhite.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskFullWhite( !((CheckBox)view).isChecked() ).execute();
            }
        });

        CheckBox checkLedOpenDetect = (CheckBox)findViewById(R.id.checkLedOpenDetect);
        checkLedOpenDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskLedOpenDetect(((CheckBox)view).isChecked()).execute();
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
        new AsyncTaskTconStatus(mAdapterTconStatus).execute();
    }

    private void UpdateTconLvds() {
        new AsyncTaskTconLvds(mAdapterTconLvds).execute();
    }

    private void UpdateLedOpen() {
//        new AsyncTaskFullWhiteBackup().execute();
        new AsyncTaskLedOpenNum(mAdapterLedOpen).execute();
    }

    private void UpdateCabinetDoor() {
        new AsyncTaskCabinetDoor(mAdapterCabinetDoor).execute();
    }

    private void UpdatePeripheral() {
        new AsyncTaskPeripheral(mAdapterPeripheral).execute();
    }

    private void UpdateVersion() {
        new AsyncTaskVersion(mAdapterVersion).execute();
    }

    //
    //
    //
    private class AsyncTaskTconStatus extends AsyncTask<Void, Integer, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskTconStatus( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                if( result[0] != StatusSimpleInfo.PASS ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (int)result[0]);
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();

            if( values[1] == StatusSimpleInfo.ERROR ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Error.", values[0] );
                ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
            }
            if( values[1] == StatusSimpleInfo.FAIL ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Failed.", values[0] );
                ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "AsyncTaskTconStatus Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            mAdapter.clear();
            mTextTconStatus.setVisibility(View.GONE);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mAdapter.getCount() == 0 ) {
                mTextTconStatus.setVisibility(View.VISIBLE);
            }
            else {
                mTextTconStatus.setVisibility(View.GONE);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "TCON Status Check Done.");

            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconLvds extends  AsyncTask<Void, Integer, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskTconLvds( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_LVDS_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                if( result[0] != StatusSimpleInfo.PASS ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (int)result[0]);
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();

            if( values[1] == StatusSimpleInfo.ERROR ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Error.", values[0] );
                ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
            }
            if( values[1] == StatusSimpleInfo.FAIL ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Failed.", values[0] );
                ((CinemaInfo)getApplicationContext()).InsertLog(strLog);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "TCON LVDS Check Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            mAdapter.clear();
            mTextTconLvds.setVisibility(View.GONE);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mAdapter.getCount() == 0 ) {
                mTextTconLvds.setVisibility(View.VISIBLE);
            }
            else {
                mTextTconLvds.setVisibility(View.GONE);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "TCON LVDS Check Done.");

            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenNum extends AsyncTask<Void, Integer, Void> {
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

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x89} );

            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet ));
                    publishProgress(cabinet & 0xFF, (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                int value = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.FORMAT_INT16 );
                if( value != 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d)", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, value ));
                    publishProgress(cabinet & 0xFF, (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, value);
                }
            }

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x89} );

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            //
            //  values[0] : Cabinet ID, values[1] : Slave Address, values[2] : Result
            //
            mAdapter.add(new StatusDetailInfo(String.format(Locale.US, "Cabinet %02d", values[1]), values[0], 0, (0 > values[2]) ? "-" : String.valueOf(values[2])));
            mAdapter.notifyDataSetChanged();

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "LED Open Check Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            mAdapter.clear();
            mTextLedOpen.setVisibility(View.GONE);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mAdapter.getCount() == 0 ) {
                mTextLedOpen.setVisibility(View.VISIBLE);
            }
            else {
                mTextLedOpen.setVisibility(View.GONE);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "LED Open Check Done.");

            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskFullWhite extends AsyncTask<Void, Void, Void> {
        private boolean mRestore;

        public AsyncTaskFullWhite(boolean bRestore) {
            Log.i(VD_DTAG, String.format("Full White Value : %s", bRestore ? "Restore" : "Full White"));
            mRestore = bRestore;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegDataFullWhite[0].length; i++ )
            {
                byte[] reg = ctrl.IntToByteArray(mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mRestore ? mDatFullWhite[i] : mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "Full White Write Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Full White Write Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskFullWhiteBackup extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegDataFullWhite.length; i++ )
            {
                byte[] reg = ctrl.IntToByteArray(mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = new byte[] { mCabinet[0] };
                inData = ctrl.AppendByteArray(inData, reg);

                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mRegDataFullWhite[i][0] ));
                    return null;
                }

                mDatFullWhite[i] = ctrl.ByteArrayToInt(result);

                Log.i(VD_DTAG, String.format(Locale.US, "Full White Backup. ( 0x%04X, %d )",
                        mRegDataFullWhite[i][0], mDatFullWhite[i]));
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "Full White Read Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Full White Read Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenDetect extends AsyncTask<Void, Void, Void> {
        private boolean mCheck;
        public AsyncTaskLedOpenDetect( boolean bCheck ) {
            mCheck = bCheck;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] reg = ctrl.IntToByteArray(0x0100, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheck ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "Led Open Detection Fla White Write Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Full White Write Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLodRemovalWrite extends AsyncTask<Void, Void, Void> {
        private boolean mRemoval1;
        private boolean mRemoval2;

        public AsyncTaskLodRemovalWrite( CheckBox removal1, CheckBox removal2 ) {
            mRemoval1 = removal1.isChecked();
            mRemoval2 = removal2.isChecked();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] reg, dat, inData;
            byte[] inData0, inData1;

            //
            //  Removal #1
            //
            reg = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
            dat = ctrl.IntToByteArray(mRemoval1 ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
            inData = ctrl.AppendByteArray(reg, dat);

            inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

            //
            //  Removal #2
            //
            reg = ctrl.IntToByteArray(0x00E6, NxCinemaCtrl.FORMAT_INT16);
            dat = ctrl.IntToByteArray(mRemoval2 ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
            inData = ctrl.AppendByteArray(reg, dat);

            inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "AsyncTaskLodRemovalWrite Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskLodRemovalWrite Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLodRemovalRead extends AsyncTask<Void, Boolean, Void> {
        private CheckBox mRemoval1;
        private CheckBox mRemoval2;

        public AsyncTaskLodRemovalRead( CheckBox removal1, CheckBox removal2 ) {
            mRemoval1 = removal1;
            mRemoval2 = removal2;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] reg1, reg2;
            byte[] inData1, inData2;
            byte[] result1, result2;

            reg1 = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
            reg2 = ctrl.IntToByteArray(0x00E6, NxCinemaCtrl.FORMAT_INT16);

            inData1 = ctrl.AppendByteArray(new byte[] { mCabinet[0] }, reg1);
            inData2 = ctrl.AppendByteArray(new byte[] { mCabinet[0] }, reg2);

            result1 = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData1 );
            if( result1 == null || result1.length == 0 ) {
                Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x011E ));
            }

            result2 = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData2 );
            if( result2 == null || result2.length == 0 ) {
                Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x00E6 ));
            }

            int res1 = ctrl.ByteArrayToInt(result1);
            int res2 = ctrl.ByteArrayToInt(result2);

            publishProgress( (res1 == 1), (res2 == 1) );
            return null;
        }

        @Override
        protected void onProgressUpdate(Boolean... values) {
            mRemoval1.setChecked(values[0]);
            mRemoval2.setChecked(values[1]);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "AsyncTaskLodRemovalRead Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "AsyncTaskLodRemovalRead Done.");
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCabinetDoor extends AsyncTask<Void, Integer, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskCabinetDoor( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }
                if( 0 > result[0] ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, StatusSimpleInfo.ERROR);
                }
                else if( result[0] == 0 || result[0] == 2 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress( (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, StatusSimpleInfo.FAIL );
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "Cabinet Door Check Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            mAdapter.clear();
            mTextCabinetDoor.setVisibility(View.GONE);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mAdapter.getCount() == 0 ) {
                mTextCabinetDoor.setVisibility(View.VISIBLE);
            }
            else {
                mTextCabinetDoor.setVisibility(View.GONE);
            }

            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Cabinet Door Check Done.");
            super.onPostExecute(aVoid);
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
            {
                StatusSimpleInfo info = mAdapter.getItem(0);
                info.SetStatus(StatusSimpleInfo.FAIL);

                ((CinemaInfo)getApplicationContext()).SetSecureAlive("false");
                for( int i = 0; i < 16; i++ ) {
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    if( ((CinemaInfo)getApplicationContext()).GetSecureAlive().equals("true") ) {
                        info.SetStatus(StatusSimpleInfo.PASS);
                        break;
                    }
                }

                publishProgress();
            }

            //
            //  P.FPGA
            //
            {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                StatusSimpleInfo info = mAdapter.getItem(1);
                info.SetStatus(StatusSimpleInfo.ERROR);

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_STATUS, null);
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG,  "Unknown Error.");
                    info.SetStatus(StatusSimpleInfo.ERROR);
                }
                else {
                    info.SetStatus( result[0] );
                }

                publishProgress();
            }

            //
            //  IMB
            //
            {
                NetworkTools tools = new NetworkTools();
                StatusSimpleInfo info = mAdapter.getItem(2);

                String strImbAddress = "";
                try {
                    BufferedReader inReader = new BufferedReader(new FileReader("/system/bin/nap_network"));
                    try {
                        inReader.readLine();    // param: ip address ( mandatory )
                        inReader.readLine();    // param: netmask    ( mandatory )
                        inReader.readLine();    // param: gateway    ( mandatory )
                        inReader.readLine();    // param: network    ( mandatory but auto generate )
                        inReader.readLine();    // param: dns1       ( optional )
                        inReader.readLine();    // param: dns2       ( optional )
                        strImbAddress = inReader.readLine();// param: imb address( optional )
                        inReader.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }

                if( strImbAddress != null && !strImbAddress.equals("") ) {
                    try {
                        if( tools.GetEthLink().equals("true") && tools.Ping(strImbAddress) ) {
                            info.SetStatus(StatusSimpleInfo.PASS);
                        }
                        else {
                            info.SetStatus(StatusSimpleInfo.FAIL);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                else {
                    info.SetStatus(StatusSimpleInfo.ERROR);
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

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            Log.i(VD_DTAG, "Peripheral Check Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "Peripheral Check Done.");
        }
    }

    private class AsyncTaskVersion extends AsyncTask<Void, Integer, Void> {
        private StatusDescribeAdapter mAdapter;

        public AsyncTaskVersion( StatusDescribeAdapter adapter) {
            mAdapter = adapter;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_VERSION, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1, -1);
                    continue;
                }

                int version1 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_MSB);
                int version2 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_LSB);
                publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, version1, version2);
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( values[1] == -1 || values[2] == -1 ) {
                mAdapter.add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), "Unknown Version") );
            }
            else {
                mAdapter.add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), String.format( Locale.US, "%05d-%05d", values[1], values[2] )));
            }

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Log.i(VD_DTAG, "TCON Version Check Start.");
            CinemaLoading.Show( DiagnosticsActivity.this );
            mAdapter.clear();
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
            Log.i(VD_DTAG, "TCON Version Check Done.");
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
    //  For Screen Rotation
    //
    private void SetScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        switch( Integer.parseInt(orientation) ) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            default:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
        }
    }

    private void ChangeScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        int curRotate;
        int prvRotate = Integer.parseInt(orientation);
        switch (prvRotate) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            default:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_ROTATE, String.valueOf(curRotate));
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
        boolean isOn = false;
        if( mService != null ) {
            isOn = mService.IsOn();
            mService.RefreshScreenSaver();
        }

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
