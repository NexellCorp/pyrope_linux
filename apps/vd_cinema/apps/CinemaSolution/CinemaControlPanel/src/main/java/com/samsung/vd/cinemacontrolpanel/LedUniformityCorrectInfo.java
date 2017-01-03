package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.StringTokenizer;

/**
 * Created by doriya on 12/23/16.
 */

public class LedUniformityCorrectInfo {
    public static final String PATH = "LED_Display(P2.5_Cinema)_IFC(20161101)";
    public static final String NAME = "UC_COEF.txt";

    public int[] mData = new int[64*64];

    public LedUniformityCorrectInfo() {
        for( int i = 0; i < mData.length; i++ ) {
            mData[i] = 0;
        }
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        try {
            FileInputStream inStream = new FileInputStream(filePath);
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inStream));

            String strValue;
            int index = 0;

            while( (strValue = bufferedReader.readLine()) != null ) {
                StringTokenizer token = new StringTokenizer(strValue);
                while(token.hasMoreTokens()) {
                    mData[index] = Integer.parseInt(token.nextToken(), 10);
                    index++;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    public int[] GetData() {
        return mData;
    }
}