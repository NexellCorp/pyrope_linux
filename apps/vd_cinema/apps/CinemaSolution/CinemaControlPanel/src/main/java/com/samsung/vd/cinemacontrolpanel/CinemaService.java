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
import java.lang.ref.WeakReference;

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
                    if( null != lSocket ) {
                        if( isDetectTamper ) {
                            String strEvent = Read(lSocket.getInputStream());

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
                        }

                        lSocket.close();
                    }
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
                    if( null != lSocket ) {
                        String strEvent = Read(lSocket.getInputStream());
                        String strAlive = ((CinemaInfo)getApplicationContext()).GetSecureAlive();

                        if( strEvent.equals("Alive") && strAlive.equals("false") ) {
                            ((CinemaInfo)getApplicationContext()).SetSecureAlive("true");
                        }

                        lSocket.close();
                    }
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
            sender.close();
        }
    }

    //
    //  For Tms Event Receiver
    //
    private class TmsEventReceiver extends Thread {
        private String SOCKET_NAME = "cinema.tms";
        private boolean mRun = false;

        public TmsEventReceiver() {
        }

        @Override
        public void run() {
            try {
                LocalServerSocket lServer = new LocalServerSocket(SOCKET_NAME);

                while( mRun )
                {
                    LocalSocket lSocket = lServer.accept();
                    if( null != lSocket ) {
                        String strValue = Read(lSocket.getInputStream());
                        if( null != strValue ) {
                            final int mode = Integer.parseInt(strValue, 10);

                            if( CinemaTask.CMD_TMS_QUE > mode ) {
                                CinemaTask.GetInstance().Run(
                                        CinemaTask.CMD_CHANGE_MODE,
                                        getApplicationContext(),
                                        mode,
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

                                                CinemaLoading.Hide();
                                            }
                                        },
                                        null
                                );
                            }

                            if( CinemaTask.CMD_TMS_QUE <= mode ) {
                                final boolean bScale2K= (
                                        CinemaTask.TMS_P25_2K_2D == mode || CinemaTask.TMS_P25_2K_3D == mode ||
                                        CinemaTask.TMS_P33_2K_2D == mode || CinemaTask.TMS_P33_2K_3D == mode );
                                final boolean bMode3D = (
                                        CinemaTask.TMS_P25_2K_3D == mode || CinemaTask.TMS_P25_4K_3D == mode ||
                                        CinemaTask.TMS_P33_2K_3D == mode || CinemaTask.TMS_P33_4K_3D == mode );

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

                                                Log.i(VD_DTAG, String.format("Change Scale Done. ( mode = %d, is2K = %b )", mode, bScale2K));

                                                if( mTmsEventCallback != null )
                                                    mTmsEventCallback.onTmsEventCallback( new Integer[]{ mode } );

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

                                                Log.i(VD_DTAG, String.format("Change 3D Done. ( mode = %d, is3D = %b )", mode, bMode3D));

                                                if( mTmsEventCallback != null )
                                                    mTmsEventCallback.onTmsEventCallback( new Integer[]{ mode } );

                                                CinemaLoading.Hide();
                                            }
                                        },
                                        null
                                );
                            }
                        }
                        lSocket.close();
                    }
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
            sender.close();
        }
    }
}
