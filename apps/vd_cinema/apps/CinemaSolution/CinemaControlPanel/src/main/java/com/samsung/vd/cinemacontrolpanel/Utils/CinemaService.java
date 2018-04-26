package com.samsung.vd.cinemacontrolpanel.Utils;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;

import com.samsung.vd.cinemacontrolpanel.Activity.LoginActivity;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.SelectRunAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigPfpgaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedGammaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedUniformityInfo;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ref.WeakReference;
import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_AsyncTaskChangeContents;

/**
 * Created by doriya on 11/9/16.
 */
public class CinemaService extends Service {
    private static final String VD_DTAG = "CinemaService";

    private final IBinder mBinder = new LocalBinder();
    private final TamperEventReceiver mTamperEventReceiver = new TamperEventReceiver();
    private final SecureEventReceiver mSecureEventReceiver = new SecureEventReceiver();
    private final ContentsChangeReceiver mContentsChangeReceiver = new ContentsChangeReceiver();

    private final ScreenSaverHandler mHandler = new ScreenSaverHandler(this);

    public class LocalBinder extends Binder {
        public CinemaService GetService() {
            return CinemaService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        RefreshScreenSaver();
        mTamperEventReceiver.Start();
        mSecureEventReceiver.Start();
        mContentsChangeReceiver.Start();

        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        mTamperEventReceiver.Stop();
        mSecureEventReceiver.Stop();
        mContentsChangeReceiver.Stop();

        return super.onUnbind(intent);
    }

    public ChangeContentsCallback mChangeContentsCallback;
    public static class ChangeContentsCallback {
        public void onChangeContentsCallback( int mode ) {

        };
    }

    public void RegisterCallback( ChangeContentsCallback callback ) {
        mChangeContentsCallback = callback;
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
    private static final int ChangeContentsStart  = 2;
    private static final int ChangeContentsEnd  = 3;

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

            case ChangeContentsStart:
                Log.i(VD_DTAG, String.format( Locale.US, "Change Contents. ( %d )",  NXAsync.getInstance().getSocketVal()));
                CinemaLoading2.Show( getApplicationContext() );
                break;

            case ChangeContentsEnd:
                Log.i(VD_DTAG, String.format( Locale.US, "Change Contents Done."));
                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(NXAsync.getInstance().getSocketVal()));
                if( mChangeContentsCallback != null )
                    mChangeContentsCallback.onChangeContentsCallback( NXAsync.getInstance().getSocketVal() );
                CinemaLoading2.Hide();
                break;
        }
    }

    public void TurnOff() {
        mHandler.removeMessages(KEY_SCREEN_LOG_OUT);
        mHandler.removeMessages(KEY_SCREEN_TURN_OFF);
        //ChangeContentsStart remove?
        //ChangeContentsEnd remove?
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

    private class ContentsChangeReceiver extends Thread {
        private String SOCKET_NAME = "cinema.change.contents";
        private boolean mRun = false;

        public ContentsChangeReceiver() {
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

                        try {
                            NXAsync.getInstance().setSocketVal(Integer.parseInt(strValue, 10));
                            //                        new AsyncTaskChangeContents(getApplicationContext(),Integer.parseInt(strValue, 10)).execute();
                            NXAsync.getInstance().Execute(CMD_AsyncTaskChangeContents , asyncCallback);
                        } catch (NumberFormatException e) {
                            e.printStackTrace();
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


    NXAsync.AsyncCallback asyncCallback = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            mHandler.sendEmptyMessage(ChangeContentsStart);

        }

        @Override
        public void onPostExe() {
            mHandler.sendEmptyMessage(ChangeContentsEnd);
        }
    };
}
