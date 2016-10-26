package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/18/16.
 */
public class AccountActivity extends AppCompatActivity {
    private static final String VD_DTAG = "AccountActivity";

    private static final int MAX_ACCOUNT_GROUP_NUM = 3;

    private LayoutInflater mInflater ;
    private LinearLayout mParentLayout;

    private LinearLayout[] mAccountServiceLayout;
    private LinearLayout[] mAccountCalibratorLayout;
    private LinearLayout[] mAccountOperatorLayout;

    private AccountPreference mAccountPreference;

    private Toast mToast;

    @Override
    protected void onResume() {
        super.onResume();

        mAccountServiceLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountServiceLayout.length; i++ ) {
            mAccountServiceLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_account, mParentLayout, false );
            AddViewAccount( mAccountServiceLayout[i], AccountPreference.GROUP_SERVICE, i );
        }

        mAccountCalibratorLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountCalibratorLayout.length; i++ ) {
            mAccountCalibratorLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_account, mParentLayout, false );
            AddViewAccount( mAccountCalibratorLayout[i], AccountPreference.GROUP_CALIBRATOR, i );
        }


        mAccountOperatorLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountOperatorLayout.length; i++ ) {
            mAccountOperatorLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_account, mParentLayout, false );
            AddViewAccount( mAccountOperatorLayout[i], AccountPreference.GROUP_OPERATOR, i );
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

//        for( LinearLayout layout : mAccountServiceLayout )
//            RemoveViewAccount(layout);
//
//        for( LinearLayout layout : mAccountCalibratorLayout )
//            RemoveViewAccount(layout);
//
//        for( LinearLayout layout : mAccountOperatorLayout )
//            RemoveViewAccount(layout);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_account);

        //
        //  Set TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutAccount ));
        titleBar.SetTitle( "Cinema LED Display System Account" );

        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), TopActivity.class);
                startActivity(intent);
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), LoginActivity.class);
                startActivity(intent);
            }
        });

        //
        //  Set StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutAccount) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());

        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayout = (LinearLayout)findViewById(R.id.layoutAccount);
    }

    private void AddViewAccount( View childView, String strGroup, int index ) {
        mParentLayout.addView(childView);

        TextView textGroup = (TextView)childView.findViewById(R.id.textAccountGroup);
        textGroup.setText( strGroup );

        final EditText editId = (EditText)childView.findViewById(R.id.editAccountId);
        final EditText editPw = (EditText)childView.findViewById(R.id.editAccountPw);
        final EditText editConfirm = (EditText)childView.findViewById(R.id.editAccountConfirm);

        String strId = mAccountPreference.ReadId(strGroup, index);
        String strPw = mAccountPreference.ReadPw(strGroup, index);

        if ( null != strId ) {
            editId.setText(strId);
        }
        if ( null != strPw ) {
            editPw.setText(strPw);
            editConfirm.setText(strPw);
        }

        Button btnTemp = (Button)childView.findViewById(R.id.btnAccountModify);
        btnTemp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( ((Button)v).getText().toString().equalsIgnoreCase("Modify") ) {
                    ((Button)v).setText(R.string.txt_done);
                    EnableEditText(editId, true);
                    EnableEditText(editPw, true);
                    EnableEditText(editConfirm, true);
                }
                else {
                    ((Button)v).setText(R.string.txt_modify);
                    EnableEditText(editId, false);
                    EnableEditText(editPw, false);
                    EnableEditText(editConfirm, false);

                    if( !ConfirmPassword( editPw.getText().toString(), editConfirm.getText().toString() ) ) {
                        ShowMessage( "Please check password." );
                    }
                    else {
                        ShowMessage( "Update password." );
                    }
                }
            }
        });
    }

    private void RemoveViewAccount( View childView ) {
        mParentLayout.removeView(childView);
    }

    private void EnableEditText(EditText id, boolean enable ) {
        id.setClickable(enable);
        id.setEnabled(enable);
        id.setFocusable(enable);
        id.setFocusableInTouchMode(enable);
    }

    private boolean ConfirmPassword( String strPw, String strConfirm )
    {
        return !(strPw == null || strConfirm == null || strPw.equals("") || strConfirm.equals(""))
                && strPw.equals(strConfirm);
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
