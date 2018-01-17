package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.util.Patterns;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SimpleCursorAdapter;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Created by doriya on 11/4/16.
 */
public class SystemActivity extends AppCompatActivity {
    private final String VD_DTAG = "SystemActivity";

    private TabHost mTabHost;

    private EditText mEditIpAddress;
    private EditText mEditSubnetMask;
    private EditText mEditDefaulGateway;
    private StatusDescribeAdapter mAdapterVersion;

    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
        setContentView(R.layout.activity_system);

        //
        //  Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - System" );
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

        if( !((CinemaInfo)getApplicationContext()).IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !((CinemaInfo)getApplicationContext()).IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        //
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar ) );
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //
        //  System Log
        //
        ListView listViewLog = (ListView)findViewById(R.id.listview_system_log);
        listViewLog.addFooterView(listViewFooter);

        CinemaLog log = new CinemaLog( getApplicationContext() );
        Cursor cursor = log.GetCursorDatabases();

        String[] from = {
            CinemaLog.FIELD_LOG_LOCAL_DATE,
            CinemaLog.FIELD_LOG_ACCOUNT,
            CinemaLog.FIELD_LOG_DESCRIBE
        };

        int[] to = {
            R.id.listview_row_system_log_date,
            R.id.listview_row_system_log_account,
            R.id.listview_row_system_log_describe
        };

        SimpleCursorAdapter adapter = new SimpleCursorAdapter( getApplicationContext(), R.layout.listview_row_system_log, cursor, from, to, CursorAdapter.FLAG_REGISTER_CONTENT_OBSERVER );
        listViewLog.setAdapter( adapter );

        //
        //  System Version
        //
        ListView listViewVersion = (ListView)findViewById(R.id.listview_system_version);
        listViewVersion.addFooterView(listViewFooter);

        mAdapterVersion = new StatusDescribeAdapter(this, R.layout.listview_row_status_describe);
        listViewVersion.setAdapter( mAdapterVersion );

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] napVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
        byte[] sapVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
        byte[] srvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
        byte[] clnVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
        byte[] pfpgaVersion = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );

        Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );
        String[][] strVersion = {
                { "Application", new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date) },
                { "N.AP", (napVersion != null && napVersion.length != 0) ? new String(napVersion) : "Unknown" },
                { "S.AP", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion) : "Unknown" },
                { "P.FPGA", (pfpgaVersion != null && pfpgaVersion.length != 0) ? String.format(Locale.US, "%05d", ctrl.ByteArrayToInt(pfpgaVersion)) : "-1" },
                { "IPC Server", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion) : "Unknown" },
                { "IPC Client", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion) : "Unknown" },
        };

        Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Server  : %s", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion) : "Unknown"));
        Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Client  : %s", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion) : "Unknown"));


        for( int i = 0; i < strVersion.length; i++ ) {
            mAdapterVersion.add( new StatusDescribeInfo( strVersion[i][0], strVersion[i][1] ) );
            mAdapterVersion.notifyDataSetChanged();
        }

        //
        //  Initialize Tab
        //
        AddTabs();

        //
        //  IMM Handler
        //
        mEditIpAddress = (EditText)findViewById(R.id.editIpAddress);
        mEditSubnetMask = (EditText)findViewById(R.id.editSubnetMask);
        mEditDefaulGateway = (EditText)findViewById(R.id.editDefaultGateway);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditIpAddress.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditSubnetMask.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditDefaulGateway.getWindowToken(), 0);
            }
        });

        GetIPAddress();

        Button btnApply = (Button)findViewById(R.id.btnIpApply);
        btnApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SaveIPAddress();
            }
        });
    }

    private void AddTabs() {
        mTabHost = (TabHost)findViewById( R.id.tabHost );
        mTabHost.setup();

        TabHost.TabSpec tabSpec0 = mTabHost.newTabSpec( "TAB0" );
        tabSpec0.setIndicator("IP CONFIG");
        tabSpec0.setContent(R.id.tab_system_ip);

        TabHost.TabSpec tabSpec1 = mTabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("SYSTEM LOG");
        tabSpec1.setContent(R.id.tab_system_log);

        TabHost.TabSpec tabSpec2 = mTabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("SYSTEM VERSION");
        tabSpec2.setContent(R.id.tab_system_version);

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);
        mTabHost.addTab(tabSpec2);

        mTabHost.setCurrentTab(0);
    }

    private void GetIPAddress() {
        EditText editIpAddress = (EditText)findViewById(R.id.editIpAddress);
        EditText editNetmask = (EditText)findViewById(R.id.editSubnetMask);
        EditText editGateway = (EditText)findViewById(R.id.editDefaultGateway);
        EditText editDns1 = (EditText)findViewById(R.id.editDns1);
        EditText editDns2 = (EditText)findViewById(R.id.editDns2);
        EditText editImbAddress = (EditText)findViewById(R.id.editImbAddress);

        String strIpAddress, strNetmask, strGateway, strDns1, strDns2, strImbAddress;

        try {
            BufferedReader inReader = new BufferedReader(new FileReader("/system/bin/nap_network"));
            try {
                strIpAddress = inReader.readLine(); // param: ip address ( mandatory )
                strNetmask = inReader.readLine();   // param: netmask    ( mandatory )
                strGateway = inReader.readLine();   // param: gateway    ( mandatory )
                inReader.readLine();                // param: network    ( mandatory but auto generate )
                strDns1 = inReader.readLine();      // param: dns1       ( optional )
                strDns2 = inReader.readLine();      // param: dns2       ( optional )
                strImbAddress = inReader.readLine();// param: imb address( optional )

                editIpAddress.setText(strIpAddress);
                editNetmask.setText(strNetmask);
                editGateway.setText(strGateway);
                editDns1.setText((strDns1 != null) ? strDns1 : "");
                editDns2.setText((strDns2 != null) ? strDns2 : "");
                editImbAddress.setText((strImbAddress != null) ? strImbAddress : "");

                inReader.close();

            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    private void SaveIPAddress() {
        EditText editIp = (EditText)findViewById(R.id.editIpAddress);
        EditText editNetmask = (EditText)findViewById(R.id.editSubnetMask);
        EditText editGateway = (EditText)findViewById(R.id.editDefaultGateway);
        EditText editDns1 = (EditText)findViewById(R.id.editDns1);
        EditText editDns2 = (EditText)findViewById(R.id.editDns2);
        EditText editImbAddress = (EditText)findViewById(R.id.editImbAddress);

        String strIp = editIp.getText().toString();
        String strNetmask = editNetmask.getText().toString();
        String strGateway = editGateway.getText().toString();
        String strDns1 = editDns1.getText().toString();
        String strDns2 = editDns2.getText().toString();
        String strImbAddress = editImbAddress.getText().toString();

        //
        //  Check Parameter.
        //
        if( !Patterns.IP_ADDRESS.matcher(strIp).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check ip address. ( %s )", strIp));
            return ;
        }

        if( !Patterns.IP_ADDRESS.matcher(strNetmask).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check netmask. ( %s )", strNetmask));
            return ;
        }

        if( !Patterns.IP_ADDRESS.matcher(strGateway).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check gateway. ( %s )", strGateway));
            return ;
        }

        if( strIp.equals(strGateway) ) {
            ShowMessage(String.format(Locale.US, "Please check ip address and gateway. ( ip:%s, gateway:%s )", strIp, strGateway));
            return ;
        }

        if( !strDns1.equals("") && !Patterns.IP_ADDRESS.matcher(strDns1).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check check dns1. ( %s )", strDns1));
            return ;
        }

        if( !strDns2.equals("") && !Patterns.IP_ADDRESS.matcher(strDns2).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check check dns2. ( %s )", strDns2));
            return ;
        }

        if( !strImbAddress.equals("") && !Patterns.IP_ADDRESS.matcher(strImbAddress).matches() ) {
            ShowMessage(String.format(Locale.US, "Please check check IMB ip address. ( %s )", strImbAddress));
            return ;
        }

        // NetworkTools tools = new NetworkTools();
        // tools.SetConfig( strIp, strNetmask, strGateway, strDns1, strDns2, strImbAddress );

        // if( tools.Ping( strGateway ) ) ShowMessage( "Valid IP Address.");
        // else ShowMessage("Please Check IP Address. ( Invalid IP Address )");

        new NetworkTools().SetConfig( strIp, strNetmask, strGateway, strDns1, strDns2, strImbAddress );
        ShowMessage("Change IP Address.");
        ((CinemaInfo)getApplicationContext()).InsertLog("Change IP Address.");
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

        bindService(new Intent(this, CinemaService.class), mConnection, Context.BIND_AUTO_CREATE);
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
