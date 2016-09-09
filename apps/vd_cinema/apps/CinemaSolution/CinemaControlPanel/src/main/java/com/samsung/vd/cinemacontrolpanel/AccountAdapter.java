package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 8/24/16.
 */
public class AccountAdapter extends ArrayAdapter<String> {
    private ArrayList<String> mData;
    private int mResource;

    public AccountAdapter(Context context, int resource) {
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

        final TextView mTextGroup;
        final EditText mEditId;
        final EditText mEditPw;
        final Button mButtonModify;
        final Holder mHolder;

        if( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(mResource, parent, false);

            mTextGroup = (TextView)convertView.findViewById(R.id.listview_row_account_group);
            mEditId = (EditText) convertView.findViewById(R.id.listview_row_account_id);
            mEditPw = (EditText)convertView.findViewById(R.id.listview_row_account_pw);
            mButtonModify = (Button)convertView.findViewById(R.id.listview_row_account_button);

            mHolder = new Holder();
            mHolder.mTextGroup = mTextGroup;
            mHolder.mEditId = mEditId;
            mHolder.mEditPw = mEditPw;
            mHolder.mButtonModify = mButtonModify;
            convertView.setTag(mHolder);
        }
        else {
            mHolder = (Holder)convertView.getTag();

            mTextGroup = mHolder.mTextGroup;
            mEditId = mHolder.mEditId;
            mEditPw = mHolder.mEditPw;
            mButtonModify = mHolder.mButtonModify;
        }

        mTextGroup.setText( mData.get(position) );
        mButtonModify.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String str = ((Button)v).getText().toString();
                Log.i("TAG", str);
                if( ((Button)v).getText().toString().equals(R.string.txt_modify) )
                {
                    ((Button)v).setText(R.string.txt_modify);
                    EnableEditText( mEditId, true );
                    EnableEditText( mEditPw, true );
                }
                else
                {
                    ((Button)v).setText(R.string.txt_done);
                    EnableEditText( mEditId, false );
                    EnableEditText( mEditPw, false );
                }
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
        TextView mTextGroup;
        EditText mEditId;
        EditText mEditPw;
        Button mButtonModify;
    }

    void EnableEditText( EditText id, boolean enable ) {
        id.setClickable(enable);
        id.setEnabled(enable);
        id.setFocusable(enable);
        id.setFocusableInTouchMode(enable);
    }
}
