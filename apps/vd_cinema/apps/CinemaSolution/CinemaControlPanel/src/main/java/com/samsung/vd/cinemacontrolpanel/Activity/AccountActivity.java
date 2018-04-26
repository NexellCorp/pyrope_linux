package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.AccountPreference;

/**
 * Created by doriya on 8/18/16.
 */
public class AccountActivity extends BaseActivity {
    private static final String VD_DTAG = "AccountActivity";

    private static final int MAX_ACCOUNT_GROUP_NUM = 3;

    private LayoutInflater mInflater;
    private LinearLayout mParentLayout;

    private LinearLayout[] mAccountServiceLayout;
    private LinearLayout[] mAccountCalibratorLayout;
    private LinearLayout[] mAccountOperatorLayout;

    private AccountPreference mAccountPreference;

    @Override
    protected void onResume() {
        super.onResume();

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
        // Set Title Bar and Status Bar
        //
        setCommonUI(R.id.layoutTitleBar, R.id.layoutStatusBar , "Cinema LED Display System - Account" , new Intent(getApplicationContext(), TopActivity.class) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());

        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayout = (LinearLayout)findViewById(R.id.layoutAccount);
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
                        ((CinemaInfo)getApplicationContext()).InsertLog( strLog );
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

    private boolean ConfirmPassword( String strPw, String strConfirm )
    {
        return !(strPw == null || strConfirm == null || strPw.equals("") || strConfirm.equals(""))
                && strPw.equals(strConfirm);
    }


}
