package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 12/16/16.
 */

public class CheckRunAdapter extends ArrayAdapter<CheckRunInfo> {
    private ArrayList<CheckRunInfo> mData;
    private Context mContext;
    private int mResource;

    public CheckRunAdapter(Context context, int resource) {
        super(context, resource);
        mContext = context;
        mData = new ArrayList<>();
        mResource = resource;
    }

    public ArrayList get() { return mData; };

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public CheckRunInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();
        final int pos = position;

        CheckBox mCheckBox;
        TextView mDescribe;
        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mCheckBox = (CheckBox) convertView.findViewById(R.id.listview_check_run_checkbox);
            mDescribe = (TextView) convertView.findViewById(R.id.listview_check_run_text);

            mHolder = new Holder();
            mHolder.mCheckBox = mCheckBox;
            mHolder.mDescribe = mDescribe;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();
            mCheckBox = mHolder.mCheckBox;
            mDescribe = mHolder.mDescribe;
        }

        mCheckBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mData.get(pos).SetChecked(((CheckBox)view).isChecked());
            }
        });

        mCheckBox.setText( mData.get(pos).GetTitle() );
        mCheckBox.setChecked( mData.get(pos).GetChecked() );
        mDescribe.setText( mData.get(pos).GetDescription() );

        return convertView;
    }

    @Override
    public void add(CheckRunInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(CheckRunInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    @Override
    public void clear() {
        mData.clear();
        super.clear();
    }

    private class Holder {
        CheckBox    mCheckBox;
        TextView    mDescribe;
    }
}
