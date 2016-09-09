package com.samsung.vd.cinemacontrolpanel;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayCheckActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayCheckActivity";
    private static final String[] TEST_PATTERN = {
            "Pattern #1", "Pattern #2", "Pattern #3", "Pattern #4",
    };
    private ListView mListViewCabinetDisplay;
    private StatusSimpleAdapter mAdapterCabinetDisplay;
    private Spinner mSpinnerTestPattern;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_check);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDisplayCheck ));
        titleBar.SetTitle( "Cinema LED Display System Display Check" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), TopActivity.class);
                startActivity(intent);
            }
        });

        // Configuration StatusBar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutDisplayCheck) );

        AddTabs();

        mListViewCabinetDisplay = (ListView)findViewById(R.id.listView_cabinet_id);
        mAdapterCabinetDisplay = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        mListViewCabinetDisplay.setAdapter( mAdapterCabinetDisplay );

        for(int i = 0; i < 69; i++ )
        {
            mAdapterCabinetDisplay.add( "Cabinet " + String.valueOf(i + 1) );
        }

        mSpinnerTestPattern = (Spinner)findViewById(R.id.spinnerTestPattern);
        ArrayAdapter<String> adapterTestPattern = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, TEST_PATTERN);
        mSpinnerTestPattern.setAdapter(adapterTestPattern);

        ((Button)findViewById(R.id.btnPlayTestPattern)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(getApplicationContext(), "Run Test Pattern.", Toast.LENGTH_SHORT).show();
                //
                //
                //
            }
        });
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Calibration Pattern");
        tabSpec1.setContent(R.id.tab_display_check_calibration);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Test Pattern");
        tabSpec2.setContent(R.id.tab_display_check_test);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Cabinet Status");
        tabSpec3.setContent(R.id.tab_display_check_cabinet);

        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec("TAB4");
        tabSpec4.setIndicator("Led Accumulation time");
        tabSpec4.setContent(R.id.tab_display_check_led);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);
        tabHost.addTab(tabSpec4);

        tabHost.setOnTabChangedListener(mDiagnosticsTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mDiagnosticsTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            Log.i(VD_DTAG, "Tab ID : " + tabId);
        }
    };
}