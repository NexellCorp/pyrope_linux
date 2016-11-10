package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTimeZone;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/17/16.
 */
public class InitialActivity extends AppCompatActivity {
    private static final String VD_DTAG = "InitialActivity";

    private VdTimeZone mTimeZone;
    private AccountPreference mAccountPref;
    private EditText mEditPassword;
    private EditText mEditConfirm;
    private EditText mEditCabinet;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_initial);

        mAccountPref = new AccountPreference( getApplicationContext() );

        //
        //  For Test..
        //
//        ((CinemaInfo)getApplicationContext()).Remove(CinemaInfo.KEY_INITIALIZE);

        if( CheckInitialize() ) {
            //
            //  Test Code : Move forcibly activity.
            //
//            startActivity( new Intent(getApplicationContext(), TopActivity.class) );
//            overridePendingTransition(0, 0);
//            finish();

            startActivity( new Intent(getApplicationContext(), LoginActivity.class) );
            overridePendingTransition(0, 0);
            finish();
            return;
        }
//        else {
//            startActivity( getApplicationContext().getPackageManager().getLaunchIntentForPackage("touchscreen.calibration") );
//        }

        // Configuration Title Bar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.title_bar_initial));
        titleBar.SetTitle( "Cinema LED Display System - Initialize" );
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);

        // Configuration Status Bar
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.status_bar_initial) );

        //
        //  Spinner for TimeZone
        //
        Spinner spinnerTimeZone = (Spinner)findViewById(R.id.spinnerTimeZone);
        mTimeZone = new VdTimeZone( getApplicationContext() );

        ArrayAdapter<String> adapterSpinTimeZone = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mTimeZone.GetTimeZoneList());
        spinnerTimeZone.setAdapter(adapterSpinTimeZone);
        spinnerTimeZone.setSelection( mTimeZone.GetTimeZoneIndex() );
        spinnerTimeZone.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mTimeZone.SetTimeZone( position );
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        //
        //  Spinner for Screen Saving
        //
        String[] strScreenSaverInfo = {
            "Disable", "1 min", "3 min", "5 min", "10 min", "20 min", "30 min"
        };

        ArrayAdapter<String> adapterSpinScreenSaving = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strScreenSaverInfo);
        final Spinner spinnerScreenSaving = (Spinner)findViewById(R.id.spinnerScreenSaving);
        spinnerScreenSaving.setAdapter(adapterSpinScreenSaving);

        Button btnAccept = (Button)findViewById(R.id.btnInitAccept);
        btnAccept.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckParameter() ) {
                    mAccountPref.Add(AccountPreference.GROUP_ROOT, mEditPassword.getText().toString());

                    ((CinemaInfo)getApplicationContext()).SetUserGroup(AccountPreference.GROUP_ROOT);
                    ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[spinnerScreenSaving.getSelectedItemPosition()]);
                    ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_CABINET_NUM, mEditCabinet.getText().toString());
                    ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIALIZE, "true");

                    mService.RefreshScreenSaver();
                    startActivity( new Intent(v.getContext(), LoginActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                }
            }
        });

        //
        //  IMM Handler
        //
        mEditPassword = (EditText)findViewById(R.id.editPassword);
        mEditConfirm = (EditText)findViewById(R.id.editConfirm);
        mEditCabinet = (EditText)findViewById(R.id.editCabinet);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParentInitial);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditPassword.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditConfirm.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditCabinet.getWindowToken(), 0);
            }
        });
    }

    private boolean CheckInitialize() {
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIALIZE);
        return (null != strTemp) && (strTemp.equals( "true" ));
    }

    private boolean CheckParameter() {
        String strPassword = mEditPassword.getText().toString();
        String strConfirm = mEditConfirm.getText().toString();
        String strCabinet = mEditCabinet.getText().toString();

        if( !strPassword.equals(strConfirm) || strPassword.equals("") || strConfirm.equals("") ) {
            ShowMessage("Please check password.");
            return false;
        }

        if( strCabinet.equals("") || Integer.parseInt(strCabinet) <= 0 ) {
            ShowMessage("Please check cabinet number.");
            return false;
        }

        return true;
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
