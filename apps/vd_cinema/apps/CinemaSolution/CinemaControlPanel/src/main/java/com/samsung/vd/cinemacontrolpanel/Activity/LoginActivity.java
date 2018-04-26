package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.Spinner;

import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.AccountPreference;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_Login;

public class LoginActivity extends BaseActivity {
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

//        new AsyncTaskCheckCabinet().execute();
        NXAsync.getInstance().Execute(CMD_Login , asyncCallback);

//        if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
//            return;
//        }

        setContentView(R.layout.activity_login);

        //
        // Set Title Bar and Status Bar
        //
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Login" , null); //new Intent(v.getContext(), InitialActivity.class)

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
                    NXAsync.getInstance().Execute(CMD_Login , asyncCallback);
//                    startActivity( new Intent(getApplicationContext(), TopActivity.class) );
//                    overridePendingTransition(0, 0);
//                    finish();
                }
            }
        });

        Button btnAcceptRescue = (Button)findViewById(R.id.btnLoginRescueAccept);
        btnAcceptRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    NXAsync.getInstance().Execute(CMD_Login , asyncCallback);
//                    startActivity( new Intent(getApplicationContext(), TopActivity.class) );
//                    overridePendingTransition(0, 0);
//                    finish();
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
    // work for UI thread with IPC
    //
    NXAsync.AsyncCallback asyncCallback = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            LoginActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( LoginActivity.this );
                    ((CinemaInfo)getApplicationContext()).ClearCabinet();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            LoginActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    if(NXAsync.getInstance().isI2CFailed()) {
                        ShowMessage("I2C Failed.. try again later");
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                        return;
                    }

                    if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
                        startActivity( new Intent(getApplicationContext(), TopActivity.class) );
                        overridePendingTransition(0, 0);
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                        finish();
                    }else {
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                    }
                }
            });
        }
    };

} //LoginActivity class
