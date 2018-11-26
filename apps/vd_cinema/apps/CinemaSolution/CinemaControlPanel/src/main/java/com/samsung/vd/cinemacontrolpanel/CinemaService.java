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

import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.Locale;

/**
 * Created by doriya on 11/9/16.
 */
public class CinemaService extends Service {
    private static final String VD_DTAG = "CinemaService";

    private final IBinder mBinder = new LocalBinder();
    private final TamperEventReceiver mTamperEventReceiver = new TamperEventReceiver();
    private final SecureEventReceiver mSecureEventReceiver = new SecureEventReceiver();
    private final TmsEventReceiver mTmsEventReceiver = new TmsEventReceiver();

    private final ScreenSaverHandler mHandler = new ScreenSaverHandler(this);

    public class LocalBinder extends Binder {
        CinemaService GetService() {
            return CinemaService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        RefreshScreenSaver();
        mTamperEventReceiver.Start();
        mSecureEventReceiver.Start();
        mTmsEventReceiver.Start();

        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        mTamperEventReceiver.Stop();
        mSecureEventReceiver.Stop();
        mTmsEventReceiver.Stop();

        return super.onUnbind(intent);
    }

    private TmsEventCallback mTmsEventCallback = null;
    interface TmsEventCallback {
        void onTmsEventCallback( Object[] values );
    }

    public void RegisterTmsCallback( TmsEventCallback callback ) {
        mTmsEventCallback = callback;
    }

    private ScreenSaverCallback mScreenSaverCallback = null;
    interface ScreenSaverCallback {
        void onPrepare();
    }

