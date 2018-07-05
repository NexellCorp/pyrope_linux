package com.samsung.vd.cinemacontrolpanel;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.util.Patterns;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.SimpleCursorAdapter;
import android.widget.TabHost;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Locale;

/**
 * Created by doriya on 11/4/16.
 */
public class SystemActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "SystemActivity";

    private CinemaInfo mCinemaInfo;

    private RadioButton mRadioResolution2K, mRadioResolution4K;
    private RadioButton mRadio3DModeOn, mRadio3DModeOff;

    private ListView mListViewLog;

    private StatusDescribeAdapter mAdapterVersion;

    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_system);

        //
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();

        final ViewGroup rootGroup = null;
        View listViewFooter = null;
        LayoutInflater inflater = (LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        if( inflater != null ) listViewFooter = inflater.inflate(R.layout.listview_footer_blank, rootGroup, false);

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
                Launch(v.getContext(), TopActivity.class);
            }
        });

        if( !mCinemaInfo.IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !mCinemaInfo.IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        //
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar ) );

        //
        //  INITIAL VALUE
        //
        mRadioResolution2K = (RadioButton)findViewById(R.id.radioResolution2k);
        mRadioResolution4K = (RadioButton)findViewById(R.id.radioResolution4k);
        mRadio3DModeOn = (RadioButton)findViewById(R.id.radio3DModeOn);
        mRadio3DModeOff = (RadioButton)findViewById(R.id.radio3DModeOff);

        mRadioResolution2K.setOnClickListener(mRadioButtonClickResolution);
        mRadioResolution4K.setOnClickListener(mRadioButtonClickResolution);
        mRadio3DModeOn.setOnClickListener(mRadioButtonClick3DMode);
        mRadio3DModeOff.setOnClickListener(mRadioButtonClick3DMode);

        //
        //  NETWORK
        //
        Button btnApply = (Button)findViewById(R.id.btnIpApply);
        btnApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AsyncTaskSetNetwork().execute();
            }
        });

        //
        //  SYSTEM LOG
        //
        mListViewLog = (ListView)findViewById(R.id.listview_system_log);
        mListViewLog.addFooterView(listViewFooter);

        //
        //  SYSTEM VERSION
        //
        ListView listViewVersion = (ListView)findViewById(R.id.listview_system_version);
        listViewVersion.addFooterView(listViewFooter);

        mAdapterVersion = new StatusDescribeAdapter(this, R.layout.listview_row_status_describe);
        listViewVersion.setAdapter( mAdapterVersion );

        //
        //  IMM Handler
        //
        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                if( imm == null )
                    return;

                EditText editIpAddress = (EditText)findViewById(R.id.editIpAddress);
                EditText editSubnetMask = (EditText)findViewById(R.id.editSubnetMask);
                EditText editDefaultGateway = (EditText)findViewById(R.id.editDefaultGateway);

                if( editIpAddress != null ) imm.hideSoftInputFromWindow(editIpAddress.getWindowToken(), 0);
                if( editSubnetMask != null ) imm.hideSoftInputFromWindow(editSubnetMask.getWindowToken(), 0);
                if( editDefaultGateway != null ) imm.hideSoftInputFromWindow(editDefaultGateway.getWindowToken(), 0);
            }
        });

        //
        //  TMS Event Callback
        //
        this.RegisterTmsCallback(new CinemaService.TmsEventCallback() {
            @Override
            public void onTmsEventCallback(Object[] values) {
                if( !(values instanceof Integer[]) )
                    return;

                if( CinemaTask.SCALE_4K == (Integer)values[0] || CinemaTask.SCALE_2K == (Integer)values[0] ) {
                    UnregisterListener();
                    SetCheckResolution((Integer) values[0] == CinemaTask.SCALE_4K ? R.id.radioResolution4k : R.id.radioResolution2k);
                    RegisterListener();
                }
            }
        });

        //
        //  Initialize Tab
        //
        AddTabs();
        RegisterListener();
        UpdateInitial();
   }

    private void AddTabs() {
        TabHost tabHost = (TabHost)findViewById( R.id.tabHost );
        tabHost.setup();

        TabHost.TabSpec tabSpec0 = tabHost.newTabSpec( "TAB0" );
        tabSpec0.setIndicator("INITIAL");
        tabSpec0.setContent(R.id.tab_system_initial);

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec( "TAB1" );
        tabSpec1.setIndicator("IP CONFIG");
        tabSpec1.setContent(R.id.tab_system_ip);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec( "TAB2" );
        tabSpec2.setIndicator("SYSTEM LOG");
        tabSpec2.setContent(R.id.tab_system_log);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec( "TAB3" );
        tabSpec3.setIndicator("SYSTEM VERSION");
        tabSpec3.setContent(R.id.tab_system_version);

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);

        tabHost.setOnTabChangedListener(mSystemTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mSystemTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateInitial();
            if( tabId.equals("TAB1") ) UpdateIpAddress();
            if( tabId.equals("TAB2") ) UpdateSystemLog();
            if( tabId.equals("TAB3") ) UpdateSystemVersion();
        }
    };

    private void UpdateInitial() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_TCON_REG_READ,
                getApplicationContext(),
                new int[] {
                        CinemaInfo.REG_TCON_0x018D
                },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        UnregisterListener();
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof Integer[]) )
                            return;

                        SetCheckResolution( ((Integer)values[0] == 0) ? R.id.radioResolution4k : R.id.radioResolution2k );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_TREG_0x018D, String.valueOf((Integer)values[0] == 0 ? 0 : 1) );
                        mCinemaInfo.SetValue(CinemaInfo.KEY_PREG_0x0199, String.valueOf((Integer)values[0] == 0 ? 1 : 0) );
                        mCinemaInfo.UpdateDefaultRegister();

                        RegisterListener();
                        HideProgress();
                    }
                },
                null
        );

        SetCheck3DMode( mCinemaInfo.IsMode3D() ? R.id.radio3DModeOn : R.id.radio3DModeOff);
    }

    private void UpdateIpAddress() {
        new AsyncTaskGetNetwork().execute();
    }

    private void UpdateSystemLog() {
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
        mListViewLog.setAdapter( adapter );
    }

    private void UpdateSystemVersion() {
        mAdapterVersion.clear();

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_NAP_VERSION,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof String[]))
                            return;

                        mAdapterVersion.add( new StatusDescribeInfo( "N.AP", (String)values[0] ));
                        mAdapterVersion.notifyDataSetChanged();

                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_SAP_VERSION,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof String[]))
                            return;

                        mAdapterVersion.add( new StatusDescribeInfo( "S.AP", (String)values[0] ));
                        mAdapterVersion.notifyDataSetChanged();

                        HideProgress();
                    }
                },
                null
        );

        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PFPGA_VERSION,
                getApplicationContext(),
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        if( !(values instanceof String[]) )
                            return;

                        mAdapterVersion.add( new StatusDescribeInfo( "P.FPGA", (String)values[0] ));
                        mAdapterVersion.notifyDataSetChanged();

                        HideProgress();
                    }
                },
                null
        );
    }

    private void SetCheckResolution( int id ) {
        mRadioResolution2K.setChecked(id == R.id.radioResolution2k);
        mRadioResolution4K.setChecked(id == R.id.radioResolution4k);
    }

    private void SetCheck3DMode( int id ) {
        mRadio3DModeOn.setChecked(id == R.id.radio3DModeOn);
        mRadio3DModeOff.setChecked(id == R.id.radio3DModeOff);
    }

    private void RegisterListener() {
        mRadioResolution2K.setOnClickListener( mRadioButtonClickResolution );
        mRadioResolution4K.setOnClickListener( mRadioButtonClickResolution );
        mRadio3DModeOn.setOnClickListener( mRadioButtonClick3DMode );
        mRadio3DModeOff.setOnClickListener( mRadioButtonClick3DMode );
    }

    private void UnregisterListener() {
        mRadioResolution2K.setOnClickListener( null );
        mRadioResolution4K.setOnClickListener( null );
        mRadio3DModeOn.setOnClickListener( null );
        mRadio3DModeOff.setOnClickListener( null );
    }

    RadioButton.OnClickListener mRadioButtonClickResolution = new RadioButton.OnClickListener() {
        @Override
        public void onClick(View view) {
            SetCheckResolution( view.getId() );

            final boolean bCheck = (view.getId() == R.id.radioResolution2k);
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_SCALE,
                    getApplicationContext(),
                    bCheck ? CinemaTask.SCALE_2K : CinemaTask.SCALE_4K,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            mCinemaInfo.SetValue(
                                    CinemaInfo.KEY_PREG_0x0199,
                                    String.valueOf(bCheck ? 0 : 1)
                            );
                            mCinemaInfo.SetValue(
                                    CinemaInfo.KEY_TREG_0x018D,
                                    String.valueOf(bCheck ? 1 : 0)
                            );
                            mCinemaInfo.UpdateDefaultRegister();
                            HideProgress();
                        }
                    },
                    null
            );
        }
    };

    RadioButton.OnClickListener mRadioButtonClick3DMode = new RadioButton.OnClickListener() {
        @Override
        public void onClick(View view) {
            SetCheck3DMode( view.getId() );
            mCinemaInfo.SetMode3D( (view.getId() == R.id.radio3DModeOn) );
        }
    };

    private class AsyncTaskGetNetwork extends AsyncTask<Void, Object, Void> {
        private EditText[] mEditNetwork;
        private String[] mStrNetwork;

        public AsyncTaskGetNetwork() {
            mEditNetwork = new EditText[7];
            mEditNetwork[0] = (EditText)findViewById(R.id.editIpAddress);
            mEditNetwork[1] = (EditText)findViewById(R.id.editSubnetMask);
            mEditNetwork[2] = (EditText)findViewById(R.id.editDefaultGateway);
            mEditNetwork[3] = null;
            mEditNetwork[4] = (EditText)findViewById(R.id.editDns1);
            mEditNetwork[5] = (EditText)findViewById(R.id.editDns2);
            mEditNetwork[6] = (EditText)findViewById(R.id.editImbAddress);

            mStrNetwork = new String[7];
        }

        @Override
        protected Void doInBackground(Void... voids) {

            try {
                FileReader inFile = new FileReader("/system/bin/nap_network");
                BufferedReader inReader = new BufferedReader(inFile);

                try {
                    for( int i = 0; i < mStrNetwork.length; i++ ) {
                        mStrNetwork[i] = inReader.readLine();
                        publishProgress( i, mStrNetwork[i] );
                    }
                    inReader.close();
                    inFile.close();

                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Object... values) {
            if( mEditNetwork[(Integer)values[0]] != null ) {
                Log.i(VD_DTAG, String.format(Locale.US, ">> network %d : %s", (Integer)values[0], (String)values[1]) );
                mEditNetwork[(Integer)values[0]].setText((String)values[1]);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            ShowProgress();
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            HideProgress();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskSetNetwork extends AsyncTask<Void, Void, Void> {
        private EditText[] mEditNetwork;
        private String[] mStrNetwork;

        public AsyncTaskSetNetwork() {
            mEditNetwork = new EditText[6];
            mEditNetwork[0] = (EditText)findViewById(R.id.editIpAddress);
            mEditNetwork[1] = (EditText)findViewById(R.id.editSubnetMask);
            mEditNetwork[2] = (EditText)findViewById(R.id.editDefaultGateway);
            mEditNetwork[3] = (EditText)findViewById(R.id.editDns1);
            mEditNetwork[4] = (EditText)findViewById(R.id.editDns2);
            mEditNetwork[5] = (EditText)findViewById(R.id.editImbAddress);

            mStrNetwork = new String[6];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            for( int i = 0; i < mEditNetwork.length; i++ ) {
                mStrNetwork[i] = mEditNetwork[i].getText().toString();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( !Patterns.IP_ADDRESS.matcher(mStrNetwork[0]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check ip address. ( %s )", mStrNetwork[0]));
                return ;
            }

            if( !Patterns.IP_ADDRESS.matcher(mStrNetwork[1]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check netmask. ( %s )", mStrNetwork[1]));
                return ;
            }

            if( !Patterns.IP_ADDRESS.matcher(mStrNetwork[2]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check gateway. ( %s )", mStrNetwork[2]));
                return ;
            }

            if( mStrNetwork[0].equals(mStrNetwork[2]) ) {
                ShowMessage(String.format(Locale.US, "Please check ip address and gateway. ( ip:%s, gateway:%s )", mStrNetwork[0], mStrNetwork[2]));
                return ;
            }

            if( !mStrNetwork[3].equals("") && !Patterns.IP_ADDRESS.matcher(mStrNetwork[3]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check check dns1. ( %s )", mStrNetwork[3]));
                return ;
            }

            if( !mStrNetwork[4].equals("") && !Patterns.IP_ADDRESS.matcher(mStrNetwork[4]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check check dns2. ( %s )", mStrNetwork[4]));
                return ;
            }

            if( !mStrNetwork[5].equals("") && !Patterns.IP_ADDRESS.matcher(mStrNetwork[5]).matches() ) {
                ShowMessage(String.format(Locale.US, "Please check check IMB ip address. ( %s )", mStrNetwork[5]));
                return ;
            }

            // NetworkTools tools = new NetworkTools();
            // tools.SetConfig( mStrNetwork[0], mStrNetwork[1], mStrNetwork[2], mStrNetwork[3], mStrNetwork[4], mStrNetwork[5] );
            // if( tools.Ping( strGateway ) ) ShowMessage( "Valid IP Address.");
            // else ShowMessage("Please Check IP Address. ( Invalid IP Address )");

            NetworkTools tools = new NetworkTools();
            tools.SetConfig( mStrNetwork[0], mStrNetwork[1], mStrNetwork[2], mStrNetwork[3], mStrNetwork[4], mStrNetwork[5] );
            ShowMessage("Change IP Address.");
            mCinemaInfo.InsertLog("Change IP Address.");

            HideProgress();
            super.onPostExecute(aVoid);
        }
    }
}
