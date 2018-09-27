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

        TextView title;
        RadioButton radio1;
        RadioButton radio2;
        RadioButton radio3;
        Holder holder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            title = (TextView) convertView.findViewById(R.id.listview_row_status_simple_title);
            radio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio1);
            radio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio2);
            radio3 = (RadioButton) convertView.findViewById(R.id.listview_row_status_simple_radio3);

            holder = new Holder();
            holder.mTitle = title;
            holder.mRadio1 = radio1;
            holder.mRadio2 = radio2;
            holder.mRadio3 = radio3;
            convertView.setTag(holder);
        }
        else {
            holder = (Holder)convertView.getTag();

            title = holder.mTitle;
            radio1 = holder.mRadio1;
            radio2 = holder.mRadio2;
            radio3 = holder.mRadio3;
        }

        title.setText( mData.get(position).GetTitle() );

        if( mData.get(position).GetStatus() == StatusSimpleInfo.PASS ) {
            radio1.setChecked( true );
            radio2.setChecked( false );
            radio3.setChecked( false );
        }
        else if( mData.get(position).GetStatus() == StatusSimpleInfo.FAIL ) {
            radio1.setChecked( false );
            radio2.setChecked( true );
            radio3.setChecked( false );
        }
        else {
            radio1.setChecked( false );
            radio2.setChecked( false );
            radio3.setChecked( true );
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
    }
}