    public void RegisterScreenSaverCallback( ScreenSaverCallback callback ) {
        mScreenSaverCallback = callback;
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

    private static class ScreenSaverHandler extends Handler {
        private WeakReference<CinemaService> mService;

        public ScreenSaverHandler( CinemaService service ) {
            mService = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            CinemaService service = mService.get();
            if( service != null ) {
                service.handleMessage(msg);
            }
        }
    }

    private void handleMessage( Message msg ) {
        switch (msg.what) {
            case KEY_SCREEN_TURN_OFF:
                SetBrightness( 0 );
                // Logout. ( It is not stable )
                if( ((CinemaInfo)getApplicationContext()).IsEnableLogout() ) {
                    mHandler.sendEmptyMessageDelayed( KEY_SCREEN_LOG_OUT, DEFAULT_LOGOUT_TIME );
                }
                break;

            case KEY_SCREEN_LOG_OUT:
                if( mScreenSaverCallback != null )
                    mScreenSaverCallback.onPrepare();

                Intent intent = new Intent(CinemaService.this, LoginActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                break;
        }
    }

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
    //  For Tamper Event Receiver
    //
    private class TamperEventReceiver extends Thread {
        private String SOCKET_NAME = "cinema.tamper";
        private boolean mRun = false;

        private boolean mDoorTamper = false;
        private boolean mMarriageTamper = false;

        public TamperEventReceiver() {
        }

        @Override
        public void run() {
            try {
                boolean isDetectTamper = ((CinemaInfo)getApplicationContext()).IsDetectTamper();
                LocalServerSocket lServer = new LocalServerSocket(SOCKET_NAME);

                while( mRun )
                {
                    LocalSocket lSocket = lServer.accept();
                    if( null == lSocket )
                        continue;

                    if( !isDetectTamper ) {
                        lSocket.close();
                        continue;
                    }

                    String strEvent = ReadString(lSocket);

                    String[] strToken = strEvent.split(" ");
                    if( strToken[0].equals("Error") )
                    {
                        if( !mDoorTamper && strToken[1].equals("DoorTamper") ) {
                            mDoorTamper = true;

                            CinemaAlert.Show( getApplicationContext(), "Alert", strToken[0] + " " + strToken[1], CinemaAlert.TYPE_DOOR, new CinemaAlert.OnFinishListener() {
                                @Override
                                public void onFinish() {
                                    mDoorTamper = false;
                                }
                            });

                            RefreshScreenSaver();
                        }

                        if( !mMarriageTamper && strToken[1].equals("MarriageTamper") ) {
                            mMarriageTamper = true;

                            CinemaAlert.Show( getApplicationContext(), "Alert", strToken[0] + " " + strToken[1], CinemaAlert.TYPE_MARRIAGE, new CinemaAlert.OnFinishListener() {
                                @Override
                                public void onFinish() {
                                    mMarriageTamper = false;
                                }
                            });

                            RefreshScreenSaver();
                        }
                    }

                    lSocket.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
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
                WriteDummy(SOCKET_NAME);
            }
        }
    }

    //
    //  For Secure Event Receiver
    //
    private class SecureEventReceiver extends Thread {
        private String SOCKET_NAME = "cinema.secure";
        private boolean mRun = false;

        public SecureEventReceiver() {
        }

        @Override
        public void run() {
            try {
                LocalServerSocket lServer = new LocalServerSocket(SOCKET_NAME);

                while( mRun )
                {
                    LocalSocket lSocket = lServer.accept();
                    if( null == lSocket )
                        continue;

                    String strEvent = ReadString(lSocket);
                    String strAlive = ((CinemaInfo)getApplicationContext()).GetSecureAlive();

                    if( strEvent.equals("Alive") && strAlive.equals("false") ) {
                        ((CinemaInfo)getApplicationContext()).SetSecureAlive("true");
                    }

                    lSocket.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
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
                WriteDummy(SOCKET_NAME);
            }
        }
    }

    //
    //  For Tms Event Receiver
    //
    private class TmsEventReceiver extends Thread {
        private String SOCKET_NAME = "cinema.tms";
        private boolean mRun = false;
        private boolean mLoop = false;
        private int mCmd = -1;

        public TmsEventReceiver() {
        }

        @Override
        public void run() {
            try {
                LocalServerSocket lServer = new LocalServerSocket(SOCKET_NAME);

                while( mRun )
                {
                    final LocalSocket lSocket = lServer.accept();
                    if( null == lSocket )
                        continue;

                    mLoop = true;

//                    while( mLoop )
                    {
                        byte[] szData = ReadByte(lSocket);
                        if( null == szData || szData.length == 0 ) {
                            lSocket.close();
                            continue;
                        }

                        mCmd = szData[0] & 0xFF;
                        Log.i(VD_DTAG, String.format(Locale.US, ">>> Receive Command ( cmd: %d, length: %d )", mCmd, szData.length));

                        //
                        //  Change Mode
                        //
                        if( CinemaTask.TMS_MODE_CHANGE <= mCmd &&
                            CinemaTask.TMS_MODE_DELETE > mCmd ) {

                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_CHANGE_MODE,
                                    getApplicationContext(),
                                    mCmd,
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            CinemaLoading.Show(getApplicationContext());
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            if( 0 > (Integer)values[0] )
                                                Log.i(VD_DTAG, "Fail, Change Mode.");
                                            else
                                                Log.i(VD_DTAG, String.format("Change Mode Done. ( mode = %d )", (Integer)values[0] + 1));

                                            if( mTmsEventCallback != null )
                                                mTmsEventCallback.onTmsEventCallback( (Integer[])values );

                                            Write(lSocket, (0 > (Integer)values[0]) ? RET_FAIL : RET_PASS );
                                            CinemaLoading.Hide();
                                            mLoop = false;
                                        }
                                    },
                                    null
                            );
                        }

                        if( CinemaTask.TMS_SCREEN_CHANGE <= mCmd &&
                            CinemaTask.TMS_SCREEN > mCmd ) {
                            final boolean bScale2K= (
                                    CinemaTask.TMS_P25_2K_2D == mCmd || CinemaTask.TMS_P25_2K_3D == mCmd ||
                                    CinemaTask.TMS_P33_2K_2D == mCmd || CinemaTask.TMS_P33_2K_3D == mCmd );
                            final boolean bMode3D = (
                                    CinemaTask.TMS_P25_2K_3D == mCmd || CinemaTask.TMS_P25_4K_3D == mCmd ||
                                    CinemaTask.TMS_P33_2K_3D == mCmd || CinemaTask.TMS_P33_4K_3D == mCmd );

                            //
                            //  Do not allow to change pitch ( P25 <--> P33 )
                            //
                            // final boolean bPitch25= (
                            //         CinemaTask.TMS_P25_4K_2D == mode || CinemaTask.TMS_P25_2K_2D == mode ||
                            //         CinemaTask.TMS_P25_4K_3D == mode || CinemaTask.TMS_P25_2K_3D == mode );

                            //
                            //  Change Scale ( 2K <-> 4K )
                            //
                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_CHANGE_SCALE,
                                    getApplicationContext(),
                                    bScale2K,
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            CinemaLoading.Show(getApplicationContext());
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            Log.i(VD_DTAG, String.format("Change Scale Done. ( mode = %d, is2K = %b )", mCmd, bScale2K));

                                            if( mTmsEventCallback != null )
                                                mTmsEventCallback.onTmsEventCallback( new Integer[]{ mCmd } );

                                            CinemaLoading.Hide();
                                        }
                                    },
                                    null
                            );

                            //
                            //  Change 3D/2D ( 2D <-> 3D )
                            //
                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_CHANGE_3D,
                                    getApplicationContext(),
                                    bMode3D,
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            CinemaLoading.Show(getApplicationContext());
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            Log.i(VD_DTAG, String.format("Change 3D Done. ( mode = %d, is3D = %b )", mCmd, bMode3D));

                                            if( mTmsEventCallback != null )
                                                mTmsEventCallback.onTmsEventCallback( new Integer[]{ mCmd } );

                                            Write(lSocket, RET_PASS);
                                            CinemaLoading.Hide();

                                            mLoop = false;
                                        }
                                    },
                                    null
                            );
                        }

