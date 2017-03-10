package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 12/11/16.
 */

public class LedDotCorrectInfo {
    private static final String VD_DTAG = "LedDotCorrectInfo";

    public static final String PATH = "DCI/DOT";
    public static final String PATTERN_DIR  = "ID(\\d*)";
    public static final String PATTERN_NAME = "RGB_P2_5_ID(\\d*)_(L|R)(\\d)(A|B).txt";
    public static final String PATTERN_DATA = "\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*),\\s*(\\d*)\\s*";

    private int[] mRegister = {
        0x0047,	0x0048,	0x0049,     // REG_IN_CC_00, REG_IN_CC_01, REG_IN_CC_02,
        0x004A,	0x004B,	0x004C,     // REG_IN_CC_10, REG_IN_CC_11, REG_IN_CC_12,
        0x004D,	0x004E,	0x004F,     // REG_IN_CC_20, REG_IN_CC_21, REG_IN_CC_22,
    };

    private int mIndex;
    private int mFlashSel;
    private int[] mData = new int[64*60*(mRegister.length-1)];

    public LedDotCorrectInfo() {
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
        //  RGB_P2_5_ID009_L0A.txt : 009, L, 0, A -> 0
        //  RGB_P2_5_ID009_R3B.txt : 009, R, 3, B -> 15
        //
        int idx = Integer.parseInt( matcher.group(1), 10 );
		if( idx < CinemaInfo.OFFSET_TCON ) {
            Log.i(VD_DTAG, String.format("Fail, Invalid Index. ( name: %s, index: %d )", fileName, idx) );
			return false;
		}

        if( (idx % 16) < 8 ) {
            // mIndex = idx + CinemaInfo.OFFSET_TCON;
            mIndex = idx;
        }
        else {
            // mIndex = (idx | 0x80) + CinemaInfo.OFFSET_TCON;
            mIndex = (idx | 0x80);
        }

        int side = matcher.group(2).equals("L") ? 0 : 1;
        int line = Integer.parseInt( matcher.group(3), 10 );
        int type = matcher.group(4).equals("A") ? 0 : 1;
        mFlashSel = (4 * line) + (2 * side) + type;

        int [][][] oriData = new int [64][60][mRegister.length];    // 14bit Original Data
        Log.i(VD_DTAG, "File Path : " + filePath);
        Pattern patternData = Pattern.compile(PATTERN_DATA);
        try {
            FileInputStream inStream = new FileInputStream( filePath );
            BufferedReader bufferedReader = new BufferedReader( new InputStreamReader(inStream) );

            String strValue;
            int cnt = 0;

            while( (strValue = bufferedReader.readLine()) != null ) {
                Matcher matcherData = patternData.matcher(strValue);
                if( matcherData.matches() ) {
                    for( int i = 0; i < mRegister.length; i++ ) {
                        oriData[cnt/oriData[0].length][cnt%oriData[0].length][i] = Integer.parseInt( matcherData.group(i+1), 10 );
                    }
                    cnt++;
                }
                else {
                    Log.i(VD_DTAG, String.format("Fail, Pattern Match. ( name: %s, pattern: %s )", fileName, PATTERN_DATA) );
                    return false;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        int [][][] tmpData = new int [64][60][mRegister.length-1];  // 16bit Aligned Data
        for( int i = 0; i < oriData.length; i++ ) {
            for( int j = 0; j < oriData[0].length; j++ ) {
                tmpData[i][j][0] = ((oriData[i][j][7] & 0x0003) << 14) | ((oriData[i][j][8] & 0x3FFF)      );
                tmpData[i][j][1] = ((oriData[i][j][6] & 0x000F) << 12) | ((oriData[i][j][7] & 0x3FFC) >>  2);
                tmpData[i][j][2] = ((oriData[i][j][5] & 0x003F) << 10) | ((oriData[i][j][6] & 0x3FF0) >>  4);
                tmpData[i][j][3] = ((oriData[i][j][4] & 0x00FF) <<  8) | ((oriData[i][j][5] & 0x3FC0) >>  6);
                tmpData[i][j][4] = ((oriData[i][j][3] & 0x03FF) <<  6) | ((oriData[i][j][4] & 0x3F00) >>  8);
                tmpData[i][j][5] = ((oriData[i][j][2] & 0x0FFF) <<  4) | ((oriData[i][j][3] & 0x3C00) >> 10);
                tmpData[i][j][6] = ((oriData[i][j][1] & 0x3FFF) <<  2) | ((oriData[i][j][2] & 0x3000) >> 12);
                tmpData[i][j][7] =                                       ((oriData[i][j][0] & 0x3FFF)      );
            }
        }

        int cnt = 0;
        for( int i = 0; i < tmpData.length; i++ ) {
            for( int j = 0; j < tmpData[0].length; j++ ) {
                for( int k = 0; k < tmpData[0][0].length; k++ ) {
                    mData[cnt] = tmpData[i][j][k];
                    cnt++;
                }
            }
        }

        Log.i(VD_DTAG, String.format(">>>>> slave: %d, module index: %d", mIndex, mFlashSel) );
        return true;
    }

    public int GetIndex() {
        return mIndex;
    }

    public int GetFlashSel() {
        return mFlashSel;
    }

    public int[] GetData() {
        return mData;
    }
}
