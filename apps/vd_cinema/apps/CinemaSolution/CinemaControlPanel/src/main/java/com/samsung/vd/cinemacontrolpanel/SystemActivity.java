package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
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
                String[] networkParam = new String[6];
                networkParam[0] = ((EditText)findViewById(R.id.editIpAddress)).getText().toString();
                networkParam[1] = ((EditText)findViewById(R.id.editSubnetMask)).getText().toString();
                networkParam[2] = ((EditText)findViewById(R.id.editDefaultGateway)).getText().toString();
                networkParam[3] = ((EditText)findViewById(R.id.editDns1)).getText().toString();
                networkParam[4] = ((EditText)findViewById(R.id.editDns2)).getText().toString();
                networkParam[5] = ((EditText)findViewById(R.id.editImbAddress)).getText().toString();

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_SET_NETWORK,
                        getApplicationContext(),
                        networkParam,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                Log.i(VD_DTAG, "Set Network Start.");
                                ShowProgress();

                                if( !(values instanceof String[]) )
                                    return;

                                String[] param = (String[])values;
                                if( !Patterns.IP_ADDRESS.matcher(param[0]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check ip address. ( %s )", param[0]));
                                    return ;
                                }

                                if( !Patterns.IP_ADDRESS.matcher(param[1]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check netmask. ( %s )", param[1]));
                                    return ;
                                }

                                if( !Patterns.IP_ADDRESS.matcher(param[2]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check gateway. ( %s )", param[2]));
                                    return ;
                                }

                                if( (param[0]).equals(param[2]) ) {
                                    ShowMessage(String.format(Locale.US, "Please check ip address and gateway. ( ip:%s, gateway:%s )", param[0], param[2]));
                                    return ;
                                }

                                if( !(param[3]).equals("") && !Patterns.IP_ADDRESS.matcher(param[3]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check check dns1. ( %s )", param[3]));
                                    return ;
                                }

                                if( !(param[4]).equals("") && !Patterns.IP_ADDRESS.matcher(param[4]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check check dns2. ( %s )", param[4]));
                                    return ;
                                }

                                if( !(param[5]).equals("") && !Patterns.IP_ADDRESS.matcher(param[5]).matches() ) {
                                    ShowMessage(String.format(Locale.US, "Please check check IMB ip address. ( %s )", param[5]));
                                    return ;
                                }

                                Log.i(VD_DTAG, "Network Parameter is clean.");
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                if( (Integer)values[0] == CinemaInfo.RET_PASS ) {
                                    ShowMessage("Change IP Address.");
                                    mCinemaInfo.InsertLog("Change IP Address.");
                                }

                                Log.i(VD_DTAG, String.format("Set Network Done. ( ret = %d )", (Integer)values[0]));
                                HideProgress();
                            }
                        },
                        null
                );
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

                if( 0 > (Integer)values[0] )
                    return;

                if( CinemaTask.CMD_TMS_QUE > (Integer)values[0] )
                    return;

                boolean bScale2K= (
                        CinemaTask.TMS_P25_2K_2D == (Integer)values[0] || CinemaTask.TMS_P25_2K_3D == (Integer)values[0] ||
                        CinemaTask.TMS_P33_2K_2D == (Integer)values[0] || CinemaTask.TMS_P33_2K_3D == (Integer)values[0] );
                boolean bMode3D = (
                        CinemaTask.TMS_P25_2K_3D == (Integer)values[0] || CinemaTask.TMS_P25_4K_3D == (Integer)values[0] ||
                        CinemaTask.TMS_P33_2K_3D == (Integer)values[0] || CinemaTask.TMS_P33_4K_2D == (Integer)values[0] );

                //
                //  Do not allow to change pitch ( P25 <--> P33 )
                //
                // final boolean bPitch25= (
                //         CinemaTask.TMS_P25_4K_2D == mode || CinemaTask.TMS_P25_2K_2D == mode ||
                //         CinemaTask.TMS_P25_4K_3D == mode || CinemaTask.TMS_P25_2K_3D == mode );

                UnregisterListener();
                SetCheckResolution(bScale2K ? R.id.radioResolution2k : R.id.radioResolution4k);
                SetCheck3DMode(bMode3D ? R.id.radio3DModeOn : R.id.radio3DModeOff);
                RegisterListener();
            }
        });

        //
        //  Initialize Tab
        //
        AddTabs();
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
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_GET_NETWORK,
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
                        HideProgress();
                    }
                },
                new CinemaTask.ProgressUpdateCallback() {
                    @Override
                    public void onProgressUpdate(Object[] values) {
                        if( !(values[0] instanceof Integer) || !(values[1] instanceof String) )
                            return;

                        EditText[] editNetwork = new EditText[7];
                        editNetwork[0] = (EditText)findViewById(R.id.editIpAddress);
                        editNetwork[1] = (EditText)findViewById(R.id.editSubnetMask);
                        editNetwork[2] = (EditText)findViewById(R.id.editDefaultGateway);
                        editNetwork[3] = null;
                        editNetwork[4] = (EditText)findViewById(R.id.editDns1);
                        editNetwork[5] = (EditText)findViewById(R.id.editDns2);
                        editNetwork[6] = (EditText)findViewById(R.id.editImbAddress);

                        // Log.i(VD_DTAG, String.format(Locale.US, ">> network #%d : %s", (Integer)values[0], (String)values[1]) );

                        if( editNetwork.length <= (Integer)values[0] ||
                            editNetwork[(Integer)values[0]] == null )
                            return;

                        editNetwork[(Integer)values[0]].setText((String)values[1]);
                    }
                }
        );
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
            final View radioButton = view;
            final boolean bScale2K = (radioButton.getId() == R.id.radioResolution2k);

            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_SCALE,
                    getApplicationContext(),
                    bScale2K,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            UnregisterListener();
                            SetCheckResolution( radioButton.getId() );
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            mCinemaInfo.SetValue(
                                    CinemaInfo.KEY_PREG_0x0199,
                                    String.valueOf(bScale2K ? 0 : 1)
                            );
                            mCinemaInfo.SetValue(
                                    CinemaInfo.KEY_TREG_0x018D,
                                    String.valueOf(bScale2K ? 1 : 0)
                            );
                            mCinemaInfo.UpdateDefaultRegister();
                            RegisterListener();
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
            final View radioButton = view;
            final boolean bMode3D = (view.getId() == R.id.radio3DModeOn);

            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_3D,
                    getApplicationContext(),
                    bMode3D,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            UnregisterListener();
                            SetCheck3DMode( radioButton.getId() );
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            RegisterListener();
                            HideProgress();
                        }
                    },
                    null
            );

        }
    };
}
