package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
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
public class LedPosDialog extends Dialog {
    private static final String VD_DTAG = "LedPosDialog";

    private Context mContext;
    private LedPosAdapter mAdapter;

    public LedPosDialog(Context context, LedPosAdapter adapter) {
        super(context);

        mContext = context;
        mAdapter = adapter;
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

        VdTitleBar titleBar = new VdTitleBar( mContext.getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Led Detail Information" );
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dismiss();
            }
        });

        ((ListView)findViewById(R.id.listView_info_led)).setAdapter(mAdapter);
    }
}
