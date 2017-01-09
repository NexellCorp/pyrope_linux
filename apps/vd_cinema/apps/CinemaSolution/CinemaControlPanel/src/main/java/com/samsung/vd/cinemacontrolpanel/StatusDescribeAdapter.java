package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 11/14/16.
 */
public class StatusDescribeAdapter extends ArrayAdapter<StatusDescribeInfo> {
    private ArrayList<StatusDescribeInfo> mData;
    private int mResource;

    public StatusDescribeAdapter(Context context, int resource) {
        super(context, resource);
        mData = new ArrayList<>();
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public StatusDescribeInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();

        TextView mTitle;
        TextView mDescribe;
        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView) convertView.findViewById(R.id.listview_row_status_describe_title);
            mDescribe = (TextView)convertView.findViewById(R.id.listview_row_status_describe_text);

            mHolder = new Holder();
            mHolder.mTitle = mTitle;
            mHolder.mDescribe = mDescribe;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mDescribe = mHolder.mDescribe;
        }

        mTitle.setText( mData.get(position).GetTitle() );
        mDescribe.setText( mData.get(position).GetDescription() );

        return convertView;
    }

    @Override
    public void add(StatusDescribeInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(StatusDescribeInfo object) {
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
        TextView    mDescribe;
    }
}
