package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 11/25/16.
 */

public class SelectRunAdapter extends ArrayAdapter<SelectRunInfo> {
    private ArrayList<SelectRunInfo> mData;
    private Context mContext;
    private int mResource;

    public SelectRunAdapter(Context context, int resource) {
        super(context, resource);
        mContext = context;
        mData = new ArrayList<>();
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public SelectRunInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();
        final int mPosition = position;

        final TextView  mTitle;
        final Spinner   mSpinner;
        final Button    mButton;

        Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView) convertView.findViewById(R.id.listview_row_select_run_title);
            mSpinner = (Spinner)convertView.findViewById(R.id.listview_row_select_run_spinner);
            mButton = (Button)convertView.findViewById(R.id.listview_row_select_run_button);

            mHolder = new SelectRunAdapter.Holder();
            mHolder.mTitle = mTitle;
            mHolder.mSpinner = mSpinner;
            mHolder.mButton = mButton;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (SelectRunAdapter.Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mSpinner = mHolder.mSpinner;
            mButton = mHolder.mButton;
        }

        mTitle.setText( mData.get(position).GetTitle() );

        if(0 == mData.get(position).GetSpinnerItem().length) {
            mSpinner.setAdapter( new ArrayAdapter<>(mContext, android.R.layout.simple_spinner_dropdown_item, new String[] {""}));
            mSpinner.setEnabled(false);
        }
        else {
            mSpinner.setAdapter( new ArrayAdapter<>(mContext, android.R.layout.simple_spinner_dropdown_item, mData.get(position).GetSpinnerItem()));
            mSpinner.setEnabled(true);
        }

        mButton.setText( !mData.get(mPosition).GetStatus() ? R.string.txt_run : R.string.txt_stop );
        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SelectRunAdapter.OnClickListener listener = mData.get(mPosition).GetOnClickListener();
                if( listener != null ) {
                    boolean status = mData.get(mPosition).GetStatus();
                    if( mData.get(mPosition).GetIsToggle() ) {
                        status = !status;
                        mButton.setText(status ? R.string.txt_stop : R.string.txt_run);
                        mData.get(mPosition).SetStatus( status );
                    }

                    listener.onClickListener( mPosition, mSpinner.getSelectedItemPosition(), !mData.get(mPosition).GetIsToggle() || status );
                }
            }
        });

        return convertView;
    }

    public interface OnClickListener {
        void onClickListener( int index, int spinnerIndex, boolean status );
    }

    @Override
    public void add(SelectRunInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(SelectRunInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    public void reset(int position) {
        mData.get(position).SetStatus(false);
        notifyDataSetChanged();
    }

    private class Holder {
        TextView    mTitle;
        Spinner     mSpinner;
        Button      mButton;
    }
}
