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

import com.samsung.vd.baseutils.VdLoginDatabase;
import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

public class LoginActivity extends AppCompatActivity {
    private static final String VD_DTAG = "LoginActivity";
    private static final String[] ACCOUNT = {
        "root", "service", "calibrator", "operator",
    };

    private Spinner mSpinnerAccount;

    private EditText mEditPassword;
    private EditText mEditPassword1;
    private EditText mEditPassword2;
    private EditText mEditPassword3;

    private boolean mRescueMode = false;

    private static Toast mToast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        // Configuration Title Bar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.title_bar_login ));
        titleBar.SetTitle( "Cinema LED Display System Login" );
        titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);

        // For Debugging
        //titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), InitialActivity.class);
                startActivity(intent);
            }
        });

        // Configuration Status Bar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.status_bar_login) );

        mSpinnerAccount = (Spinner)findViewById(R.id.spinnerAccount);
        ArrayAdapter<String> adapterAccount = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, ACCOUNT );
        mSpinnerAccount.setAdapter(adapterAccount);
        mSpinnerAccount.setSelection( 3 );

        Button btnLoginNormal = (Button)findViewById(R.id.btnLoginNormal);
        btnLoginNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginRescue).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginNormal).setVisibility(View.VISIBLE);

                mEditPassword1.setText(null);
                mEditPassword2.setText(null);
                mEditPassword3.setText(null);

                mRescueMode = true;
            }
        });

        Button btnLoginRescue = (Button)findViewById(R.id.btnLoginRescue);
        btnLoginRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginNormal).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginRescue).setVisibility(View.VISIBLE);

                mEditPassword.setText(null);

                mRescueMode = false;
            }
        });

        Button btnAccpetNormal = (Button)findViewById(R.id.btnLoginNormalAccept);
        btnAccpetNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    Intent intent = new Intent(v.getContext(), TopActivity.class);
                    startActivity(intent);
                }
            }
        });

        Button btnAcceptRescue = (Button)findViewById(R.id.btnLoginRescueAccept);
        btnAcceptRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    Intent intent = new Intent(v.getContext(), TopActivity.class);
                    startActivity(intent);
                }
            }
        });

        mEditPassword = (EditText)findViewById(R.id.editTextLoginPw);
        mEditPassword1 = (EditText)findViewById(R.id.editTextLoginPw1);
        mEditPassword2 = (EditText)findViewById(R.id.editTextLoginPw2);
        mEditPassword3 = (EditText)findViewById(R.id.editTextLoginPw3);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParentLogin);
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
//        if( !mRescueMode ) {
//            //  Normal Login Mode
//            String strPassword = mEditPassword.getText().toString();
//
//            if( strPassword.equals("") ) {
//                ShowMessage("Please check password.");
//                return false;
//            }
//        }
//        else {
//            //  Rescue Login Mode
//            String strPassword1 = mEditPassword1.getText().toString();
//            String strPassword2 = mEditPassword2.getText().toString();
//            String strPassword3 = mEditPassword3.getText().toString();
//
//            if( strPassword1.equals("") ||
//                strPassword2.equals("") ||
//                strPassword3.equals("") ) {
//                ShowMessage("Please check password.");
//                return false;
//            }
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
