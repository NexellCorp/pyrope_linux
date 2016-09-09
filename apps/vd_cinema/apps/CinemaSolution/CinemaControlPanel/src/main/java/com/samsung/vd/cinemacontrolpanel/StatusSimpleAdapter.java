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
 * Created by doriya on 8/19/16.
 */
public class StatusSimpleAdapter extends ArrayAdapter<String> {
    private ArrayList<String> mData;
    private int mResource;

    public StatusSimpleAdapter (Context context, int resource) {
        super(context, resource);
        mData = new ArrayList<>();
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public String getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();

        TextView mText = null;
        RadioButton mRadio1 = null;
        RadioButton mRadio2 = null;
        Holder mHolder = null;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mText = (TextView) convertView.findViewById(R.id.listview_row_status_simple_text);
            mRadio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio1);
            mRadio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio2);

            mHolder = new Holder();
            mHolder.mText = mText;
            mHolder.mRadio1 = mRadio1;
            mHolder.mRadio2 = mRadio2;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mText = mHolder.mText;
            mRadio1 = mHolder.mRadio1;
            mRadio2 = mHolder.mRadio2;

        }

        mText.setText( mData.get(position) );
        return convertView;
    }

    @Override
    public void add(String object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(String object) {
        mData.remove(object);
        super.remove(object);
    }

    private class Holder {
        TextView mText;
        RadioButton mRadio1;
        RadioButton mRadio2;
    }
}
