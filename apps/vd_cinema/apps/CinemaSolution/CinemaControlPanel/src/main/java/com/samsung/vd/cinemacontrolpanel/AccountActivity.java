package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
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
public class AccountActivity extends CinemaBaseActivity {
    private static final String VD_DTAG = "AccountActivity";

    private static final int MAX_ACCOUNT_GROUP_NUM = 3;

    private CinemaInfo mCinemaInfo;

    private LayoutInflater mInflater;
    private LinearLayout mParentLayout;

    private LinearLayout[] mAccountServiceLayout;
    private LinearLayout[] mAccountCalibratorLayout;
    private LinearLayout[] mAccountOperatorLayout;

    private AccountPreference mAccountPreference;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_account);

        //
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();

        //
        //  Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Account" );
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
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TurnOff();
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
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());

        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayout = (LinearLayout)findViewById(R.id.layoutAccount);

        //
        //
        //
        mAccountServiceLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountServiceLayout.length; i++ ) {
            mAccountServiceLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_item_account, mParentLayout, false );
            AddViewAccount( mAccountServiceLayout[i], AccountPreference.GROUP_SERVICE, i );
        }

        mAccountCalibratorLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountCalibratorLayout.length; i++ ) {
            mAccountCalibratorLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_item_account, mParentLayout, false );
            AddViewAccount( mAccountCalibratorLayout[i], AccountPreference.GROUP_CALIBRATOR, i );
        }

        mAccountOperatorLayout = new LinearLayout[MAX_ACCOUNT_GROUP_NUM];
        for( int i = 0; i < mAccountOperatorLayout.length; i++ ) {
            mAccountOperatorLayout[i] = (LinearLayout) mInflater.inflate(R.layout.layout_item_account, mParentLayout, false );
            AddViewAccount( mAccountOperatorLayout[i], AccountPreference.GROUP_OPERATOR, i );
        }
    }

    private void AddViewAccount( View childView, String strGroup, int index ) {
        mParentLayout.addView(childView);

        final int accountIndex = index;
        final String accountGroup = strGroup;

        final TextView textGroup = (TextView)childView.findViewById(R.id.textAccountGroup);
        textGroup.setText( accountGroup );

        final EditText editId = (EditText)childView.findViewById(R.id.editAccountId);
        final EditText editPw = (EditText)childView.findViewById(R.id.editAccountPw);
        final EditText editConfirm = (EditText)childView.findViewById(R.id.editAccountConfirm);

        String strId = mAccountPreference.ReadId(strGroup, accountIndex);
        String strPw = mAccountPreference.ReadPw(strGroup, accountIndex);

        editId.setText(strId);
        editPw.setText(strPw);

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

                    if( editPw.getText().toString().equals("") ||
                        !editPw.getText().toString().equals( editConfirm.getText().toString() ) ) {
                        ShowMessage( "Please check password." );

                        String strId = mAccountPreference.ReadId(accountGroup, accountIndex);
                        String strPw = mAccountPreference.ReadPw(accountGroup, accountIndex);

                        editId.setText(strId);
                        editPw.setText(strPw);
                    }
                    else {
                        ShowMessage( "Update password." );
                        mAccountPreference.Add( accountGroup, accountIndex, editId.getText().toString(), editPw.getText().toString() );

                        String strId = mAccountPreference.ReadId(accountGroup, accountIndex);
                        String strLog = String.format("Update account. ( %s, %s )", accountGroup, strId );
                        mCinemaInfo.InsertLog( strLog );
                    }

                    editConfirm.setText("");
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

    private boolean ConfirmPassword( String strPw, String strConfirm ) {
        return !(strPw == null || strConfirm == null || strPw.equals("") || strConfirm.equals(""))
                && strPw.equals(strConfirm);
    }
}
