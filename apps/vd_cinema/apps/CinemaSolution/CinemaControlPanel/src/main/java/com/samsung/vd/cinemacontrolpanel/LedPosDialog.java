package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
import android.content.Context;
import android.os.AsyncTask;
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
public class LedPosDialog extends Dialog {
    private static final String VD_DTAG = "LedPosDialog";

    public static final int TYPE_LED_OPEN_POS = 1;
    public static final int TYPE_LED_SHORT_POS = 2;

    private Context mContext;
    private int mType;
    private int mIndex;

    private AsyncTaskLedOpenPos mAsyncTaskLedOpenPos;
    private AsyncTaskLedShortPos mAsyncTaskLedShortPos;

    public LedPosDialog(Context context, int type, int index ) {
        super(context);

        mContext= context;
        mType   = type;
        mIndex  = index;
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
                if( mAsyncTaskLedOpenPos != null && mAsyncTaskLedOpenPos.getStatus() == AsyncTask.Status.RUNNING ) {
                    mAsyncTaskLedOpenPos.cancel( true );
                }

                if( mAsyncTaskLedShortPos != null && mAsyncTaskLedShortPos.getStatus() == AsyncTask.Status.RUNNING ) {
                    mAsyncTaskLedShortPos.cancel( true );
                }

                dismiss();
            }
        });

        ListView listView = (ListView)findViewById(R.id.listView_info_led);
        LedPosAdapter adapter = new LedPosAdapter(mContext.getApplicationContext(), R.layout.listview_row_info_led);
        listView.setAdapter(adapter);

        if( mType == TYPE_LED_OPEN_POS )
            mAsyncTaskLedOpenPos = new AsyncTaskLedOpenPos( adapter, mIndex );

        if( mType == TYPE_LED_SHORT_POS )
            mAsyncTaskLedShortPos = new AsyncTaskLedShortPos( adapter, mIndex );
    }

    private class AsyncTaskLedOpenPos extends AsyncTask<Void, Void, Void> {
        private LedPosAdapter mAdapter;
        private int mIndex;

        public AsyncTaskLedOpenPos( LedPosAdapter adapter, int index ) {
            mAdapter= adapter;
            mIndex  = index;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] resultNum = ctrl.Send(mIndex, NxCinemaCtrl.CMD_TCON_OPEN_NUM, null);
            if( resultNum == null || resultNum.length == 0)
                return null;

            for( int i = 0; i < ctrl.ByteArrayToInt( resultNum ); i++ ) {
                if( isCancelled() ) {
                    return null;
                }

                //
                //  Implementation LedOpenPos ( Not Tested )
                //
                mAdapter.add( new LedPosInfo(String.valueOf(i)) );

                byte[] resultPos = ctrl.Send(mIndex, NxCinemaCtrl.CMD_TCON_OPEN_POS, null);
                if( resultPos == null || resultPos.length == 0)
                    continue;

                int posX = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.FORMAT_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.FORMAT_INT16_LSB);

                LedPosInfo info = mAdapter.getItem(i);
                info.SetPosX( String.valueOf(posX) );
                info.SetPosY( String.valueOf(posY) );

                publishProgress();
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }

    private class AsyncTaskLedShortPos extends AsyncTask<Void, Void, Void> {
        private LedPosAdapter mAdapter;
        private int mIndex;

        public AsyncTaskLedShortPos( LedPosAdapter adapter, int index ) {
            mAdapter= adapter;
            mIndex  = index;
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] resultNum = ctrl.Send(mIndex, NxCinemaCtrl.CMD_TCON_SHORT_NUM, null);
            if( resultNum == null || resultNum.length == 0)
                return null;

            for( int i = 0; i < ctrl.ByteArrayToInt( resultNum ); i++ ) {
                if( isCancelled() ) {
                    return null;
                }

                //
                //  Implementation LedShortPos ( Not Tested )
                //
                mAdapter.add( new LedPosInfo(String.valueOf(i)) );

                byte[] resultPos = ctrl.Send(mIndex, NxCinemaCtrl.CMD_TCON_SHORT_POS, null);
                if( resultPos == null || resultPos.length == 0)
                    continue;

                int posX = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.FORMAT_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.FORMAT_INT16_LSB);

                LedPosInfo info = mAdapter.getItem(i);
                info.SetPosX( String.valueOf(posX) );
                info.SetPosY( String.valueOf(posY) );

                publishProgress();

            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }
    }
}
