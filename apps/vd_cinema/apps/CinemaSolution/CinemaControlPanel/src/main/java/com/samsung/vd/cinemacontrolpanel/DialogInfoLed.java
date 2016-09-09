package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.samsung.vd.baseutils.VdTitleBar;

/**
 * Created by doriya on 8/19/16.
 */
public class DialogInfoLed extends Dialog {
    private Context mContext;

    private ListView mListView;
    private View mListHeader;

    private Button mButtonConfirm;
    private InfoLedAdapter mAdapter;

    public DialogInfoLed(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        WindowManager.LayoutParams lpWindow = new WindowManager.LayoutParams();
        lpWindow.flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
        lpWindow.dimAmount = 0.8f;
        getWindow().setAttributes(lpWindow);
        setContentView(R.layout.dialog_info_led);

        VdTitleBar titleBar = new VdTitleBar( mContext.getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutDialog ));
        titleBar.SetTitle( "Led Detail Information" );
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dismiss();
            }
        });

        mListView = (ListView)findViewById(R.id.listView_info_led);
        mAdapter = new InfoLedAdapter(mContext.getApplicationContext(), R.layout.listview_row_info_led);
        mListView.setAdapter(mAdapter);

        for( int i = 0 ;i < 30; i++ )
        {
            mAdapter.add(String.valueOf(i + 1));
        }
    }
}
