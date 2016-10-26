package com.samsung.vd.cinemacontrolpanel;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;

import com.samsung.vd.baseutils.VdPreference;

/**
 * Created by doriya on 10/24/16.
 */
public class ScreenSaverService extends Service {
    private static final String VD_DTAG = "ScreenSaverService";

    private static final int KEY_SCREEN_TURN_OFF = 0;
    private static final int KEY_SCREEN_LOG_OUT = 1;

    private static final int DEFAULT_BRIGHTNESS = 102;

    private final IBinder mBinder = new LocalBinder();

    private int mScreenTurnOffTime = 60000;
    private int mScreenLogOutTime = 3000;

    public class LocalBinder extends Binder {
        ScreenSaverService GetService() {
            return ScreenSaverService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        RefreshScreenSaver();
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    public void RefreshScreenSaver() {
        VdPreference preference = new VdPreference( this, VdPreference.CINEMA_PREFERENCE );
        String strTemp = preference.GetValue( VdPreference.KEY_SCREEN_SAVING );
        if( null == strTemp )   mScreenTurnOffTime = 0;
        else                    mScreenTurnOffTime = Integer.parseInt( strTemp );
        if( mScreenTurnOffTime <= 0 )
            return ;

        try {
            int brightness = Settings.System.getInt( getContentResolver(), "screen_brightness" );
            if( 0 == brightness ) {
                Settings.System.putInt( getContentResolver(), "screen_brightness", DEFAULT_BRIGHTNESS );
            }
        } catch (Settings.SettingNotFoundException e) {
            e.printStackTrace();
        }

        mHandler.removeMessages(KEY_SCREEN_LOG_OUT);
        mHandler.removeMessages(KEY_SCREEN_TURN_OFF);

        mHandler.sendEmptyMessageDelayed( KEY_SCREEN_TURN_OFF, mScreenTurnOffTime );
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case KEY_SCREEN_TURN_OFF:
                    Settings.System.putInt( getContentResolver(), "screen_brightness", 0 );
                    mHandler.sendEmptyMessageDelayed( KEY_SCREEN_LOG_OUT, mScreenLogOutTime );
                    break;
//                case KEY_SCREEN_LOG_OUT:
//                    Intent intent = new Intent(ScreenSaverService.this, LoginActivity.class);
//                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//                    startActivity(intent);
//                    break;
            }
        }
    };
}
