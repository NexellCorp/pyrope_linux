package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private LayoutInflater  mInflater;
    private LinearLayout    mParentLayoutMastering;
    private LinearLayout[]  mLayoutMastering;

    private String[][] mMasteringFunc = {
        { "CC00",           "16383" },
        { "CC01",           "16383" },
        { "CC02",           "16383" },
        { "CC10",           "16383" },
        { "CC11",           "16383" },
        { "CC12",           "16383" },
        { "CC20",           "16383" },
        { "CC21",           "16383" },
        { "CC22",           "16383" },
        { "Global",         "7"     },
        { "R Channel",      "511"   },
        { "G Channel",      "511"   },
        { "B Channel",      "511"   },
        { "R Channel(Low)", "15"    },
        { "G Channel(Low)", "15"    },
        { "B Channel(Low)", "15"    },
    };

    private int[] mMasteringValue;

    @Override
    protected void onResume() {
        super.onResume();

        mMasteringValue = new int[mMasteringFunc.length];

        mLayoutMastering = new LinearLayout[mMasteringFunc.length];
        for( int i = 0; i < mMasteringFunc.length; i++ ) {
            mLayoutMastering[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_mastering, mParentLayoutMastering, false );
            AddViewMastering( i, mLayoutMastering, mMasteringFunc );
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_mode);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Display Mode" );
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


        //
        //  Spinner for Mastering
        //
        final String[] strMastering = {
                "DCI Mode", "HDR Mode", "Master Mode"
        };

        final Spinner spinnerMasteringMode = (Spinner)findViewById(R.id.spinnerMasteringMode);
        spinnerMasteringMode.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strMastering));

        Button btnMastringMode1 = (Button)findViewById(R.id.btnMasteringMode);
        btnMastringMode1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });


        //
        //
        //
        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayoutMastering = (LinearLayout)findViewById(R.id.layoutMastering);

        //
        //  Spinner Input Source
        //
        String[] strInputSourceEnable = {
            "Enable",   "Disable",
        };

        String[] strInputResolution = {
            "FHD ( 1920 x 1080 )",  "2K ( 2048 x 1080 ",    "4K ( 4096 x 2160 )",
        };

        String[] strInputSource = {
            "IMB",  "HDMI", "3G-SDI",
        };

        final Spinner spinnerInputSourceEnable  = (Spinner)findViewById(R.id.spinnerInputSourceEnable);
        final Spinner spinnerInputResolution = (Spinner)findViewById(R.id.spinnerInputResolution);
        final Spinner spinnerInputSource = (Spinner)findViewById(R.id.spinnerInputSource);

        spinnerInputSourceEnable.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strInputSourceEnable) );
        spinnerInputResolution.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strInputResolution) );
        spinnerInputSource.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strInputSource) );

        Button btnInputSourceEnableApply = (Button)findViewById(R.id.btnInputSourceEnableApply);
        btnInputSourceEnableApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ApplyInputSourceEnable(spinnerInputSourceEnable.getSelectedItemPosition());
            }
        });

        Button btnInputSourceSelectApply = (Button)findViewById(R.id.btnInputSourceSelectApply);
        btnInputSourceSelectApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ApplyInputSourceSelect(spinnerInputResolution.getSelectedItemPosition(), spinnerInputSource.getSelectedItemPosition() );
            }
        });

        //
        //  Spinner Time
        //
        String[] strYear = new String[100];
        String[] strDay = new String[31];
        String[] strHour = new String[24];
        String[] strMin = new String[60];

        String[] strMonth = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        for( int i = 0; i < strYear.length; i++ )   strYear[i] = String.valueOf(2000 + i);
        for( int i = 0; i < strDay.length; i++ )    strDay[i] = String.valueOf(1 + i);
        for( int i = 0; i < strHour.length; i++ )   strHour[i] = String.valueOf(i);
        for( int i = 0; i < strMin.length; i++ )    strMin[i] = String.valueOf(i);

        final Spinner spinnerYear = (Spinner)findViewById(R.id.spinnerYear);
        final Spinner spinnerMonth = (Spinner)findViewById(R.id.spinnerMonth);
        final Spinner spinnerDay = (Spinner)findViewById(R.id.spinnerDay);
        final Spinner spinnerHour = (Spinner)findViewById(R.id.spinnerHour);
        final Spinner spinnerMin = (Spinner)findViewById(R.id.spinnerMinute);

        spinnerYear.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strYear));
        spinnerMonth.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strMonth));
        spinnerDay.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strDay));
        spinnerHour.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strHour));
        spinnerMin.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strMin));

        Button btnTime = (Button)findViewById(R.id.btnTimeApply);
        btnTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });

        //
        //  Spinner Cabinet Number
        //
        String curCabinetNum = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM);

        final EditText editCabinetNum = (EditText)findViewById(R.id.editCabinet);
        editCabinetNum.setText(curCabinetNum);

        Button btnCabinetNum = (Button)findViewById(R.id.btnCabinetNumApply);
        btnCabinetNum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                CinemaInfo info = (CinemaInfo)getApplicationContext();
                String prevCabinetNum   = info.GetValue(CinemaInfo.KEY_CABINET_NUM);
                String curCabinetNum    = editCabinetNum.getText().toString();

                info.SetValue(CinemaInfo.KEY_CABINET_NUM, curCabinetNum);

                String strLog = String.format( "Change cabinet number. ( %s -> %s)", prevCabinetNum, curCabinetNum );
                info.InsertLog(strLog);
                Log.i(VD_DTAG, strLog);
            }
        });

        //
        //  Spinner Suspend Time
        //
        String[] strSuspendTime = {
            "Disable", "1 min", "3 min", "5 min", "10 min", "20 min", "30 min"
        };

        final Spinner spinnerSuspendTime = (Spinner)findViewById(R.id.spinnerSuspendTime);
        spinnerSuspendTime.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strSuspendTime) );

        Button btnSuspendTime = (Button)findViewById(R.id.btnSuspendTimeApply);
        btnSuspendTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ShowMessage("Apply");
                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[spinnerSuspendTime.getSelectedItemPosition()]);
                mService.RefreshScreenSaver();
            }
        });

        //
        //  IMM Handler
        //
        LinearLayout parent = (LinearLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(editCabinetNum.getWindowToken(), 0);
            }
        });
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Mastering Mode");
        tabSpec1.setContent(R.id.tabMasteringMode);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Input Source");
        tabSpec2.setContent(R.id.tabInputSource);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Set up");
        tabSpec3.setContent(R.id.tabSetup);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
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

    //
    //
    //
    private void AddViewMastering( int index, View[] childView, String[][] funcName ) {
        mParentLayoutMastering.addView(childView[index]);

        final int funcIndex = index;
        final LinearLayout layoutText = (LinearLayout)childView[index].findViewById(R.id.layoutTextMastering0);
        final TextView textView0 = (TextView)childView[index].findViewById(R.id.textMastering0);
        final TextView textView1 = (TextView)childView[index].findViewById(R.id.textMastering1);
        final TextView textView2 = (TextView)childView[index].findViewById(R.id.textMastering2);
        final SeekBar seekBar = (SeekBar)childView[index].findViewById(R.id.seekMastering);
        final Button btnSet = (Button)childView[index].findViewById(R.id.btnMastering);

        if( funcName[index][0].equals("CC00") ) {
            textView0.setText("Gamut");
        }
        else if( funcName[index][0].equals("Global") ) {
            layoutText.setBackgroundResource( R.drawable.draw_line_top_right );
            layoutText.setPadding(0, 15, 0, 15);
            textView0.setText("Brightness");
        }

        mMasteringValue[index] = 0;

        textView1.setText( funcName[index][0] );
        textView2.setText( String.valueOf( mMasteringValue[index]) );

        seekBar.setMax( Integer.parseInt( funcName[index][1] ) );
        seekBar.setProgress( mMasteringValue[index] );

        btnSet.setEnabled( false );

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                textView2.setText(String.valueOf(progress));
                if( mMasteringValue[funcIndex] == progress && btnSet.isEnabled() ) {
                    btnSet.setEnabled(false);
                }
                if( mMasteringValue[funcIndex] != progress && !btnSet.isEnabled() ) {
                    btnSet.setEnabled(true);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                Log.i(VD_DTAG, "onStartTrackingTouch()");
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Log.i(VD_DTAG, "onStopTrackingTouch()");
            }
        });

        btnSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mMasteringValue[funcIndex] = Integer.parseInt( textView2.getText().toString() );
                ApplyMasteringMode( funcIndex, mMasteringValue[funcIndex] );
                btnSet.setEnabled( false );
            }
        });
    }

    //
    //
    //
    private void ApplyInputSourceEnable( int index ) {
        Log.i(VD_DTAG,  String.format( "Apply( %d )", index ) );
    }

    private void ApplyInputSourceSelect( int indexResolution, int indexSource ) {
        Log.i(VD_DTAG,  String.format( "Apply( %d, %d )", indexResolution, indexSource) );
    }

    private void ApplyMasteringMode( int funcIndex, int value ) {
        Log.i( VD_DTAG, String.format( "Apply( %d, %d)", funcIndex, value ) );

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] inData = ctrl.AppendByteArray( new byte[]{(byte)funcIndex}, ctrl.IntToByteArray(value, NxCinemaCtrl.FORMAT_INT16) );

        ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING, inData );
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