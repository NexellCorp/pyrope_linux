package com.samsung.vd.cinemacontrolpanel;

import android.app.Service;
import android.content.Intent;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * Created by doriya on 11/9/16.
 */
public class CinemaService extends Service {
    private static final String VD_DTAG = "CinemaService";
    private final IBinder mBinder = new LocalBinder();
    private final EventReceiver  mEventReceiver = new EventReceiver();

    public class LocalBinder extends Binder {
        CinemaService GetService() {
            return CinemaService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        RefreshScreenSaver();
        mEventReceiver.Start();

        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        mEventReceiver.Stop();

        return super.onUnbind(intent);
    }

    //
    //  For Screen Saver
    //
    public static final String OFF_DISABLE = "0";
    public static final String OFF_1MIN    = "60000";
    public static final String OFF_3MIN    = "180000";
    public static final String OFF_5MIN    = "300000";
    public static final String OFF_10MIN   = "6000000";
    public static final String OFF_20MIN   = "12000000";
    public static final String OFF_30MIN   = "18000000";
    public static final String OFF_TIME[] = {
            OFF_DISABLE, OFF_1MIN, OFF_3MIN, OFF_5MIN, OFF_10MIN, OFF_20MIN, OFF_30MIN
    };

    private static final int DEFAULT_BRIGHTNESS  = 102;
    private static final int DEFAULT_LOGOUT_TIME = 3000;

    private static final int KEY_SCREEN_TURN_OFF = 0;
    private static final int KEY_SCREEN_LOG_OUT  = 1;

    public void RefreshScreenSaver() {
        mHandler.removeMessages(KEY_SCREEN_LOG_OUT);
        mHandler.removeMessages(KEY_SCREEN_TURN_OFF);

        if( 0 == GetBrightness() ) {
            SetBrightness( DEFAULT_BRIGHTNESS );
        }

        int screenTurnOffTime;
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_SAVING);

        if( null == strTemp )   screenTurnOffTime = 0;
        else                    screenTurnOffTime = Integer.parseInt( strTemp );
        if( screenTurnOffTime <= 0 )
            return ;

        mHandler.sendEmptyMessageDelayed( KEY_SCREEN_TURN_OFF, screenTurnOffTime );
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case KEY_SCREEN_TURN_OFF:
                    SetBrightness( 0 );
                    mHandler.sendEmptyMessageDelayed( KEY_SCREEN_LOG_OUT, DEFAULT_LOGOUT_TIME );
                    break;
                case KEY_SCREEN_LOG_OUT:
                    ((CinemaInfo)getApplicationContext()).InsertLog("Logout.");

                    Intent intent = new Intent(CinemaService.this, LoginActivity.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                    break;
            }
        }
    };

    public void TurnOff() {
        mHandler.removeMessages(KEY_SCREEN_LOG_OUT);
        mHandler.removeMessages(KEY_SCREEN_TURN_OFF);

        mHandler.sendEmptyMessage( KEY_SCREEN_TURN_OFF );
    }

    public boolean IsOn() {
        return (GetBrightness() != 0);
    }

    private void SetBrightness( int brightness ) {
        Settings.System.putInt( getContentResolver(), "screen_brightness", brightness );
    }

    private int GetBrightness() {
        int brightness = 0;
        try {
            brightness = Settings.System.getInt( getContentResolver(), "screen_brightness" );
        } catch (Settings.SettingNotFoundException e) {
            e.printStackTrace();
        }

        return brightness;
    }

    //
    //  For Event Receiver
    //
    private class EventReceiver extends Thread {
        private String SOCKET_NAME = "cinema.event";
        private boolean mRun = false;

        public EventReceiver() {
        }

        @Override
        public void run() {
            Log.i(VD_DTAG, ">>>>> Start Local Socket Server");
            try {
                LocalServerSocket lServer = new LocalServerSocket(SOCKET_NAME);

                while( mRun )
                {
                    LocalSocket lSocket = lServer.accept();
                    if( null != lSocket ) {
                        String strEvent = Read(lSocket.getInputStream());
                        Log.i(VD_DTAG, ">>> " + strEvent);

                        String[] strToken = strEvent.split(" ");
                        if( strToken[0].equals("Error") )
                        {
                            CinemaAlert.Show( getApplicationContext(), "Alert", strToken[1] + " " + strToken[0] );
                            ((CinemaInfo)getApplicationContext()).InsertLog(strToken[1] + ", " + strToken[0] + ".");
                        }

                        lSocket.close();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            Log.i(VD_DTAG, ">>>>> Stop Local Socket Server");
        }

        public synchronized void Start() {
            if( !mRun ) {
                mRun = true;
                start();
            }
        }

        public synchronized void Stop() {
            if( mRun ) {
                mRun = false;
                try {
                    Write("");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        private String Read(InputStream inStream) {
            InputStreamReader inStreamReader = new InputStreamReader(inStream);
            BufferedReader bufferedReader = new BufferedReader(inStreamReader);
            StringBuilder strBuilder = new StringBuilder();

            String inLine;
            try {
                while (((inLine = bufferedReader.readLine()) != null)) {
                    strBuilder.append(inLine);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            String strResult = strBuilder.toString();

            try {
                bufferedReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            return strResult;
        }

        private void Write(String message) throws IOException {
            LocalSocket sender = new LocalSocket();
            sender.connect(new LocalSocketAddress(SOCKET_NAME));
            sender.getOutputStream().write(message.getBytes());
            sender.getOutputStream().close();

            Log.i( VD_DTAG, ">>>>> " + new LocalSocketAddress(SOCKET_NAME).getName());
        }
    }
}
