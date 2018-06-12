package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.RadioButton;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 11/11/16.
 */
public class LedPosAdapter extends ArrayAdapter<LedPosInfo> {
    private ArrayList<LedPosInfo> mData;
    private int mResource;

    public LedPosAdapter(Context context, int resource ) {
        super(context, resource);
        mData = new ArrayList<>();
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public LedPosInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();

        TextView mTextId;
        TextView mTextX;
        TextView mTextY;
        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTextId = (TextView)convertView.findViewById(R.id.listview_row_info_led_num);
            mTextX = (TextView)convertView.findViewById(R.id.listview_row_info_led_x);
            mTextY = (TextView)convertView.findViewById(R.id.listview_row_info_led_y);

            mHolder = new Holder();
            mHolder.mTextId = mTextId;
            mHolder.mTextX = mTextX;
            mHolder.mTextY = mTextY;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mTextId = mHolder.mTextId;
            mTextX = mHolder.mTextX;
            mTextY = mHolder.mTextY;
        }

        mTextId.setText( mData.get(position).GetId() );
        mTextX.setText( mData.get(position).GetPosX() );
        mTextY.setText( mData.get(position).GetPosY() );

        return convertView;
    }

    @Override
    public void add(LedPosInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(LedPosInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    private class Holder {
        TextView mTextId;
        TextView mTextX;
        TextView mTextY;
    }
}
