package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 12/11/16.
 */

public class LedDotCorrectInfo {
    private static final String VD_DTAG = "LedDotCorrectInfo";

    public static final String PATH = "DCI/DOT";
    public static final String PATTERN_DIR  = "ID(\\d*)";

    public static final String PATTERN_NAME = "RGB_P(2_5|3_3)_ID(\\d*)_(L|R)(\\d)(A|B).txt";
    public static final String PATTERN_DATA = "\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*)\\s*";

    public static final int MAX_REG_NUM = 9;
    public static final int MAX_MODULE_NUM = 24;

    // Sending Data Format
    private int[] mData;

    private Context mContext;
    private int mIndex;         // Slave Address
    private int mModule;        // Module Number

    public LedDotCorrectInfo( Context context ) {
        mContext = context;
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );

        Pattern pattern = Pattern.compile(PATTERN_NAME);
        Matcher matcher = pattern.matcher(fileName);
        if( !matcher.matches() ) {
            Log.i(VD_DTAG, String.format("Fail, Pattern Match. ( name: %s, pattern: %s )", fileName, PATTERN_NAME) );
            return false;
        }

        //
        //  RGB_P2_5_ID009_L0A.txt : 2_5, 009, L, 0, A -> 0
        //  RGB_P2_5_ID009_R3B.txt : 2_5, 009, R, 3, B -> 15
        //  RGB_P3_3_ID293_L2A.txt : 3_3, 293, L, 2, A ->
        //
        int[][][] src;
        int col, row;

        int screenType = ((CinemaInfo)mContext).GetScreenType();
        if( (screenType == CinemaInfo.SCREEN_TYPE_P25) && matcher.group(1).equals("2_5") ) {
            col = 64;
            row = 60;
        }
        else if( (screenType == CinemaInfo.SCREEN_TYPE_P33) && matcher.group(1).equals("3_3") ) {
            col = 48;
            row = 45;
        }
        else {
            return false;
        }

        mData = new int[col*row*(MAX_REG_NUM-1)];   // 16bit aligned data
        src = new int [col][row][MAX_REG_NUM];      // 14bit original data

        int number = Integer.parseInt( matcher.group(2), 10 );
        mIndex = ((CinemaInfo)mContext).GetCabinetId(number);

        int side = matcher.group(3).equals("L") ? 0 : 1;
        int line = Integer.parseInt( matcher.group(4), 10 );
        int type = matcher.group(5).equals("A") ? 0 : 1;
        mModule = (4 * line) + (2 * side) + type;