                        //
                        //
                        //
                        if( CinemaTask.TMS_SCREEN <= mCmd &&
                            CinemaTask.TMS_CONFIG > mCmd ) {
                            final boolean bMute = (CinemaTask.TMS_SCREEN_OFF == mCmd);
                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_SCREEN_MUTE,
                                    getApplicationContext(),
                                    bMute,
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            CinemaLoading.Show(getApplicationContext());
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            Log.i(VD_DTAG, String.format("Screen Mute Done. ( mode = %d, isMute = %b )", mCmd, bMute));

                                            if( mTmsEventCallback != null )
                                                mTmsEventCallback.onTmsEventCallback( new Integer[]{ mCmd } );

                                            Write(lSocket, RET_PASS);
                                            CinemaLoading.Hide();

                                            mLoop = false;
                                        }
                                    },
                                    null
                            );
                        }

                        //
                        //
                        //
                        if( CinemaTask.TMS_CONFIG_UPLOAD == mCmd ) {
                            byte[] inData = new byte[szData.length-1];
                            System.arraycopy( szData, 1, inData, 0, szData.length-1);

                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_CONFIG_UPLOAD,
                                    getApplicationContext(),
                                    inData,
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            CinemaLoading.Show(getApplicationContext());
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            if( mTmsEventCallback != null )
                                                mTmsEventCallback.onTmsEventCallback( new Integer[]{ mCmd, (Integer)values[0] } );

                                            Write(lSocket, (0 > (Integer)values[0]) ? RET_FAIL : RET_PASS );

                                            CinemaLoading.Hide();
                                            mLoop = false;
                                        }
                                    },
                                    null
                            );
                        }

                        //
                        //  Delete Configuration
                        //
                        if( CinemaTask.TMS_MODE_DELETE <= mCmd &&
                            CinemaTask.TMS_MODE_DELETE_29 >= mCmd ) {

                            if( CinemaTask.TMS_MODE_DELETE_ALL == mCmd ) {
                                for( int i = 10; i < 30; i++ ) {
                                    final int pos = i;
                                    CinemaTask.GetInstance().Run(
                                            CinemaTask.CMD_CONFIG_DELETE,
                                            getApplicationContext(),
                                            pos,
                                            new CinemaTask.PreExecuteCallback() {
                                                @Override
                                                public void onPreExecute(Object[] values) {
                                                    CinemaLoading.Show(getApplicationContext());
                                                }
                                            },
                                            new CinemaTask.PostExecuteCallback() {
                                                @Override
                                                public void onPostExecute(Object[] values) {
                                                    if( !(values instanceof Integer[]) )
                                                        return;

                                                    if( mTmsEventCallback != null && ((Integer)values[0] == CinemaInfo.RET_PASS) )
                                                        mTmsEventCallback.onTmsEventCallback( (Integer[])new Integer[]{mCmd} );

                                                    CinemaLoading.Hide();

                                                    if( pos == 29 ) {
                                                        Write(lSocket, RET_PASS);
                                                        mLoop = false;
                                                    }
                                                }
                                            },
                                            null
                                    );
                                }
                            }
                            else {
                                CinemaTask.GetInstance().Run(
                                        CinemaTask.CMD_CONFIG_DELETE,
                                        getApplicationContext(),
                                        mCmd - CinemaTask.TMS_MODE_DELETE_10 + 10,
                                        new CinemaTask.PreExecuteCallback() {
                                            @Override
                                            public void onPreExecute(Object[] values) {
                                                CinemaLoading.Show(getApplicationContext());
                                            }
                                        },
                                        new CinemaTask.PostExecuteCallback() {
                                            @Override
                                            public void onPostExecute(Object[] values) {
                                                if( !(values instanceof Integer[]) )
                                                    return;

                                                if( mTmsEventCallback != null && ((Integer)values[0] == CinemaInfo.RET_PASS) )
                                                    mTmsEventCallback.onTmsEventCallback( (Integer[])new Integer[]{mCmd} );

                                                CinemaLoading.Hide();

                                                Write(lSocket, RET_PASS);
                                                mLoop = false;
                                            }
                                        },
                                        null
                                );
                            }
                        }

                        //
                        //  Download Operation operate in TMS_Server().
                        //
                    }

                    //lSocket.close();
                }

                lServer.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public synchronized void Start() {
            if( !mRun ) {
                mRun = true;

                start();
            }
        }

        public synchronized void Stop() {
            if( mRun ) {
                mLoop   = false;
                mRun    = false;
                WriteDummy(SOCKET_NAME);
            }
        }
    }

    private static final String RET_PASS = "pass";
    private static final String RET_FAIL = "fail";

    private boolean IsLogPrint() {
        return false;
    }

    private String ReadString(LocalSocket socket) {
        byte[] readData = new byte[1024];
        String message = "";
        int readSize = 0;
        try {
            readSize = socket.getInputStream().read(readData);
            message = new String(readData, 0, readSize);
            if( IsLogPrint() ) {
                Log.i(VD_DTAG, String.format(Locale.US, ">>> Recv %d bytes: %s", readSize, message));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return message;
    }

    private byte[] ReadByte(LocalSocket socket) {
        byte[] result = new byte[0];
        try {
            byte[] readData = new byte[65535];
            int readSize;
            int expectedSize = 0;

            do {
                readSize = socket.getInputStream().read(readData);

                if( result.length == 0)
                    expectedSize = ((readData[0] & 0xFF) << 8) | (readData[1] & 0xFF);

                byte[] temp = new byte[result.length+readSize];

                System.arraycopy( result, 0, temp, 0, result.length );
                System.arraycopy( readData, (result.length == 0) ? 2 : 0, temp, result.length, readSize );

                result = temp;
            } while(result.length < expectedSize);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }

    private void Write(LocalSocket socket, String message) {
        try {
            socket.getOutputStream().write(message.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }

        if( IsLogPrint() )
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Send %d bytes: %s", message.length(), message));
    }

    private void Write(String name, String message) {
        try {
            LocalSocket socket = new LocalSocket();
            socket.connect(new LocalSocketAddress(name));
            OutputStream outStream = socket.getOutputStream();
            outStream.write(message.getBytes());
            outStream.close();
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void WriteDummy(String name) {
        try {
            String message = "";
            LocalSocket socket = new LocalSocket();
            socket.connect(new LocalSocketAddress(name));
            OutputStream outStream = socket.getOutputStream();
            outStream.write(message.getBytes());
            outStream.close();
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
