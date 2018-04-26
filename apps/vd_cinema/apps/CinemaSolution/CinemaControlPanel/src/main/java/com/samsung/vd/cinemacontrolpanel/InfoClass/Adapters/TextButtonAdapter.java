package com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;

import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.InfoClass.TextButtonInfo;

import java.util.ArrayList;

/**
 * Created by doriya on 10/20/17.
 */

public class TextButtonAdapter extends ArrayAdapter<TextButtonInfo> {

    public static int BTN_APPLY = 0;
    public static int BTN_UPDATE = 1;
    public static int BTN_REMOVE = 2;

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
        final TextView mMode;
        final TextView mText;
        final Button mButton;
        final Button mRemovebtn;
        final Button mUpdatebtn;


        TextButtonAdapter.Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTitle = (TextView)convertView.findViewById(R.id.listview_row_text_button_title);
            mMode = (TextView)convertView.findViewById(R.id.listview_row_text_button_mode);
            mText = (TextView)convertView.findViewById(R.id.listview_row_text_button_text);
            mButton = (Button)convertView.findViewById(R.id.listview_row_text_button_button);
            mRemovebtn = (Button)convertView.findViewById(R.id.listview_row_text_button_remove);
            mUpdatebtn = (Button)convertView.findViewById(R.id.listview_row_text_button_update);

            mHolder = new TextButtonAdapter.Holder();
            mHolder.mTitle = mTitle;
            mHolder.mMode = mMode;
            mHolder.mText = mText;
            mHolder.mButton = mButton;
            mHolder.mRemovebtn = mRemovebtn;
            mHolder.mUpdatebtn = mUpdatebtn;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (TextButtonAdapter.Holder)convertView.getTag();

            mTitle = mHolder.mTitle;
            mMode = mHolder.mMode;
            mText = mHolder.mText;
            mButton = mHolder.mButton;
            mRemovebtn = mHolder.mRemovebtn;
            mUpdatebtn = mHolder.mUpdatebtn;
        }

        mTitle.setText( mData.get(position).GetIndex() );
        mMode.setText( mData.get(position).GetMode() );
        mText.setText( mData.get(position).GetText() );

        mTitle.setEnabled( mData.get(position).GetEnable() );
        mMode.setEnabled( mData.get(position).GetEnable() );
        mText.setEnabled( mData.get(position).GetEnable() );
        mButton.setEnabled( mData.get(position).GetEnable() );
        mRemovebtn.setEnabled(mData.get(position).GetEnable());

        if(null != mData.get(mPosition).GetUpdateOnClickListener()) {
            mUpdatebtn.setEnabled(mData.get(position).GetEnable());
        }else {
            mUpdatebtn.setEnabled(false);
        }


        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TextButtonAdapter.OnClickListener listener = mData.get(mPosition).GetOnClickListener();
                if( listener != null ) {
                    listener.onClickListener(mPosition);
                }
            }
        });

        mRemovebtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TextButtonAdapter.OnClickListener listener = mData.get(mPosition).GetRemoveOnClickListener();
                if( listener != null ) {
                    listener.onClickListener(mPosition);

//                    mData.get(mPosition) = new TextButtonInfo(String.format(Locale.US, "mode #%d", mPosition + 1), "");
                    mData.get(mPosition).SetIndex("");
                    mData.get(mPosition).SetText("");
                    mData.get(mPosition).SetEnable(false);

                    mTitle.setText("");
                    mMode.setText("");
                    mText.setText("");

                    mTitle.setEnabled( false );
                    mMode.setEnabled( false );
                    mText.setEnabled( false );
                    mButton.setEnabled( false );
                    mRemovebtn.setEnabled( false );
                    mUpdatebtn.setEnabled( false );
                    notifyDataSetChanged();
                }
            }
        });

        mUpdatebtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TextButtonAdapter.OnClickListener listener = mData.get(mPosition).GetUpdateOnClickListener();
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
        TextView mMode;
        TextView mText;
        Button mButton;
        Button mRemovebtn;
        Button mUpdatebtn;
    }
}

