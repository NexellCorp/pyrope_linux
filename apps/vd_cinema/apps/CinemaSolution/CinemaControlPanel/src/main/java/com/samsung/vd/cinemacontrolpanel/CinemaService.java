package com.samsung.vd.cinemacontrolpanel;

import android.app.Dialog;
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
import android.view.ViewGroup;
import android.widget.ProgressBar;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
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
    private final ContentsChangeReceiver mContentsChangeReceiver = new ContentsChangeReceiver();

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

                        if( strValue.equals("0") || strValue.equals("1") ) {
                            new AsyncTaskChangeContents(getApplicationContext(),Integer.parseInt(strValue, 10)).execute();
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

    private class AsyncTaskChangeContents extends AsyncTask<Void, Void, Void> {
        private Context mContext;
        byte[] mCabinet;
        private int mMode;

        public AsyncTaskChangeContents(Context context, int mode ) {
            mContext = context;
            mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
            mMode = mode;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            //
            //  Check TCON Booting Status
            //
            if( ((CinemaInfo)getApplicationContext()).IsCheckTconBooting() ) {
                boolean bTconBooting = true;
                for( byte id : mCabinet ) {
                    byte[] result;
                    result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                    if (result == null || result.length == 0) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / port: %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id));
                        continue;
                    }

                    if( result[0] == 0 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / port: %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id, result[0] ));
                        bTconBooting = false;
                    }
                }

                if( !bTconBooting ) {
                    Log.i(VD_DTAG, "Fail, TCON booting.");
                    return null;
                }
            }

            //
            //  PFPGA Mute on
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

            //
            //  Parse P_REG.txt
            //
            String[] resultPath;
            boolean enableUniformity = false;
            boolean[] enableGamma = {false, };

            resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : resultPath ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    enableUniformity = info.GetEnableUpdateUniformity(mMode);
                    for( int i = 0; i < info.GetRegister(mMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            //
            //  Auto Uniformity Correction Writing
            //
            resultPath = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
            for( String file : resultPath ) {
                LedUniformityInfo info = new LedUniformityInfo();
                if( info.Parse( file ) ) {
                    if( !enableUniformity ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Uniformity. ( %s )", file ));
                        continue;
                    }

                    byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                    ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                }
            }

            //
            //  Parse T_REG.txt
            //
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                ConfigTconInfo info = new ConfigTconInfo();
                if( info.Parse( file ) ) {
                    enableGamma = info.GetEnableUpdateGamma(mMode);

                    for( int i = 0; i < info.GetRegister(mMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }
            }

            //
            //  Write Gamma
            //
            resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && !enableGamma[0]) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && !enableGamma[1]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && !enableGamma[2]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && !enableGamma[3]) ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Gamma. ( %s )", file ));
                        continue;
                    }

                    int cmd;
                    if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                        cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                    else
                        cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                    byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                    byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                    byte[] inData = ctrl.AppendByteArray(table, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                    if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                }
            }

            //
            //  PFPGA Mute off
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
            return null;

        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            Log.i(VD_DTAG, String.format( Locale.US, "Change Contents. ( %d )", mMode));
            CinemaLoading2.Show( mContext );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            Log.i(VD_DTAG, String.format( Locale.US, "Change Contents Done."));
            CinemaLoading2.Hide();
        }
    }
}