        Log.i(VD_DTAG, "File Path : " + filePath);
        Pattern patternData = Pattern.compile(PATTERN_DATA);
        try {
            FileInputStream inStream = new FileInputStream( filePath );
            InputStreamReader inStreamReader = new InputStreamReader( inStream );
            BufferedReader bufferedReader = new BufferedReader( inStreamReader );

            String strValue;
            int cnt = 0;

            while( (strValue = bufferedReader.readLine()) != null ) {
                Matcher matcherData = patternData.matcher(strValue);
                if( matcherData.matches() ) {
                    for( int i = 0; i < MAX_REG_NUM; i++ ) {
                        src[cnt/src[0].length][cnt%src[0].length][i] = Integer.parseInt( matcherData.group(i+1), 10 );
                    }
                    cnt++;
                }
                else {
                    Log.i(VD_DTAG, String.format("Fail, Pattern Match. ( name: %s, pattern: %s )", fileName, PATTERN_DATA) );
                    return false;
                }

                if( col*row == cnt ) {
                    Log.i(VD_DTAG, String.format("Warning, Out of Line Length.( name: %s )", fileName));
                    break;
                }
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        int offset = 0;
        for( int i = 0; i < src.length; i++ ) {
            for( int j = 0; j < src[0].length; j++ ) {
                mData[offset    ] = ((src[i][j][7] & 0x0003) << 14) | ((src[i][j][8] & 0x3FFF)      );
                mData[offset + 1] = ((src[i][j][6] & 0x000F) << 12) | ((src[i][j][7] & 0x3FFC) >>  2);
                mData[offset + 2] = ((src[i][j][5] & 0x003F) << 10) | ((src[i][j][6] & 0x3FF0) >>  4);
                mData[offset + 3] = ((src[i][j][4] & 0x00FF) <<  8) | ((src[i][j][5] & 0x3FC0) >>  6);
                mData[offset + 4] = ((src[i][j][3] & 0x03FF) <<  6) | ((src[i][j][4] & 0x3F00) >>  8);
                mData[offset + 5] = ((src[i][j][2] & 0x0FFF) <<  4) | ((src[i][j][3] & 0x3C00) >> 10);
                mData[offset + 6] = ((src[i][j][1] & 0x3FFF) <<  2) | ((src[i][j][2] & 0x3000) >> 12);
                mData[offset + 7] =                                   ((src[i][j][0] & 0x3FFF)      );

                offset += 8;
            }
        }

        Log.i(VD_DTAG, String.format("Parse Done.( slave: 0x%02x, module: %d )", (byte)mIndex, mModule));
        return true;
    }

    public boolean Make( int index, int module, byte[] inData, String outPath ) {
        //
        //
        //
        int col, row;
        int screenType = ((CinemaInfo)mContext).GetScreenType();
        if( screenType == CinemaInfo.SCREEN_TYPE_P25 ) {
            col = 64;
            row = 60;
        }
        else if( screenType == CinemaInfo.SCREEN_TYPE_P33 ) {
            col = 48;
            row = 45;
        }
        else {
            return false;
        }

        int expectSize = col * row * (MAX_REG_NUM-1) * 2;
        if( inData == null || inData.length != expectSize ) {
            Log.i(VD_DTAG, String.format("invalid input data. ( expected: %d bytes, size: %s bytes )",
                    expectSize,
                    (inData != null) ? String.valueOf(inData.length) : "unknown") );

            return false;
        }

        String strPitch = (screenType == CinemaInfo.SCREEN_TYPE_P33) ? "P3_3" : "P2_5";
        String strLine = String.valueOf(module/4);
        String strSide = ((module % 4) < 2) ? "L" : "R";
        String strType = (((module % 4) % 2) == 0) ? "A" : "B";
        String strFile = String.format(Locale.US, "%s/RGB_%s_ID%03d_%s%s%s.txt",
                outPath, strPitch, ((CinemaInfo)mContext).GetCabinetNumber((byte)index), strSide, strLine, strType );

        //
        // Convert Data
        //
        int[] dstData = new int[col*row*MAX_REG_NUM];
        int[] tmpData = new int[col*row*(MAX_REG_NUM-1)];

        int dstOffset = 0;
        int tmpOffset = 0;

        for( int i = 0; i < inData.length / 2; i++ )
        {
            tmpData[i] = (((int)inData[2*i] << 8) & 0xFF00) | (((int)inData[2*i+1]) & 0x00FF);
        }

//        for( int i = 0; i < tmpData.length / (MAX_REG_NUM-1); i++ )
//        {
//            int offset = i * (MAX_REG_NUM-1);
//            String strData = String.format( Locale.US, ">> %d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\r\n",
//                    tmpData[offset  ], tmpData[offset+1], tmpData[offset+2], tmpData[offset+3],
//                    tmpData[offset+4], tmpData[offset+5], tmpData[offset+6], tmpData[offset+7] );
//
//            Log.i(VD_DTAG, strData);
//        }


        for( int i = 0; i < dstData.length / MAX_REG_NUM; i++ )
        {
            dstData[dstOffset    ] = ((tmpData[tmpOffset + 7]      ) & 0x3FFF);
            dstData[dstOffset + 1] = ((tmpData[tmpOffset + 6] >>  2) & 0x3FFF);
            dstData[dstOffset + 2] = ((tmpData[tmpOffset + 6] << 12) & 0x3000) | ((tmpData[tmpOffset + 5] >>  4) & 0x0FFF);
            dstData[dstOffset + 3] = ((tmpData[tmpOffset + 5] << 10) & 0x3C00) | ((tmpData[tmpOffset + 4] >>  6) & 0x03FF);
            dstData[dstOffset + 4] = ((tmpData[tmpOffset + 4] <<  8) & 0x3F00) | ((tmpData[tmpOffset + 3] >>  8) & 0x00FF);
            dstData[dstOffset + 5] = ((tmpData[tmpOffset + 3] <<  6) & 0x3FC0) | ((tmpData[tmpOffset + 2] >> 10) & 0x003F);
            dstData[dstOffset + 6] = ((tmpData[tmpOffset + 2] <<  4) & 0x3FF0) | ((tmpData[tmpOffset + 1] >> 12) & 0x000F);
            dstData[dstOffset + 7] = ((tmpData[tmpOffset + 1] <<  2) & 0x3FFC) | ((tmpData[tmpOffset    ] >> 14) & 0x0003);
            dstData[dstOffset + 8] = ((tmpData[tmpOffset    ]      ) & 0x3FFF);

            dstOffset += (MAX_REG_NUM    );
            tmpOffset += (MAX_REG_NUM - 1);
        }

//        for( int i = 0; i < dstData.length / MAX_REG_NUM; i++ )
//        {
//            int offset = MAX_REG_NUM * i;
//            String strData = String.format( Locale.US, "%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\r\n",
//                    dstData[offset  ], dstData[offset+1], dstData[offset+2], dstData[offset+3], dstData[offset+4],
//                    dstData[offset+5], dstData[offset+6], dstData[offset+7], dstData[offset+8] );
//
//            Log.i(VD_DTAG, strData);
//        }


        //
        //
        //
        File file = new File(strFile);
        try {
            FileWriter fileWriter = new FileWriter( file );

            for( int i = 0; i < dstData.length / MAX_REG_NUM; i++ )
            {
                int offset = MAX_REG_NUM * i;
//                String strData = String.format( Locale.US, "%5d,\t%5d,\t%5d,\t%5d,\t%5d,\t%5d,\t%5d,\t%5d,\t%5d\r\n",
//                String strData = String.format( Locale.US, "%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\r\n",
                String strData = String.format( Locale.US, "%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d\r\n",
                        dstData[offset  ], dstData[offset+1], dstData[offset+2], dstData[offset+3], dstData[offset+4],
                        dstData[offset+5], dstData[offset+6], dstData[offset+7], dstData[offset+8] );

                fileWriter.write( strData );
            }

            fileWriter.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Log.i(VD_DTAG, String.format("Make Done. ( %s )", strFile));
        return true;
    }

    public int GetIndex() {
        return mIndex;
    }

    public int GetModule() {
        return mModule;
    }

    public int[] GetData() {
        return mData;
    }
}
