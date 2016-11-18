package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

public class LoginActivity extends AppCompatActivity {
    private static final String VD_DTAG = "LoginActivity";

    private Spinner mSpinnerAccount;

    private EditText mEditPassword;
    private EditText mEditPassword1;
    private EditText mEditPassword2;
    private EditText mEditPassword3;

    private boolean mRescueMode = false;

    private AccountPreference mAccountPreference;
    private String[] mStrAccount;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        //
        // Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Login" );

        // For Debugging
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                startActivity( new Intent(v.getContext(), InitialActivity.class) );
//                overridePendingTransition(0, 0);
//                finish();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mService.TurnOff();
            }
        });

        //
        //  Configuration Status Bar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());
        mStrAccount = new String[] {
            AccountPreference.GROUP_ROOT,
            AccountPreference.GROUP_SERVICE,
            AccountPreference.GROUP_CALIBRATOR,
            AccountPreference.GROUP_OPERATOR };

        mSpinnerAccount = (Spinner)findViewById(R.id.spinnerAccount);
        ArrayAdapter<String> adapterAccount = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrAccount );
        mSpinnerAccount.setAdapter(adapterAccount);
        mSpinnerAccount.setSelection( 3 );

        Button btnLoginNormal = (Button)findViewById(R.id.btnLoginNormal);
        btnLoginNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginRescue).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginNormal).setVisibility(View.VISIBLE);

                mEditPassword1.setText("");
                mEditPassword2.setText("");
                mEditPassword3.setText("");

                mRescueMode = false;
            }
        });

        Button btnLoginRescue = (Button)findViewById(R.id.btnLoginRescue);
        btnLoginRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginNormal).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginRescue).setVisibility(View.VISIBLE);

                mEditPassword.setText("");

                mRescueMode = true;
            }
        });

        Button btnAccpetNormal = (Button)findViewById(R.id.btnLoginNormalAccept);
        btnAccpetNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    startActivity( new Intent(v.getContext(), TopActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                }
            }
        });

        Button btnAcceptRescue = (Button)findViewById(R.id.btnLoginRescueAccept);
        btnAcceptRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    startActivity( new Intent(v.getContext(), TopActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                }
            }
        });

        //
        //  IMM Handler
        //
        mEditPassword = (EditText)findViewById(R.id.editLoginPw);
        mEditPassword1 = (EditText)findViewById(R.id.editLoginPw1);
        mEditPassword2 = (EditText)findViewById(R.id.editLoginPw2);
        mEditPassword3 = (EditText)findViewById(R.id.editLoginPw3);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditPassword.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword1.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword2.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword3.getWindowToken(), 0);
            }
        });
    }

    private boolean CheckAccount() {
        if( !mRescueMode ) {
            //  Normal Login Mode
            Log.i(VD_DTAG, "Normal Mode.");
            String strGroup = mSpinnerAccount.getSelectedItem().toString();
            String strPassword = mEditPassword.getText().toString();

            if( strPassword.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            int index = mAccountPreference.Confirm(strGroup, strPassword);
            if( 0 > index ) {
                ShowMessage("Please check password.");

                mEditPassword.setText("");

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(strGroup);
            info.SetUserId(( mAccountPreference.ReadId(strGroup, index) ));
            info.InsertLog( "Login." );
        }
        else {
            Log.i(VD_DTAG, "Rescue Mode.");
            //  Rescue Login Mode
            String strPassword1 = mEditPassword1.getText().toString();
            String strPassword2 = mEditPassword2.getText().toString();
            String strPassword3 = mEditPassword3.getText().toString();

            if( strPassword1.equals("") ||
                strPassword2.equals("") ||
                strPassword3.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            if( 0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword1) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword2) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword3) ) {
                ShowMessage("Please check password.");

                mEditPassword1.setText(null);
                mEditPassword2.setText(null);
                mEditPassword3.setText(null);

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(AccountPreference.GROUP_ROOT);
            info.InsertLog( "Login." );
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
