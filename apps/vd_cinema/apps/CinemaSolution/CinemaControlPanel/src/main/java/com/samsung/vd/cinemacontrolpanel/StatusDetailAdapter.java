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

        TextView title;
        RadioButton radio1;
        RadioButton radio2;
        RadioButton radio3;
        TextView describe;
        Button button;
        Holder holder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            title = (TextView) convertView.findViewById(R.id.listview_row_status_detail_title);
            radio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio1);
            radio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio2);
            radio3 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio3);
            describe = (TextView)convertView.findViewById(R.id.listview_row_status_detail_text);
            button = (Button)convertView.findViewById(R.id.listview_row_status_detail_button);

            holder = new Holder();
            holder.mTitle = title;
            holder.mRadio1 = radio1;
            holder.mRadio2 = radio2;
            holder.mRadio3 = radio3;
            holder.mDescribe = describe;
            holder.mButton = button;

            convertView.setTag(holder);
        }
        else {
            holder = (Holder)convertView.getTag();

            title = holder.mTitle;
            radio1 = holder.mRadio1;
            radio2 = holder.mRadio2;
            radio3 = holder.mRadio3;
            describe = holder.mDescribe;
            button = holder.mButton;
        }

        title.setText( mData.get(position).GetTitle() );
        describe.setText( mData.get(position).GetDescription() );

        if( mData.get(position).GetStatus() == StatusDetailInfo.PASS ) {
            radio1.setChecked( true );
            radio2.setChecked( false );
            radio3.setChecked( false );
            button.setEnabled( false );
        }
        else if( mData.get(position).GetStatus() == StatusDetailInfo.FAIL ) {
            radio1.setChecked( false );
            radio2.setChecked( true );
            radio3.setChecked( false );
            button.setEnabled( true );
        }
        else {
            radio1.setChecked( false );
            radio2.setChecked( false );
            radio3.setChecked( true );
            button.setEnabled( false );
        }

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AsyncTaskLedPos(context, mData.get(pos).GetSlave(), mType).execute();
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

    @Override
    public void clear() {
        mData.clear();
        super.clear();
    }

    private class Holder {
        TextView    mTitle;
        RadioButton mRadio1;
        RadioButton mRadio2;
        RadioButton mRadio3;
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

//            byte[] cabinet = ((CinemaInfo)(context.getApplicationContext())).GetCabinet();
//            mId = cabinet[position];
            mId = (byte)(position & 0xFF);

            if( type == NxCinemaCtrl.CMD_TCON_OPEN_NUM || type == NxCinemaCtrl.CMD_TCON_OPEN_POS ) {
                mCmd1 = NxCinemaCtrl.CMD_TCON_OPEN_NUM;
                mCmd2 = NxCinemaCtrl.CMD_TCON_OPEN_POS;
            }

            if( type == NxCinemaCtrl.CMD_TCON_SHORT_NUM || type == NxCinemaCtrl.CMD_TCON_SHORT_POS ) {
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
            ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{mId} );

            byte[] resultNum = ctrl.Send( mCmd1, new byte[]{mId});
            if (resultNum == null || resultNum.length == 0)
                return null;

            int numOfValue = ctrl.ByteArrayToInt16(resultNum, NxCinemaCtrl.FORMAT_INT16);
            for( int i = 0; i < numOfValue; i++ ) {
                if (isCancelled()) {
                    return null;
                }

                byte[] resultPos = ctrl.Send( mCmd2, new byte[]{mId} );
                if (resultPos == null || resultPos.length == 0)
                    continue;

                int posX = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_LSB);
                mAdapter.add( new LedPosInfo(String.valueOf(i), String.valueOf(posX), String.valueOf(posY)));
            }

            ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{mId} );
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
