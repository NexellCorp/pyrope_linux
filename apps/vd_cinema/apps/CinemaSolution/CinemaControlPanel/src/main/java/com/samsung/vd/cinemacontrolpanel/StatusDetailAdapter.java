package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 8/19/16.
 */
public class StatusDetailAdapter extends ArrayAdapter<StatusDetailInfo> {
    private ArrayList<StatusDetailInfo> mData;
    private int mResource;
    private int mType;

    public StatusDetailAdapter (Context context, int resource, int type) {
        super(context, resource);
        mData = new ArrayList<>();
        mResource = resource;
        mType = type;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public StatusDetailInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();
        final int pos = position;

        TextView mTitle;
        RadioButton mRadio1;
        RadioButton mRadio2;
        TextView mDescribe;
        Button mButton;
        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView) convertView.findViewById(R.id.listview_row_status_detail_title);
            mRadio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio1);
            mRadio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio2);
            mDescribe = (TextView)convertView.findViewById(R.id.listview_row_status_detail_text);
            mButton = (Button)convertView.findViewById(R.id.listview_row_status_detail_button);

            mHolder = new Holder();
            mHolder.mTitle = mTitle;
            mHolder.mRadio1 = mRadio1;
            mHolder.mRadio2 = mRadio2;
            mHolder.mDescribe = mDescribe;
            mHolder.mButton = mButton;

            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mRadio1 = mHolder.mRadio1;
            mRadio2 = mHolder.mRadio2;
            mDescribe = mHolder.mDescribe;
            mButton = mHolder.mButton;
        }

        mTitle.setText( mData.get(position).GetTitle() );
        mDescribe.setText( mData.get(position).GetDescription() );

        if( mData.get(position).GetStatus() == 1 ) {
            mRadio1.setChecked( true );
            mRadio2.setChecked( false );
            mButton.setEnabled( false );
        }
        else {
            mRadio1.setChecked( false );
            mRadio2.setChecked( true );
            mButton.setEnabled( true );
        }

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AsyncTaskLedPos(context, pos, mType).execute();
            }
        });

        return convertView;
    }

    @Override
    public void add(StatusDetailInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(StatusDetailInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    private class Holder {
        TextView    mTitle;
        RadioButton mRadio1;
        RadioButton mRadio2;
        TextView    mDescribe;
        Button      mButton;
    }

    private class AsyncTaskLedPos extends AsyncTask<Void, Void, Void> {
        private Context mContext;
        private LedPosAdapter mAdapter;

        private byte mId;
        private int mCmd1;
        private int mCmd2;

        public AsyncTaskLedPos( Context context, int position, int type ) {
            mContext = context;
            mAdapter = new LedPosAdapter(mContext.getApplicationContext(), R.layout.listview_row_info_led);

            byte[] cabinet = ((CinemaInfo)(context.getApplicationContext())).GetCabinet();
            mId = cabinet[position];

            if( mType == NxCinemaCtrl.CMD_TCON_OPEN_NUM || mType == NxCinemaCtrl.CMD_TCON_OPEN_POS ) {
                mCmd1 = NxCinemaCtrl.CMD_TCON_OPEN_NUM;
                mCmd2 = NxCinemaCtrl.CMD_TCON_OPEN_POS;
            }

            if( mType == NxCinemaCtrl.CMD_TCON_SHORT_NUM || mType == NxCinemaCtrl.CMD_TCON_SHORT_POS ) {
                mCmd1 = NxCinemaCtrl.CMD_TCON_SHORT_NUM;
                mCmd2 = NxCinemaCtrl.CMD_TCON_SHORT_POS;
            }
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            CinemaLoading.Show(mContext);
        }

        @Override
        protected Void doInBackground(Void... params) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] resultNum;
            resultNum = ctrl.Send( mId, mCmd1, null);
            if (resultNum == null || resultNum.length == 0)
                return null;

            int numOfValue = ctrl.ByteArrayToInt16(resultNum, NxCinemaCtrl.FORMAT_INT16);
            for( int i = 0; i < numOfValue; i++ ) {
                if (isCancelled()) {
                    return null;
                }

                byte[] resultPos = ctrl.Send( mId, mCmd2, null );
                if (resultPos == null || resultPos.length == 0)
                    continue;

                int posX = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_LSB);
                mAdapter.add( new LedPosInfo(String.valueOf(i), String.valueOf(posX), String.valueOf(posY)));


                Log.i("", String.format("%d, %d", posX, posY));
                ctrl.PrintByteArrayToHex(resultPos);
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            new LedPosDialog(mContext, mAdapter).show();
            CinemaLoading.Hide();
        }
    }
}
