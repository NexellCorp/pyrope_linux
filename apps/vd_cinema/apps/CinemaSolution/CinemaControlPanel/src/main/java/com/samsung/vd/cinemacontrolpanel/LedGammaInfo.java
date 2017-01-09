package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 12/11/16.
 */

public class LedGammaInfo {
    private static final String VD_DTAG = "LedGammaInfo";

    public static final String PATH_SOURCE = "DCI/LUT";
    public static final String PATH_TARGET = "/storage/sdcard0";
    public static final String PATTERN_NAME = "(T|D)GAM(0|1)_(R|G|B).txt";

    public static final int TYPE_TARGET     = 0;
    public static final int TYPE_DEVICE     = 1;

    public static final int TABLE_LUT0      = 0;
    public static final int TABLE_LUT1      = 1;

    public static final int CHANNEL_RED     = 0;
    public static final int CHANNEL_GREEN   = 1;
    public static final int CHANNEL_BLUE    = 2;

    private int mType = -1;        // 0: target gamma, 1: device gamma
    private int mTable = -1;       // 0: Table Lut #0, 1: Table Lut #1
    private int mChannel = -1;     // 0: Red, 1: Green, 2: Blue
    private int[] mData = new int[4096];

    public LedGammaInfo() {
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        Pattern pattern = Pattern.compile( PATTERN_NAME );
        Matcher matcher = pattern.matcher(fileName);
        if( !matcher.matches() ) {
            Log.i(VD_DTAG, String.format("Fail, Pattern Match. ( name : %s, pattern : %s )", fileName, PATTERN_NAME) );
            return false;
        }

        if( matcher.group(1).equals("T") ) mType = TYPE_TARGET;
        if( matcher.group(1).equals("D") ) mType = TYPE_DEVICE;

        if( matcher.group(2).equals("0") ) mTable = TABLE_LUT0;
        if( matcher.group(2).equals("1") ) mTable = TABLE_LUT1;

        if( matcher.group(3).equals("R") ) mChannel = CHANNEL_RED;
        if( matcher.group(3).equals("G") ) mChannel = CHANNEL_GREEN;
        if( matcher.group(3).equals("B") ) mChannel = CHANNEL_BLUE;

        int index = 0;
        try {
            FileInputStream inStream = new FileInputStream( filePath );
            BufferedReader bufferedReader = new BufferedReader( new InputStreamReader(inStream) );

            String strValue;
            while( (strValue = bufferedReader.readLine()) != null )
            {
                mData[index] = Integer.parseInt(strValue, 10);
                index++;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        Log.i(VD_DTAG, String.format(">>>>> path: %s, type: %s(%d), table: %s(%d), channel: %s(%d)",
                filePath,
                matcher.group(1), mType,
                matcher.group(2), mTable,
                matcher.group(3), mChannel)
        );

        return true;
    }

    public int GetType() {
        return mType;
    }

    public int GetTable() {
        return mTable;
    }

    public int GetChannel() {
        return mChannel;
    }

    public int[] GetData() {
        return mData;
    }
}
