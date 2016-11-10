package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.os.Handler;
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
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
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

    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_system);

        //
        //  Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutSystem ));
        titleBar.SetTitle( "Cinema LED Display System - System" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });

        //
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutSystem ) );

        //
        //  System Log
        //
        ListView listViewLog = (ListView)findViewById(R.id.listview_system_log);
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
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
        //  Initialize Tab
        //
        AddTabs();

        //
        //  IMM Handler
        //
        mEditIpAddress = (EditText)findViewById(R.id.editIpAddress);
        mEditSubnetMask = (EditText)findViewById(R.id.editSubnetMask);
        mEditDefaulGateway = (EditText)findViewById(R.id.editDefaultGateway);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParentSystem);
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

        mTabHost.addTab(tabSpec0);
        mTabHost.addTab(tabSpec1);

        mTabHost.setOnTabChangedListener(mSystemTabChange);
        mTabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mSystemTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
        }
    };

    private void GetIPAddress() {
        EditText editIpAddress = (EditText)findViewById(R.id.editIpAddress);
        EditText editNetmask = (EditText)findViewById(R.id.editSubnetMask);
        EditText editGateway = (EditText)findViewById(R.id.editDefaultGateway);
        EditText editDns1 = (EditText)findViewById(R.id.editDns1);
        EditText editDns2 = (EditText)findViewById(R.id.editDns2);

        String strIpAddress, strNetmask, strGateway, strDns1, strDns2;

        try {
            BufferedReader inReader = new BufferedReader(new FileReader("/system/bin/nap_network"));
            try {
                strIpAddress = inReader.readLine(); // param: ip address ( mandatory )
                strNetmask = inReader.readLine();   // param: netmask    ( mandatory )
                strGateway = inReader.readLine();   // param: gateway    ( mandatory )
                inReader.readLine();                // param: network    ( mandatory but auto generate )
                strDns1 = inReader.readLine();      // param: dns1       ( optional )
                strDns2 = inReader.readLine();      // param: dns2       ( optional )

                editIpAddress.setText(strIpAddress);
                editNetmask.setText(strNetmask);
                editGateway.setText(strGateway);
                editDns1.setText((strDns1 != null) ? strDns1 : "");
                editDns2.setText((strDns2 != null) ? strDns2 : "");

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

        String strIp = editIp.getText().toString();
        String strNetmask = editNetmask.getText().toString();
        String strGateway = editGateway.getText().toString();
        String strDns1 = editDns1.getText().toString();
        String strDns2 = editDns2.getText().toString();

        //
        //  Check Parameter.
        //
        if( !Patterns.IP_ADDRESS.matcher(strIp).matches() ) {
            ShowMessage("Please check ip address.");
            return ;
        }

        if( !Patterns.IP_ADDRESS.matcher(strNetmask).matches() ) {
            ShowMessage("Please check netmask.");
            return ;
        }

        if( !Patterns.IP_ADDRESS.matcher(strGateway).matches() ) {
            ShowMessage("Please check gateway.");
            return ;
        }

        if( strIp.equals(strGateway) ) {
            ShowMessage("Please check ip address and gateway." );
            return ;
        }

        if( !strDns1.equals("") && !Patterns.IP_ADDRESS.matcher(strDns1).matches() ) {
            ShowMessage("Please check dns1.");
            return ;
        }

        if( !strDns2.equals("") && !Patterns.IP_ADDRESS.matcher(strDns2).matches() ) {
            ShowMessage("Please check dns2.");
            return ;
        }

        String strCidr = GetCidr( strIp, strNetmask );
        try {
            LocalSocket sender = new LocalSocket();
            sender.connect(new LocalSocketAddress("cinema.network"));
            sender.getOutputStream().write(String.format("%s\n%s\n%s\n%s\n%s\n%s\n", strIp, strNetmask, strGateway, strCidr, strDns1, strDns2).getBytes());
            sender.getOutputStream().close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if( Ping( strGateway ) ) ShowMessage( "Valid IP Address.");
        else ShowMessage("Please Check IP Address. ( Invalid IP Address )");

        ((CinemaInfo)getApplicationContext()).InsertLog("Change IP Address.");
    }

    private String GetCidr( String ip, String mask ) {
        String[] strIp = ip.split("\\.");
        String[] strMask = mask.split("\\.");
        int subnet = 0;

        byte[] byteMask = new byte[4];
        int[] intCidr = new int[4];

        for(int i = 0; i < 4; i++) {
            byteMask[i] = (byte)Integer.parseInt(strMask[i]);
            intCidr[i] = Integer.parseInt(strIp[i]) & Integer.parseInt(strMask[i]);
        }

        for(int i = 0; i < 4; i++)
        {
            int tmp = 0;
            for( int j = 0; j < 8; j++ ) {
                tmp = (byteMask[i] >> ( 7 - j) & 0x01);
                if( 0 == tmp ) break;
                subnet++;
            }
            if( 0 == tmp ) break;
        }

        return String.format(Locale.US, "%d.%d.%d.%d/%d", intCidr[0], intCidr[1], intCidr[2], intCidr[3], subnet);
    }

    //
    //  Network Tools
    //
    public boolean Ping(String host) {
        NetworkChecker checker = new NetworkChecker(host);
        checker.start();
        return checker.GetResult();
    }

    private class NetworkChecker extends Thread {
        private String mHost;
        private boolean mResult = false;

        public NetworkChecker( String host ) {
            mHost = host;
        }

        @Override
        public void run() {
            try {
                InetAddress inetAddr = InetAddress.getByName(mHost);
                try {
                    mResult = inetAddr.isReachable(0);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }
        }

        public boolean GetResult() {
            try {
                join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return mResult;
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
