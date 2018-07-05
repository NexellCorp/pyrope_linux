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
    private static final String VD_DTAG = "StatusDetailAdapter";
    private ArrayList<StatusDetailInfo> mData;
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
    public StatusDetailInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();
        final int pos = position;

        TextView title;
        RadioButton radio1;
        RadioButton radio2;
        RadioButton radio3;
        TextView describe;
        Button button;
        Holder holder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            title = (TextView) convertView.findViewById(R.id.listview_row_status_detail_title);
            radio1 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio1);
            radio2 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio2);
            radio3 = (RadioButton) convertView.findViewById(R.id.listview_row_status_detail_radio3);
            describe = (TextView)convertView.findViewById(R.id.listview_row_status_detail_text);
            button = (Button)convertView.findViewById(R.id.listview_row_status_detail_button);

            holder = new Holder();
            holder.mTitle = title;
            holder.mRadio1 = radio1;
            holder.mRadio2 = radio2;
            holder.mRadio3 = radio3;
            holder.mDescribe = describe;
            holder.mButton = button;

            convertView.setTag(holder);
        }
        else {
            holder = (Holder)convertView.getTag();

            title = holder.mTitle;
            radio1 = holder.mRadio1;
            radio2 = holder.mRadio2;
            radio3 = holder.mRadio3;
            describe = holder.mDescribe;
            button = holder.mButton;
        }

        title.setText( mData.get(position).GetTitle() );
        describe.setText( mData.get(position).GetDescription() );

        if( mData.get(position).GetStatus() == StatusDetailInfo.PASS ) {
            radio1.setChecked( true );
            radio2.setChecked( false );
            radio3.setChecked( false );
            button.setEnabled( false );
        }
        else if( mData.get(position).GetStatus() == StatusDetailInfo.FAIL ) {
            radio1.setChecked( false );
            radio2.setChecked( true );
            radio3.setChecked( false );
            button.setEnabled( true );
        }
        else {
            radio1.setChecked( false );
            radio2.setChecked( false );
            radio3.setChecked( true );
            button.setEnabled( false );
        }

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final LedPosAdapter adapter = new LedPosAdapter(context.getApplicationContext(), R.layout.listview_row_info_led);

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_LED_OPEN_POS,
                        context,
                        mData.get(pos).GetSlave(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                CinemaLoading.Show( context );
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                new LedPosDialog(context, adapter).show();
                                CinemaLoading.Hide();
                            }
                        },
                        new CinemaTask.ProgressUpdateCallback() {
                            @Override
                            public void onProgressUpdate(Object[] values) {
                                adapter.add( new LedPosInfo(String.valueOf(values[0]), String.valueOf(values[1]), String.valueOf(values[2])) );
                                adapter.notifyDataSetChanged();
                            }
                        }
                );
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
        TextView    mDescribe;
        Button      mButton;
    }
}
