package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 12/11/16.
 */

public class LedGammaInfo {
    private static final String VD_DTAG = "LedGammaInfo";

    public static final String PATH_SOURCE = "DCI/TCON/LUT";
    public static final String PATH_TARGET_EEPROM = "/storage/sdcard0/SAMSUNG/TCON_EEPROM/LUT";
    public static final String PATH_TARGET_USB = "/storage/sdcard0/SAMSUNG/TCON_USB/LUT";

    public static final String PATTERN_NAME = "(T|D)GAM(0|1)_(R|G|B)(.*).txt";

    public static final int TYPE_TARGET     = 0;
    public static final int TYPE_DEVICE     = 1;

    public static final int TABLE_LUT0      = 0;
    public static final int TABLE_LUT1      = 1;

    public static final int CHANNEL_RED     = 0;
    public static final int CHANNEL_GREEN   = 1;
    public static final int CHANNEL_BLUE    = 2;

    private int mType = -1;         // 0: target gamma, 1: device gamma
    private int mTable = -1;        // 0: Table Lut #0, 1: Table Lut #1
    private int mChannel = -1;      // 0: Red, 1: Green, 2: Blue
    private int mCurveType = -1;    //
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

        if( matcher.group(1).toUpperCase().equals("T") ) mType = TYPE_TARGET;
        if( matcher.group(1).toUpperCase().equals("D") ) mType = TYPE_DEVICE;

        if( matcher.group(2).toUpperCase().equals("0") ) mTable = TABLE_LUT0;
        if( matcher.group(2).toUpperCase().equals("1") ) mTable = TABLE_LUT1;

        if( matcher.group(3).toUpperCase().equals("R") ) mChannel = CHANNEL_RED;
        if( matcher.group(3).toUpperCase().equals("G") ) mChannel = CHANNEL_GREEN;
        if( matcher.group(3).toUpperCase().equals("B") ) mChannel = CHANNEL_BLUE;

        if( !matcher.group(4).equals("") ) {
            String curveType = matcher.group(4).toUpperCase();
            try {
                if( curveType.contains("_G") ) mCurveType = 1000 + Integer.parseInt(curveType.replaceAll("_G", "" ), 10);
                if( curveType.contains("_P") ) mCurveType = 2000 + Integer.parseInt(curveType.replaceAll("_P", "" ), 10);
                if( curveType.contains("_U") ) mCurveType = 3000 + Integer.parseInt(curveType.replaceAll("_U", "" ), 10);
            } catch (NumberFormatException e) {
                Log.i(VD_DTAG, String.format(Locale.US, "Cannot Convert String to Integer. ( %s )", curveType));
                return false;
            }
        }

        int index = 0;
        try {
            FileInputStream inStream = new FileInputStream( filePath );
            InputStreamReader inStreamReader = new InputStreamReader( inStream );
            BufferedReader bufferedReader = new BufferedReader( inStreamReader );

            String strValue;
            while( (strValue = bufferedReader.readLine()) != null )
            {
                try {
                    mData[index] = Integer.parseInt(strValue, 10);
                } catch (NumberFormatException e) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Cannot Convert String to Integer. ( %s )", strValue));
                    return false;
                }

                index++;
                if( index >= mData.length )
                    break;
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Log.i(VD_DTAG, String.format(">> path: %s, type: %s(%d), table: %s(%d), channel: %s(%d), curve: %s(%d)",
                filePath,
                matcher.group(1), mType,
                matcher.group(2), mTable,
                matcher.group(3), mChannel,
                matcher.group(4), mCurveType)
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

    public int GetCurveType() {
        return mCurveType;
    }
}
