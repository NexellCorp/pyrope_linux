package com.samsung.vd.cinemadiagnostics;

import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TabHost;

import com.samsung.vd.baseutils.VdTitleBar;

//
// Launch for this application.
// am start -a android.intent.action.MAIN -n com.samsung.vd.cinemadiagnostics/com.samsung.vd.cinemadiagnostics.DiagnosticsActivity
// am start -a android.intent.action.MAIN -n com.samsung.vd.cinemacontrolpanel/com.samsung.vd.cinemacontrolpanel.InitialActivity
// am force-stop com.samsung.vd.cinemacontrolpanel
// am kill com.samsung.vd.cinemacontrolpanel
// pm disable com.samsung.vd.cinemacontrolpanel

// pm enable com.samsung.vd.cinemacontrolpanel; am start -a android.intent.action.MAIN -n com.samsung.vd.cinemacontrolpanel/com.samsung.vd.cinemacontrolpanel.InitialActivity
// am start -a android.intent.action.MAIN -n com.samsung.vd.cinemacontrolpanel/com.samsung.vd.cinemacontrolpanel.InitialActivity
//
public class DiagnosticsActivity extends AppCompatActivity {
    private static final String NX_DTAG = "DiagnosticsActivity";

    private TabHost mTabHost;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_diagnostics);

        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema Diagnostics for Product" );

        titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 1. Finish this activity as well as all activities immediately below it in the current task that have the same affinity.
                finishAffinity();

                // 2. Runs the finalization methods of any objects pending finalization.
                System.runFinalization();

                // 3. Terminates the currently running Java Virtual Machine.
                System.exit(0);
            }
        });
    }
}
