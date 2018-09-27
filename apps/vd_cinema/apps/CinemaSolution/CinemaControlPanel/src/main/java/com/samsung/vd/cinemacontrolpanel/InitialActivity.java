package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTimeZone;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/17/16.
 */
public class InitialActivity extends CinemaBaseActivity {
    private static final String VD_DTAG = "InitialActivity";

    private CinemaInfo mCinemaInfo;

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

        //
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();

        if( !mCinemaInfo.IsCheckInitial() ) {
            Launch(getApplicationContext(), LoginActivity.class);
            return;
        }

        //
        //  Test Code : Run Initialize Activity.
        //
        //mCinemaInfo.Remove(CinemaInfo.KEY_INITIALIZE);

        if( CheckInitialize() ) {
            //
            //  Test Code : Move forcibly activity.
            //
            //Launch(getApplicationContext(), TopActivity.class);

            Launch(getApplicationContext(), LoginActivity.class);
            return;
        }
        //else {
        //    //
        //    //  Test Code : Touch Calibration
        //    //
        //    LaunchPackage(getApplicationContext(), "touchscreen.calibration");
        //}

        //
        // Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar));
        titleBar.SetTitle( "Cinema LED Display System - Initialize" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);

        if( !mCinemaInfo.IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        //
        //  Configuration Status Bar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //  Account
        //
        mAccountPref = new AccountPreference( getApplicationContext() );

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
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_CHECK_CABINET_NUM,
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
                                    if( !(values instanceof Integer[]) )
                                        return;

                                    int detectCabinetNum = (Integer)values[0];
                                    int desireCabinetNum = Integer.parseInt( mEditCabinet.getText().toString() );

                                    if( detectCabinetNum != desireCabinetNum && mCinemaInfo.IsCheckCabinetNum() ) {
                                        ShowMessage( String.format(Locale.US, "Please check cabinet number. ( desire: %d, detect: %d )", desireCabinetNum, detectCabinetNum) );
                                    }
                                    else {
                                        mAccountPref.Add(AccountPreference.GROUP_ROOT, mEditPassword.getText().toString());

                                        mCinemaInfo.SetUserGroup(AccountPreference.GROUP_ROOT);
                                        mCinemaInfo.SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[mSpinnerScreenSaving.getSelectedItemPosition()]);
                                        mCinemaInfo.SetValue(CinemaInfo.KEY_CABINET_NUM, mEditCabinet.getText().toString());
                                        mCinemaInfo.SetValue(CinemaInfo.KEY_INITIALIZE, "true");

                                        RefreshScreenSaver();

                                        Launch(getApplicationContext(), LoginActivity.class);
                                    }
                                    HideProgress();
                                }
                            },
                            null
                    );
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
                if( null == imm )
                    return;

                imm.hideSoftInputFromWindow(mEditPassword.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditConfirm.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditCabinet.getWindowToken(), 0);
            }
        });
    }

    private boolean CheckInitialize() {
        String strTemp = mCinemaInfo.GetValue(CinemaInfo.KEY_INITIALIZE);
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
}
