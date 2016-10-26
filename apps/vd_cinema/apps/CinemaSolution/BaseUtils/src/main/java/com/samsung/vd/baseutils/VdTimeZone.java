package com.samsung.vd.baseutils;

import android.app.AlarmManager;
import android.content.Context;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Locale;
import java.util.TimeZone;

/**
 * Created by doriya on 10/24/16.
 */
public class VdTimeZone {
    private static final String VD_DTAG = "VdTimeZone";

    private Context mContext;
    ArrayList<VdTimeZoneInfo> mTimeZoneInfo;

    public VdTimeZone(Context context) {
        mContext = context;
        GetAvaliableTimeZone();
    }

    public String GetTimeZone() {
        return TimeZone.getDefault().getID();
    }

    public int GetTimeZoneIndex() {
        for( int i = 0; i < mTimeZoneInfo.size(); i++) {
            if( TimeZone.getDefault().getID().equals( mTimeZoneInfo.get(i).GetTimeZone() ) )
                return i;
        }

        return -1;
    }

    public boolean SetTimeZone( String timeZone ) {
        AlarmManager alarm = (AlarmManager)mContext.getSystemService(Context.ALARM_SERVICE);

        for( VdTimeZoneInfo info : mTimeZoneInfo )
        {
            if( timeZone.equals( info.GetTimeZone() ) )
            {
                alarm.setTimeZone(timeZone);
                return true;
            }
        }

        Log.i( VD_DTAG, "Illegal TimeZone : " + timeZone );
        return false;
    }

    public boolean SetTimeZone( int index ) {
        if( index > mTimeZoneInfo.size() || index < 0 )
            return false;

        AlarmManager alarm = (AlarmManager)mContext.getSystemService(Context.ALARM_SERVICE);
        alarm.setTimeZone(mTimeZoneInfo.get(index).GetTimeZone());
        return true;
    }

    public String[] GetTimeZoneList() {
        String[] strResult = new String[mTimeZoneInfo.size()];

        for( int i = 0; i < mTimeZoneInfo.size(); i++ )
        {
            int offset = mTimeZoneInfo.get(i).GetOffsetTime() / 1000;
            int offsetHour = offset / 3600;
            int offsetMin = (offset >= 0) ? offset % 3600 / 60 : -offset % 3600 / 60;

            String strTemp1, strTemp2;
            strTemp1 = String.format( Locale.US, "GMT %+03d:%02d", offsetHour, offsetMin );
            strTemp2 = mTimeZoneInfo.get(i).GetTimeZone();

            strResult[i] = "[ " + strTemp1 + " ]  " + strTemp2;
        }

        return strResult;
    }

    private void GetAvaliableTimeZone() {
        String[] strTimeZone = TimeZone.getAvailableIDs();
        mTimeZoneInfo = new ArrayList<>();

        for( String strTemp : strTimeZone )
        {
            TimeZone timeZone = TimeZone.getTimeZone(strTemp);
            VdTimeZoneInfo info = new VdTimeZoneInfo( timeZone.getID(), timeZone.getDisplayName(), timeZone.getDisplayName(false, TimeZone.SHORT), timeZone.getRawOffset() );

            mTimeZoneInfo.add( info );
        }

        Collections.sort(mTimeZoneInfo, new Comparator<VdTimeZoneInfo>() {
            @Override
            public int compare(VdTimeZoneInfo lhs, VdTimeZoneInfo rhs) {
                if( lhs.GetOffsetTime() > rhs.GetOffsetTime() ) {
                    return 1;
                }
                else if( lhs.GetOffsetTime() < rhs.GetOffsetTime() ) {
                    return -1;
                }

                return 0;
            }
        });
    }

    private class VdTimeZoneInfo {
        private String mTimeZone;
        private String mDisplayName;
        private String mOffset;
        private int mOffsetTime;

        VdTimeZoneInfo( String timeZone, String displayName, String offset, int offsetTime ) {
            mTimeZone = timeZone;
            mDisplayName = displayName;
            mOffset = offset;
            mOffsetTime = offsetTime;
        }

        public String GetTimeZone() {
            return mTimeZone;
        }

        public String GetDisplayName() {
            return mDisplayName;
        }

        public String GetOffset() {
            return mOffset;
        }

        public int GetOffsetTime() {
            return mOffsetTime;
        }
    }
}
