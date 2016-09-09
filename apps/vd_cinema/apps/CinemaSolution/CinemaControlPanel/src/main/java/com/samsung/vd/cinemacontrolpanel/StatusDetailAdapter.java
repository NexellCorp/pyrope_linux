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
public class StatusDetailAdapter extends ArrayAdapter<String> {
    private ArrayList<String> mData;
    private int mResource;

    public StatusDetailAdapter (Context context, int resource) {
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

        TextView mText;
        RadioButton mRadio1;
        RadioButton mRadio2;
        TextView mDescribe;
        Button mButton;

        Holder mHolder = null;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mText = (TextView) convertView.findViewById(R.id.listview_row_status_detail_text);
            mRadio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio1);
            mRadio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio2);
            mDescribe = (TextView)convertView.findViewById(R.id.listview_row_status_detail_describe);
            mButton = (Button)convertView.findViewById(R.id.listview_row_status_detail_button);

            mHolder = new Holder();
            mHolder.mText = mText;
            mHolder.mRadio1 = mRadio1;
            mHolder.mRadio2 = mRadio2;
            mHolder.mDescribe = mDescribe;
            mHolder.mButton = mButton;

            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mText = mHolder.mText;
            mRadio1 = mHolder.mRadio1;
            mRadio2 = mHolder.mRadio2;
            mDescribe = mHolder.mDescribe;
            mButton = mHolder.mButton;
        }

        mText.setText( mData.get(position) );

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new DialogInfoLed(context).show();
            }
        });

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
        TextView mDescribe;
        Button mButton;
    }
}
