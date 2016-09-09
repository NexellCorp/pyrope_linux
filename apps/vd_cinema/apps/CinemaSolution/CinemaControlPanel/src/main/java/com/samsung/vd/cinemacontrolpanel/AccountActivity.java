package com.samsung.vd.cinemacontrolpanel;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/18/16.
 */
public class AccountActivity extends AppCompatActivity {
    private static final String VD_DTAG = "AccountActivity";

    private ListView mListViewAccount;
    private AccountAdapter mAdapterAccount;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_account);

        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutAccount ));
        titleBar.SetTitle( "Cinema LED Display System Account" );

        // Configuration Status Bar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutAccount) );

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

        mListViewAccount = (ListView)findViewById(R.id.listview_account);
        mAdapterAccount = new AccountAdapter(this, R.layout.listview_row_account);
        mListViewAccount.setAdapter(mAdapterAccount);

        mAdapterAccount.add( "Service" );
        mAdapterAccount.add( "Service" );
        mAdapterAccount.add( "Service" );
        mAdapterAccount.add( "Operator" );
        mAdapterAccount.add( "Operator" );
        mAdapterAccount.add( "Operator" );
        mAdapterAccount.add( "Calibrator" );
        mAdapterAccount.add( "Calibrator" );
        mAdapterAccount.add( "Calibrator" );
    }
}
