package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by doriya on 12/21/16.
 */

public class StatusDescribeExpandableAdapter extends BaseExpandableListAdapter{
    private ArrayList<StatusDescribeExpandableInfo> mData;

    private int mGroupResource;
    private int mChildReousrce;

    public StatusDescribeExpandableAdapter(Context context, int groupResource, int childResource ) {
        super();
        mData = new ArrayList<>();
        mGroupResource = groupResource;
        mChildReousrce = childResource;
    }

    @Override
    public int getGroupCount() {
        return mData.size();
    }

    @Override
    public int getChildrenCount(int i) {
        return mData.get(i).GetSize();
    }

    @Override
    public Object getGroup(int i) {
        return mData.get(i);
    }

    @Override
    public Object getChild(int i, int i1) {
        return mData.get(i).GetInfo(i1);
    }

    @Override
    public long getGroupId(int i) {
        return 0;
    }

    @Override
    public long getChildId(int i, int i1) {
        return 0;
    }

    @Override
    public boolean hasStableIds() {
        return false;
    }

    @Override
    public View getGroupView(int i, boolean b, View view, ViewGroup viewGroup) {
        final Context context = viewGroup.getContext();
        TextView title;
        TextView describe;
        Holder holder;

        if( view == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = inflater.inflate(mGroupResource, viewGroup, false);

            title = (TextView)view.findViewById(R.id.listview_row_status_describe_title);
            describe = (TextView)view.findViewById(R.id.listview_row_status_describe_text);

            holder = new Holder();
            holder.mTitle = title;
            holder.mDescribe = describe;
            view.setTag(holder);
        }
        else {
            holder = (Holder)view.getTag();

            title = holder.mTitle;
            describe = holder.mDescribe;
        }

        title.setText( mData.get(i).GetGroup());
        describe.setText( "" );

        return view;
    }

    @Override
    public View getChildView(int i, int i1, boolean b, View view, ViewGroup viewGroup) {
        final Context context = viewGroup.getContext();
        TextView title;
        TextView describe;
        Holder holder;

        if( view == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = inflater.inflate(mChildReousrce, viewGroup, false);

            title = (TextView)view.findViewById(R.id.listview_row_status_describe_title);
            describe = (TextView)view.findViewById(R.id.listview_row_status_describe_text);

            holder = new Holder();
            holder.mTitle = title;
            holder.mDescribe = describe;
            view.setTag(holder);
        }
        else {
            holder = (Holder)view.getTag();

            title = holder.mTitle;
            describe = holder.mDescribe;
        }

        StatusDescribeInfo info = mData.get(i).GetInfo(i1);

        title.setText( info.GetTitle() );
        describe.setText( info.GetDescription() );

        return view;
    }

    public void addGroup( StatusDescribeExpandableInfo info ) {
        mData.add(info);
    }

    public void addChild( int pos, StatusDescribeInfo info ) {
        mData.get(pos).AddInfo(info);
    }

    public void clear() {
        int dataNum = mData.size();
        for( int i = 0; i < dataNum; i++ ) {
            mData.remove(0);
        }
    }

    @Override
    public boolean isChildSelectable(int i, int i1) {
        return false;
    }

    private class Holder {
        public TextView mTitle;
        public TextView mDescribe;
    }
}
