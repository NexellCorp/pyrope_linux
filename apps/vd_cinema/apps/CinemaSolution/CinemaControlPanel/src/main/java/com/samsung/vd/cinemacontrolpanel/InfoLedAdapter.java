package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 8/19/16.
 */
public class InfoLedAdapter extends ArrayAdapter<String> {
    private ArrayList<String> mStrNumber;
    private int mResource;

    public InfoLedAdapter(Context context, int resource) {
        super(context, resource);
        mStrNumber = new ArrayList<>();
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mStrNumber.size();
    }

    @Override
    public String getItem(int position) {
        return mStrNumber.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();

        TextView mTextNum;
        TextView mTextX;
        TextView mTextY;
        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTextNum = (TextView)convertView.findViewById(R.id.listview_row_info_led_num);
            mTextX = (TextView)convertView.findViewById(R.id.listview_row_info_led_x);
            mTextY = (TextView)convertView.findViewById(R.id.listview_row_info_led_y);

            mHolder = new Holder();
            mHolder.mTextNum = mTextNum;
            mHolder.mTextX = mTextX;
            mHolder.mTextY = mTextY;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mTextNum = mHolder.mTextNum;
            mTextX = mHolder.mTextX;
            mTextY = mHolder.mTextY;
        }

        mTextNum.setText( mStrNumber.get(position) );
        return convertView;
    }

    @Override
    public void add(String object) {
        mStrNumber.add(object);
        super.add(object);
    }

    @Override
    public void remove(String object) {
        mStrNumber.remove(object);
        super.remove(object);
    }

    private class Holder {
        TextView mTextNum;
        TextView mTextX;
        TextView mTextY;
    }
}
