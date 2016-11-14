package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
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
        }
        else {
            mRadio1.setChecked( false );
            mRadio2.setChecked( true );
        }

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new LedPosDialog(context, mType, pos ).show();
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
}
