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
        final Button[] mButton = new Button[4];

        TextButtonAdapter.Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView)convertView.findViewById(R.id.listview_row_text_button_title);
            mText = (TextView)convertView.findViewById(R.id.listview_row_text_button_text);
            mButton[0] = (Button)convertView.findViewById(R.id.listview_row_text_button_button0);
            mButton[1] = (Button)convertView.findViewById(R.id.listview_row_text_button_button1);
            mButton[2] = (Button)convertView.findViewById(R.id.listview_row_text_button_button2);
            mButton[3] = (Button)convertView.findViewById(R.id.listview_row_text_button_button3);

            mHolder = new TextButtonAdapter.Holder();
            mHolder.mTitle = mTitle;
            mHolder.mText = mText;

            mHolder.mButton[0] = mButton[0];
            mHolder.mButton[1] = mButton[1];
            mHolder.mButton[2] = mButton[2];
            mHolder.mButton[3] = mButton[3];
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (TextButtonAdapter.Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mText = mHolder.mText;
            mButton[0] = mHolder.mButton[0];
            mButton[1] = mHolder.mButton[1];
            mButton[2] = mHolder.mButton[2];
            mButton[3] = mHolder.mButton[3];
        }

        mTitle.setText( mData.get(position).GetTitle() );
        mText.setText( mData.get(position).GetText() );

        mTitle.setEnabled( mData.get(position).GetTextEnable() );
        mText.setEnabled( mData.get(position).GetTextEnable() );

        for( int i = 0; i < TextButtonInfo.MAX_BTN_NUM; i++ ) {
            final int button = i;
            mButton[button].setEnabled( mData.get(position).GetBtnEnable(button) );

            mButton[button].setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    TextButtonAdapter.OnClickListener listener = mData.get(mPosition).GetOnClickListener(button);
                    if( listener != null ) {
                        listener.onClickListener(mPosition);
                    }
                }
            });
        }

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
        Button[] mButton = new Button[TextButtonInfo.MAX_BTN_NUM];
    }
}

