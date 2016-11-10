package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;
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
public class StatusSimpleAdapter extends ArrayAdapter<StatusSimpleInfo> {
    private ArrayList<StatusSimpleInfo> mData;
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
    public StatusSimpleInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();

        TextView mId = null;
        RadioButton mRadio1 = null;
        RadioButton mRadio2 = null;
        Holder mHolder = null;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mId = (TextView) convertView.findViewById(R.id.listview_row_status_simple_text);
            mRadio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio1);
            mRadio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio2);

            mHolder = new Holder();
            mHolder.mText = mId;
            mHolder.mRadio1 = mRadio1;
            mHolder.mRadio2 = mRadio2;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mId = mHolder.mText;
            mRadio1 = mHolder.mRadio1;
            mRadio2 = mHolder.mRadio2;
        }

        mId.setText( mData.get(position).GetId() );

        if( mData.get(position).GetStatus() == 1 ) {
            mRadio1.setChecked( true );
            mRadio2.setChecked( false );
        }
        else {
            mRadio1.setChecked( false );
            mRadio2.setChecked( true );
        }

        return convertView;
    }

    @Override
    public void add(StatusSimpleInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(StatusSimpleInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    private class Holder {
        TextView mText;
        RadioButton mRadio1;
        RadioButton mRadio2;
    }
}
