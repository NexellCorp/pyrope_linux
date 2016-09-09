package com.samsung.vd.cinemacontrolpanel;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends AppCompatActivity {
    private final String VD_DTAG = "DiagnosticsActivity";

    private VdTitleBar mTitleBar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_top);

        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutTop ));
        titleBar.SetTitle( "Cinema LED Display System Menu" );
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), LoginActivity.class);
                startActivity(intent);
            }
        });

        // Configuration Status Bar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutTop) );

        Button btnMenuDiagonostics = (Button)findViewById(R.id.btnMenuDiagonostics);
        Button btnMenuDisplayCheck = (Button)findViewById(R.id.btnMenuDisplayCheck);
        Button btnMenuDisplayMode = (Button)findViewById(R.id.btnMenuDisplayMode);
        Button btnMenuAccount = (Button)findViewById(R.id.btnMenuAccount);

        btnMenuDiagonostics.setOnClickListener(mClickListener);
        btnMenuDisplayCheck.setOnClickListener(mClickListener);
        btnMenuDisplayMode.setOnClickListener(mClickListener);
        btnMenuAccount.setOnClickListener(mClickListener);
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Intent intent;
            switch( ((Button)v).getId() ) {
                case R.id.btnMenuDiagonostics:
                    intent = new Intent(v.getContext(), DiagnosticsActivity.class);
                    startActivity(intent);
                    break;
                case R.id.btnMenuDisplayCheck:
                    intent = new Intent(v.getContext(), DisplayCheckActivity.class);
                    startActivity(intent);
                    break;
                case R.id.btnMenuDisplayMode:
                    intent = new Intent(v.getContext(), DisplayModeActivity.class);
                    startActivity(intent);
                    break;
                case R.id.btnMenuAccount:
                    intent = new Intent(v.getContext(), AccountActivity.class);
                    startActivity(intent);
                    break;
            }
        }
    };
}