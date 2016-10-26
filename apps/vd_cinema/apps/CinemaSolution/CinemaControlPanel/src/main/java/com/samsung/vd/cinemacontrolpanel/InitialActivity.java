package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.Settings;
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

    private final String[] SCREEN_SAVING = {
        "Disable", "1 min", "3 min", "5 min", "10 min", "20 min", "30 min",
    };

    private VdTimeZone mTimeZone;

    private EditText mEditPassword;
    private EditText mEditConfirm;
    private EditText mEditCabinet;

    private static Toast mToast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_initial);

        if( CheckInitialize() ) {
            Intent intent = new Intent(getApplicationContext(), LoginActivity.class);
            startActivity(intent);
            return;
        }
        else {
//            Intent intent = getApplicationContext().getPackageManager().getLaunchIntentForPackage("touchscreen.calibration");
//            startActivity(intent);
        }

        // Configuration Title Bar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.title_bar_initial));
        titleBar.SetTitle( "Cinema LED Display System Initialize" );
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
        ArrayAdapter<String> adapterSpinScreenSaving = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, SCREEN_SAVING);
        Spinner spinnerScreenSaving = (Spinner)findViewById(R.id.spinnerScreenSaving);
        spinnerScreenSaving.setAdapter(adapterSpinScreenSaving);

        Button btnAccept = (Button)findViewById(R.id.btnInitAccept);
        btnAccept.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckParameter() ) {
                    Intent intent = new Intent(v.getContext(), LoginActivity.class);
                    startActivity(intent);
                }

//                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
//                ctrl.Send( 0xFF, NxCinemaCtrl.CMD_TCON_STATE );
            }
        });


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


        //
        //  Test Code : Move forcibly activity.
        //
//        Intent intent = new Intent(getApplicationContext(), AccountActivity.class);
//        startActivity(intent);


        //
        //  Test Code : Read LCD Brightness
        //
//        try {
//            int brightness = Settings.System.getInt( getContentResolver(), "screen_brightness");
//            Log.i( VD_DTAG, ">>>>>>>>>> " + String.valueOf(brightness) );
//        } catch (Settings.SettingNotFoundException e) {
//            e.printStackTrace();
//        }

        //
        //  Test Code : Write Brightness
        //
//        Settings.System.putInt( getContentResolver(), "screen_brightness", 255 );
    }

    private boolean CheckInitialize() {
        //
        //  Check Initialized
        //

        return false;
    }

    private boolean CheckParameter() {
//        String strPassword = mEditPassword.getText().toString();
//        String strConfirm = mEditConfirm.getText().toString();
//        String strCabinet = mEditCabinet.getText().toString();
//
//        if( !strPassword.equals(strConfirm) || strPassword.equals("") || strConfirm.equals("") ) {
//            ShowMessage("Please check password.");
//            return false;
//        }
//
//        if( strCabinet.equals("") || Integer.parseInt(strCabinet) <= 0 ) {
//            ShowMessage("Please check cabinet number.");
//            return false;
//        }

        return true;
    }

    private void ShowMessage( String strMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_SHORT);

        mToast.setText(strMsg);
        mToast.show();
    }

    //
    //  For ScreenSaver
    //
    private ScreenSaverService mService = null;
    private boolean mServiceRun = false;

    @Override
    protected void onStart() {
        super.onStart();

        Intent intent = new Intent(this, ScreenSaverService.class);
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
        mService.RefreshScreenSaver();
        return super.dispatchTouchEvent(ev);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            ScreenSaverService.LocalBinder binder = (ScreenSaverService.LocalBinder)service;
            mService = binder.GetService();
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };
}
