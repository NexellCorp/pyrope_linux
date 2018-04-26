package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.Spinner;

import com.samsung.vd.baseutils.VdTimeZone;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.AccountPreference;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaService;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.AsyncCallback;

import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_Initial;

/**
 * Created by doriya on 8/17/16.
 */
public class InitialActivity extends BaseActivity {
    private static final String VD_DTAG = "InitialActivity";

    private VdTimeZone mTimeZone;
    private AccountPreference mAccountPref;
    private EditText mEditPassword;
    private EditText mEditConfirm;
    private EditText mEditCabinet;
    private Spinner mSpinnerScreenSaving;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_initial);

        mAccountPref = new AccountPreference( getApplicationContext() );

        //
        //  For Test..
        //
        ((CinemaInfo)getApplicationContext()).Remove(CinemaInfo.KEY_INITIALIZE);
//
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

        //
        // Set Title Bar and Status Bar
        //
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Initialize" , null);

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
        mSpinnerScreenSaving = (Spinner)findViewById(R.id.spinnerScreenSaving);
        mSpinnerScreenSaving.setAdapter(adapterSpinScreenSaving);

        Button btnAccept = (Button)findViewById(R.id.btnAccept);
        btnAccept.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckParameter() ) {
//                    new AsyncTaskCheckCabinet().execute();
                    NXAsync.getInstance().Execute(CMD_Initial , asyncCallback);
                }
            }
        });

        //
        //  IMM Handler
        //
        mEditPassword = (EditText)findViewById(R.id.editPassword);
        mEditConfirm = (EditText)findViewById(R.id.editConfirm);
        mEditCabinet = (EditText)findViewById(R.id.editCabinet);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
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
        Log.i(VD_DTAG , "CheckInitialize , strTemp : "+strTemp);
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
    // work for UI thread with IPC
    //
    AsyncCallback asyncCallback = new AsyncCallback(){
        @Override
        public void onPreExe() {
            InitialActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( InitialActivity.this );
                    ((CinemaInfo)getApplicationContext()).ClearCabinet();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            InitialActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(NXAsync.getInstance().isI2CFailed()) {
                        ShowMessage("I2C Failed.. try again later");
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                        return;
                    }

                    byte[] cabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
                    int curCabinetNum = Integer.parseInt( mEditCabinet.getText().toString() );

                    //Log.i(VD_DTAG , "detected cabinet.length : "+cabinet.length);
                    //Log.i(VD_DTAG , "ui input CabinetNum  : "+curCabinetNum );
                    if( cabinet.length != curCabinetNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinet() ) {
                        ShowMessage( String.format(Locale.US, "Please check cabinet number. ( value: %d, detect: %d )", curCabinetNum, cabinet.length) );
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                    }
                    else {
                        mAccountPref.Add(AccountPreference.GROUP_ROOT, mEditPassword.getText().toString());

                        ((CinemaInfo)getApplicationContext()).SetUserGroup(AccountPreference.GROUP_ROOT);
                        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[mSpinnerScreenSaving.getSelectedItemPosition()]);
                        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_CABINET_NUM, mEditCabinet.getText().toString());
                        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIALIZE, "true");

                        mService.RefreshScreenSaver();
                        startActivity( new Intent(getApplicationContext(), LoginActivity.class) );
                        overridePendingTransition(0, 0);
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                        finish();
                    }
                }
            });
        }
    };//asyncCallback

}//InitialActivity class
