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

public class LedQualityInfo {
    private static final String VD_DTAG = "LedQualityInfo";

    public static final String PATH_SOURCE = "DCI";
    public static final String PATH_TARGET = "/storage/sdcard0";
    public static final String NAME = "DEMO.txt";
    public static final String PATTERN_DATA = "\\w+\\s*=\\s*\\{\\s*(\\d*)\\s*,\\s*(\\d*)\\s*,\\s*(\\d*)\\s*,\\s*(\\d*)\\s*\\}";

    private int[] mRegister = {
            0x0004,     // XYZ_TO_RGB   XYZtoRGB
            0x0097,     // DGAM_RD_SEL  DgamRdSel
            0x0082,     // TGAM_RD_SEL  TgamRdSel
            0x0056,     // CC00         CC00
            0x0057,     // CC01         CC01
            0x0058,     // CC02         CC02
            0x0059,     // CC10         CC10
            0x005A,     // CC11         CC11
            0x005B,     // CC12         CC12
            0x005C,     // CC20         CC20
            0x005D,     // CC21         CC21
            0x005E,     // CC22         CC22
            0x00B6,     // LGSE1_R      LGSE1_R
            0x00B7,     // LGSE1_G      LGSE1_G
            0x00B8,     // LGSE1_B      LGSE1_B
            0x00B9,     // CC_R         CcGain_R
            0x00BA,     // CC_G         CcGain_G
            0x00BB,     // CC_B         CcGain_B
            0x00DD,     // BC           BcGain
    };

    private int[][] mData = new int[4][mRegister.length];

    public LedQualityInfo() {
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
            InputStreamReader inStreamReader = new InputStreamReader( inStream );
            BufferedReader bufferedReader = new BufferedReader( inStreamReader );

            String strValue;
            int index = 0;

            while( (strValue = bufferedReader.readLine()) != null )
            {
                Matcher matcher = pattern.matcher(strValue);
                if( matcher.matches() ) {
                    mData[0][index] = Integer.parseInt( matcher.group(1), 10 );
                    mData[1][index] = Integer.parseInt( matcher.group(2), 10 );
                    mData[2][index] = Integer.parseInt( matcher.group(3), 10 );
                    mData[3][index] = Integer.parseInt( matcher.group(4), 10 );
                }
                index++;
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Log.i(VD_DTAG, String.format(">>>>> path: %s", filePath)
        );

        return true;
    }

    public int[] GetRegister() {
        return mRegister;
    }

    public int[] GetData(int index ) {
        return mData[index];
    }
}
