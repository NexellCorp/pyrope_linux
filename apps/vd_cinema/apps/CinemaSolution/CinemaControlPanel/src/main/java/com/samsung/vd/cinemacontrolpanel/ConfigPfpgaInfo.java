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
 * Created by doriya on 2/28/17.
 */

public class ConfigPfpgaInfo {
    private static final String VD_DTAG = "ConfigPfpgaInfo";

    public static final String PATH_SOURCE = "DCI/PFPGA";
    public static final String PATH_TARGET = "/storage/sdcard0";
    public static final String NAME = "P_REG.txt";

    private static final int NUM_INDEX = 1;
    private static final int NUM_ENABLE = 1;
    private static final int NUM_REG_NUMBER = 1;

    public static final String PATTERN_DATA = "\\w+\\s*(\\d*)\\s*(\\d*)\\s*(\\d*)\\s*(\\d*)\\s*";

    public boolean[] mEnable = new boolean[4];
    public int[] mDataNum = new int[4];
    public int[][][] mData = new int[4][2][];

    public ConfigPfpgaInfo() {
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        Pattern pattern = Pattern.compile(PATTERN_DATA);
        try {
            FileInputStream inStream = new FileInputStream( filePath );
            BufferedReader bufferedReader = new BufferedReader( new InputStreamReader(inStream) );

            for( int i = 0; i < NUM_INDEX + NUM_ENABLE + NUM_REG_NUMBER; i++ ) {
                String strLine = bufferedReader.readLine();
                if( strLine == null ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Read Configuration. ( idx = %d )", i));
                    return false;
                }

                if( i < NUM_INDEX )
                    continue;

                Matcher matcher = pattern.matcher(strLine);
                if( matcher.matches() ) {
                    if( i < NUM_INDEX + NUM_ENABLE ) {
                        mEnable[0] = (Integer.parseInt( matcher.group(1), 10 ) == 1);
                        mEnable[1] = (Integer.parseInt( matcher.group(2), 10 ) == 1);
                        mEnable[2] = (Integer.parseInt( matcher.group(3), 10 ) == 1);
                        mEnable[3] = (Integer.parseInt( matcher.group(4), 10 ) == 1);
                    }
                    else {
                        mDataNum[0] = Integer.parseInt(matcher.group(1), 10);
                        mDataNum[1] = Integer.parseInt(matcher.group(2), 10);
                        mDataNum[2] = Integer.parseInt(matcher.group(3), 10);
                        mDataNum[3] = Integer.parseInt(matcher.group(4), 10);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        for( int i = 0; i < 4; i++ ) {
            mData[i][0] = new int[mDataNum[i]];
            mData[i][1] = new int[mDataNum[i]];
        }

        try {
            FileInputStream inStream = new FileInputStream( filePath );
            BufferedReader bufferedReader = new BufferedReader( new InputStreamReader(inStream) );

            // adjust reading offset
            int dataOffset = 0;
            while( bufferedReader.readLine() != null )
            {
                if (dataOffset++ >= NUM_INDEX + NUM_ENABLE + NUM_REG_NUMBER - 1)
                    break;
            }

            int index = 0;
            while( true )
            {
                String strLineReg = bufferedReader.readLine();
                String strLineData = bufferedReader.readLine();
                if( strLineReg == null || strLineData == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", filePath ));
                    break;
                }

                String[] strRegSplit = strLineReg.split("\\s+");
                String[] strDataSplit = strLineData.split("\\s+");

                int curPos = 0;
                for( int i = 0; i < strRegSplit.length; i++ )
                {
                    if( strRegSplit[i].equals("") )
                        continue;

                    for( int j = curPos; j < 4; j++ ) {
                        if( index < mDataNum[j] ) {
                            mData[j][0][index] = Integer.decode( strRegSplit[i] );
                            mData[j][1][index] = Integer.parseInt( strDataSplit[i], 10 );
                            curPos++;
                            break;
                        }
                        curPos++;
                    }
                }
                index++;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        //
        // Debug Message
        //
        for( int i = 0; i < 4; i++ ) {
            Log.i(VD_DTAG, String.format("* mode %d", i));
            Log.i(VD_DTAG, String.format("-. Uniformity Correction ( %b )", mEnable[i] ));
            Log.i(VD_DTAG, String.format("> register number for writing : %d", mData[i][0].length));
            for( int j = 0; j < mData[i][0].length; j++ ) {
                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                        mData[i][0][j], mData[i][0][j], mData[i][1][j], mData[i][1][j] ));
            }
        }

        return true;
    }

    boolean GetEnableUpdateUniformity( int mode ) {
        return mEnable[mode];
    }

    int[] GetRegister( int mode ) {
        return mData[mode][0];
    }

    int[] GetData( int mode ) {
        return mData[mode][1];
    }
}