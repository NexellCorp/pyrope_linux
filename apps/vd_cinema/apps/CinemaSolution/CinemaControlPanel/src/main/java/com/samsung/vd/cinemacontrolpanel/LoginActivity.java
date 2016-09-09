package com.samsung.vd.cinemacontrolpanel;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
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

        ((Button)findViewById(R.id.btnLoginNormal)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginRescue).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginNormal).setVisibility(View.VISIBLE);
            }
        });

        ((Button)findViewById(R.id.btnLoginRescue)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginNormal).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginRescue).setVisibility(View.VISIBLE);
            }
        });

        ((Button)findViewById(R.id.btnLoginNormalAccept)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    Intent intent = new Intent(v.getContext(), TopActivity.class);
                    startActivity(intent);
                }
                else {
                    Toast.makeText(getApplicationContext(), "Invalid Account. Check your ID or Password.", Toast.LENGTH_SHORT).show();
                }
            }
        });

        ((Button)findViewById(R.id.btnLoginRescueAccept)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    Intent intent = new Intent(v.getContext(), TopActivity.class);
                    startActivity(intent);
                }
                else {
                    Toast.makeText(getApplicationContext(), "Invalid Account. Check your ID or Password.", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    private boolean CheckAccount() {
        return true;
    }
}
