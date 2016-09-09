package com.samsung.vd.cinemacontrolpanel;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TabHost;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private final String[] INPUT_SOURCE = {
        "DCP (24Hz, 4K)",
        "HD-SDI (120Hz, 4K)",
        "HDMI (60Hz, 4K)",
    };

    private Spinner mSpinnerInputSource = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_mode);

        // Configuration TitleBar
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDisplayMode ));
        titleBar.SetTitle( "Cinema LED Display System Display Mode" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(v.getContext(), TopActivity.class);
                startActivity(intent);
            }
        });

        // Configuration StatusBar
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutDisplayMode) );

        mSpinnerInputSource = (Spinner)findViewById(R.id.spinnerInputSource);
        ArrayAdapter<String> adapterSpinInputSource = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, INPUT_SOURCE);
        mSpinnerInputSource.setAdapter(adapterSpinInputSource);

        AddTabs();
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Brightness");
        tabSpec1.setContent(R.id.tab_display_mode_brightness);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Input Source");
        tabSpec2.setContent(R.id.tab_display_mode_input);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Set up");
        tabSpec3.setContent(R.id.tab_display_mode_setup);

        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            Log.i(VD_DTAG, "Tab ID : " + tabId);
        }
    };
}