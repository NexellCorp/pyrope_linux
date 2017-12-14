package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 10/20/17.
 */

public class TextButtonAdapter extends ArrayAdapter<TextButtonInfo> {
    private ArrayList<TextButtonInfo> mData;
    private Context mContext;
    private int mResource;

    public TextButtonAdapter(Context context, int resource) {
        super(context, resource);

        mData = new ArrayList<>();
        mContext = context;
        mResource = resource;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public TextButtonInfo getItem(int position) {
        return mData.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Context context = parent.getContext();
        final int mPosition = position;

        final TextView mTitle;
        final TextView mText;
        final Button mButton;

        TextButtonAdapter.Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView)convertView.findViewById(R.id.listview_row_text_button_title);
            mText = (TextView)convertView.findViewById(R.id.listview_row_text_button_text);
            mButton = (Button)convertView.findViewById(R.id.listview_row_text_button_button);

            mHolder = new TextButtonAdapter.Holder();
            mHolder.mTitle = mTitle;
            mHolder.mText = mText;
            mHolder.mButton = mButton;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (TextButtonAdapter.Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mText = mHolder.mText;
            mButton = mHolder.mButton;
        }

        mTitle.setText( mData.get(position).GetTitle() );
        mText.setText( mData.get(position).GetText() );

        mTitle.setEnabled( mData.get(position).GetEnable() );
        mText.setEnabled( mData.get(position).GetEnable() );
        mButton.setEnabled( mData.get(position).GetEnable() );

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TextButtonAdapter.OnClickListener listener = mData.get(mPosition).GetOnClickListener();
                if( listener != null ) {
                    listener.onClickListener(mPosition);
                }
            }
        });

        return convertView;
    }

    public interface OnClickListener {
        void onClickListener( int position );
    }

    @Override
    public void add(TextButtonInfo object) {
        mData.add(object);
        super.add(object);
    }

    @Override
    public void remove(TextButtonInfo object) {
        mData.remove(object);
        super.remove(object);
    }

    @Override
    public void clear() {
        mData.clear();
        super.clear();
    }

    private class Holder {
        TextView mTitle;
        TextView mText;
        Button mButton;
    }
}

