package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.util.Log;
import android.util.Patterns;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SimpleCursorAdapter;
import android.widget.TabHost;

import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.VersionAsyncParam;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusDescribeInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLog;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;
import com.samsung.vd.cinemacontrolpanel.Utils.NetworkTools;
import com.samsung.vd.cinemacontrolpanel.Utils.NxCinemaCtrl;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeAdapter;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_Version;

/**
 * Created by doriya on 11/4/16.
 */
public class SystemActivity extends BaseActivity {
    private final String VD_DTAG = "SystemActivity";

    private TabHost mTabHost;

    private EditText mEditIpAddress;
    private EditText mEditSubnetMask;
    private EditText mEditDefaulGateway;
    private StatusDescribeAdapter mAdapterVersion;
    private View listViewFooter;

    View.OnClickListener onRefreshClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            bRefreshFlag = true;
            int tabIdx = mTabHost.getCurrentTab();
            if( 0 == tabIdx ) {
            }
            else if( 1 == tabIdx ) {
            }
            else if( 2 == tabIdx ) {
                NXAsync.getInstance().Execute(CMD_Version , asyncCallbackVersion);
            }
        }
    };

    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_system);

        contentParent = (RelativeLayout)findViewById(R.id.contentParentSystem);
        bI2CFailFlag = false;
        bRefreshFlag = false;
        setOnRefreshClickListener(onRefreshClickListener);

        // Set Title Bar and Status Bar
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - System" , new Intent(getApplicationContext(), TopActivity.class));

        listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //  System Log
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

        //  Initialize Tab
        AddTabs();

        //  IMM Handler
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

        mTabHost.setOnTabChangedListener(mTabChange);
        mTabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) {
                contentParent.setVisibility(View.VISIBLE);
            }
            if( tabId.equals("TAB1") ) {
                contentParent.setVisibility(View.VISIBLE);
            }
            if( tabId.equals("TAB2") ) {
                contentParent.setVisibility(View.VISIBLE);
                //  System Version
                NXAsync.getInstance().Execute(CMD_Version , asyncCallbackVersion);
            }
        }
    };

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


    NXAsync.AsyncCallback asyncCallbackVersion = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            SystemActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( SystemActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            SystemActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if( NXAsync.getInstance().isI2CFailed() || bI2CFailFlag ){
                        if(NXAsync.getInstance().isI2CFailed()) {
                            ShowMessage("I2C Failed.. try again later");
                            if(!bI2CFailFlag) {
                                bI2CFailFlag = true;
                                contentParent.setVisibility(View.GONE);
                                setRefreshBtn(onRefreshClickListener);
                            }
                            CinemaLoading.Hide();
                            NXAsync.getInstance().getAsyncSemaphore().release();
                            return;
                        }else {
                            if(bRefreshFlag) {
                                setRefreshBtn(null);
                                contentParent.setVisibility(View.VISIBLE);
                                bI2CFailFlag = false;
                                bRefreshFlag = false;
                            }else {
                                CinemaLoading.Hide();
                                NXAsync.getInstance().getAsyncSemaphore().release();
                                return;
                            }
                        }
                    }

                    ListView listViewVersion = (ListView)findViewById(R.id.listview_system_version);
                    listViewVersion.addFooterView(listViewFooter);

                    mAdapterVersion = new StatusDescribeAdapter(SystemActivity.this, R.layout.listview_row_status_describe);
                    listViewVersion.setAdapter( mAdapterVersion );

                    VersionAsyncParam versionAsyncParam = NXAsync.getInstance().getVersionAsyncParam();
                    NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

                    byte[] napVersion = versionAsyncParam.getNapVersion();
                    byte[] sapVersion = versionAsyncParam.getSapVersion();
                    byte[] srvVersion = versionAsyncParam.getSrvVersion();
                    byte[] clnVersion = versionAsyncParam.getClnVersion();
                    byte[] pfpgaVersion = versionAsyncParam.getPfpgaVersion();

                    //Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );
                    String[][] strVersion = {
//                            { "Application", new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date) },
                            { "N.AP", (napVersion != null && napVersion.length != 0) ? new String(napVersion) : "Unknown" },
                            { "S.AP", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion) : "Unknown" },
                            { "P.FPGA", (pfpgaVersion != null && pfpgaVersion.length != 0) ? String.format(Locale.US, "%05d", ctrl.ByteArrayToInt(pfpgaVersion)) : "Unknown" },
//                            { "IPC Server", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion) : "Unknown" },
//                            { "IPC Client", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion) : "Unknown" },
                    };

                    Log.i(VD_DTAG, ">>> Version Information.");
                    for( int i = 0; i < strVersion.length; i++ ) {
                        Log.i(VD_DTAG, String.format(Locale.US, " -. %-12s: %s", strVersion[i][0], strVersion[i][1]));
                    }

                    //
                    //  Request VD Cinema. ( Remove Build Time )
                    //
                    for( int i = 0; i < strVersion.length; i++ ) {
                        if( strVersion[i][1].equals("Unknown") )
                            continue;

                        String[] strTemp = strVersion[i][1].split( "\\(" );
                        strVersion[i][1] = strTemp[0].trim();
                    }

                    for( int i = 0; i < strVersion.length; i++ ) {
                        mAdapterVersion.add( new StatusDescribeInfo( strVersion[i][0], strVersion[i][1] ) );
                        mAdapterVersion.notifyDataSetChanged();
                    }
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    }; //asyncCallbackVersion
}
