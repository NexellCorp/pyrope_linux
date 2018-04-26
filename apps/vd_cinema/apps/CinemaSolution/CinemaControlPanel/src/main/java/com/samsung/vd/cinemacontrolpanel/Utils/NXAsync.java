package com.samsung.vd.cinemacontrolpanel.Utils;

import android.content.Context;
import android.os.AsyncTask;
import android.os.SystemClock;
import android.util.Log;

import com.samsung.vd.cinemacontrolpanel.BuildConfig;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.CheckRunAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.LedPosAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeExpandableAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.CheckRunInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedDotCorrectInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedPosInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.DiagnosticsAsyncParam;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.DisplayCheckAsyncParam;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.DisplayModeAsyncParam;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.StatusDetailAdapterAsyncParam;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.TconInfoCarrier;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigPfpgaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedGammaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedUniformityInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams.VersionAsyncParam;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusDescribeExpandableInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusDescribeInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.StatusSimpleInfo;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Vector;
import java.util.concurrent.Semaphore;


/**
 * Created by spjun on 18. 3. 15.
 */

public class NXAsync {
    private static String NxTag = "NXAsync";
    private SendCommand sendCommand;
    private AsyncCallback dummyAsyncCallback = new AsyncCallback();
    private Vector<Integer> cmdList = new Vector<>();
    private Vector<AsyncCallback> callbackList = new Vector<>();
    private Semaphore semaphore = new Semaphore(0);
    private Semaphore asyncSemaphore= new Semaphore(0);
    private boolean isThreadRunning = false;
    private boolean stopFlag = false;
    private boolean clearFlag = false;
    private Context appContext;

    public Semaphore getAsyncSemaphore() {
        return asyncSemaphore;
    }
    private static NXAsync ourInstance = new NXAsync();
    private NXAsync() {
        Log.d(NxTag,"constructor ++++");
        sendCommand = new SendCommand();

        thread.start();
        Log.d(NxTag,"constructor ----");
    }

    public static NXAsync getInstance() {
        return ourInstance;
    }

    public void setAppContext(Context context) {
        if(null == context) {
            Log.e(NxTag , "setAppContext , context input is null");
            return;
        }
        appContext = context;
    }


    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            isThreadRunning = true;
            Log.d(NxTag, "NXAsync thread start!!!");
            //this thread starts automatically by CinemaInfo.onCreate()

            while(true) //while app is running
            {
                //wait for semaphore
                // release cases..
                // singletonInstance->Execute(cmd)  -----> semaphore ++
                // onPostExecute of AsyncTask       -----> semaphore ++
                // stopThread()                     -----> semaphore ++ ---> stopFlag = true ---> break;
                try {
                    semaphore.acquire();
                    if(stopFlag) {
                        cmdList.clear();
                        callbackList.clear();
                        stopFlag = false;
                        break;
                    }
                    if(clearFlag) {
                        cmdList.clear();
                        callbackList.clear();
                        clearFlag = false;
                    }

                    //list check.. exist ----> check asynctask status
                    if(0 != cmdList.size()){
                        //task finished
                        if( AsyncTask.Status.FINISHED == sendCommand.getStatus() ){
                            Log.d(NxTag, "Status.FINISHED");
                            sendCommand = new SendCommand();
                            sendCommand.execute(cmdList.get(0));
                        }
                        //task not started
                        else if( AsyncTask.Status.PENDING == sendCommand.getStatus() ){
                            Log.d(NxTag, "Status.PENDING");
                            sendCommand.execute(cmdList.get(0));
                        }
                        //task is running
                        else if( AsyncTask.Status.RUNNING == sendCommand.getStatus() ){
                            Log.d(NxTag, "Status.RUNNING , time : "+System.currentTimeMillis());
                            if(1 == cmdList.size()) {
                                semaphore.release();
                            }
                        }
                        else
                        {
                            Log.e(NxTag,"AsyncTask status check ERROR");
                            Log.e(NxTag,"AsyncTask status check ERROR");
                            Log.e(NxTag,"AsyncTask status check ERROR");
                        }
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            Log.d(NxTag, "NXAsync thread finished!!!");
            isThreadRunning = false;
        }
    });

    public void startThread() {
        if(isThreadRunning) {
            try {
                stopThread();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        thread.start();
    }

    public void stopThread() throws InterruptedException {
        stopFlag = true;
        semaphore.release();

        try {
            thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void clearCommandList() {
        clearFlag = true;
        semaphore.release();
    }

    public boolean getIsThreadRunning(){
        return isThreadRunning;
    }


    public static final int CMD_Initial                     = 0x01;
    public static final int CMD_Login                       = 0x02;
    public static final int CMD_TopEEPRomRead               = 0x03;
    public static final int CMD_TopInitMode                 = 0x04;
    public static final int CMD_DiagnosticsTconStatus       = 0x05;
    public static final int CMD_DiagnosticsTconLvds         = 0x06;
    public static final int CMD_DiagnosticsLedOpenNum       = 0x07;
    public static final int CMD_DiagnosticsFullWhite        = 0x08;
    public static final int CMD_DiagnosticsFullWhiteBackup  = 0x09;
    public static final int CMD_DiagnosticsLedOpenDetect    = 0x0A;
    public static final int CMD_DiagnosticsLodRemovalWrite  = 0x0B;
    public static final int CMD_DiagnosticsLodRemovalRead   = 0x0C;
    public static final int CMD_DiagnosticsCabinetDoor      = 0x0D;
    public static final int CMD_DiagnosticsPeriPheral       = 0x0E;
    public static final int CMD_DiagnosticsVersion          = 0x0F;
    public static final int CMD_DisplayCheckTestPattern     = 0x10;
    public static final int CMD_DisplayCheckAccumulation    = 0x11;
    public static final int CMD_DisplayModeCheckCabinet     = 0x12;
    public static final int CMD_DisplayModeMasteringWrite   = 0x13;
    public static final int CMD_DisplayModeMasteringRead    = 0x14;
    public static final int CMD_DisplayModeUniformityCorrection = 0x15;
    public static final int CMD_DisplayModeImageQuality     = 0x16;
    public static final int CMD_DisplayModeAdapterDotCorrection = 0x17;
    public static final int CMD_DisplayModeDotCorrection    = 0x18;
    public static final int CMD_DisplayModeDotCorrectionExtract = 0x19;
    public static final int CMD_DisplayModeWhiteSeamRead    = 0x1A;
    public static final int CMD_DisplayModeWhiteSeamEmulate = 0x1B;
    public static final int CMD_DisplayModeWhiteSeamWrite   = 0x1C;
    //UI
    public static final int CMD_StatusDetailAdapterLedPos   = 0x1D;
    public static final int CMD_Version                     = 0x1E;
    public static final int CMD_DisplayCheckStopTestPattern = 0x1F;
    public static final int CMD_DisplayModeUniformityBtn    = 0x20;
    public static final int CMD_DisplayModeInitWhiteSeam    = 0x21;
    public static final int CMD_DisplayModeApplyMastering   = 0x22;
    public static final int CMD_DisplayModeUpdateUnif       = 0x23;
    public static final int CMD_DisplayModeonCheckchanged   = 0x24;

    public static final int CMD_DisplayCheckUpdateTestPattern   = 0x25;
    public static final int CMD_DisplayModeGlobalRead           = 0x26;
    public static final int CMD_DisplayModeLodReset             = 0x27;
    public static final int CMD_DisplayModeApplySyncWidth       = 0x28;
    public static final int CMD_DisplayModeApplySyncDelay       = 0x29;
    public static final int CMD_DisplayModeApplySyncReverse     = 0x2A;
    public static final int CMD_DisplayModeApplyScale           = 0x2B;
    public static final int CMD_DisplayModeApplyZeroScale       = 0x2C;
    public static final int CMD_DisplayModeApplySeam            = 0x2D;
    public static final int CMD_DisplayModeApplyModule          = 0x2E;
    public static final int CMD_DisplayModeApplyXyzInput        = 0x2F;
    public static final int CMD_DisplayModeApplyLedOpenDetection= 0x30;
    public static final int CMD_DisplayModeApplyLodRemoval      = 0x31;


    public static final int CMD_AsyncTaskChangeContents      = 0x32;


    public synchronized void Execute(Integer cmd, AsyncCallback asyncCallback){
        Log.d(NxTag,"Execute +++++");

        cmdList.add(cmd);
        if(null == asyncCallback){
            callbackList.add(dummyAsyncCallback);
        }else{
            callbackList.add(asyncCallback);
        }
        semaphore.release();
    }

    public boolean isI2CFailed() {
        return sendCommand.getI2CFail();
    }

    private class SendCommand extends AsyncTask<Integer, Void, Void>{
        private boolean bI2CFail = false;
        long time;
        public boolean getI2CFail() {
            return bI2CFail;
        }

        public void onProgressUpdate() {
            if( 0 != callbackList.size() ){
               callbackList.get(0).onProgress( );
                try {
                    asyncSemaphore.acquire();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            time = System.currentTimeMillis();
            Log.d(NxTag,"onPreExecute +++++  time : " + time);

            if( 0 != callbackList.size() ){
                callbackList.get(0).onPreExe( );
                try {
                    asyncSemaphore.acquire();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        protected synchronized void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);


            if( 0 != callbackList.size() ){
                callbackList.get(0).onPostExe( );
                try {
                    asyncSemaphore.acquire();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            cmdList.remove(0);
            callbackList.remove(0);
            Log.d(NxTag,"onPostExecute ----- semaphore release , time diff : " + (System.currentTimeMillis() - time));
            semaphore.release();
        }

        @Override
        protected void onCancelled(Void aVoid) {
            super.onCancelled(aVoid);
            Log.d(NxTag,"onCancelled(aVoid)");
//            semaphore.release();
        }

        @Override
        protected void onCancelled() {
            super.onCancelled();
            Log.d(NxTag,"onCancelled");
        }

        @Override
        protected Void doInBackground(Integer... integers) {
            
            int cmd = integers[0];
            Log.d(NxTag , "doInBackground , cmd : "+cmd);
            bI2CFail = false;

            byte[] ret = ctrl.Send(NxCinemaCtrl.CINEMA_CMD_BURST_START , null);
            if (null == ret) {
                Log.i(NxTag , "CINEMA_CMD_BURST_START failed,, ret is null");
                //CinemaAlert.Show( appContext, "Alert", "BURST_START failed, ret is null" , CinemaAlert.TYPE_NXAsync, null);
                bI2CFail = true;
            }
            else if ( RESOURCE_BUSY == ctrl.ByteArrayToInt(ret) ) {
                Log.i(NxTag , "CINEMA_CMD_BURST_START failed,, I2C busy.");
                //CinemaAlert.Show( appContext, "Alert", "BURST_START failed, I2C busy , try again later." , CinemaAlert.TYPE_NXAsync, null);
                bI2CFail = true;
            }
            else if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(ret) ) {
                Log.i(NxTag , "CINEMA_CMD_BURST_START failed,, Unknown Error");
                //CinemaAlert.Show( appContext, "Alert", "BURST_START failed, Unknown Err" , CinemaAlert.TYPE_NXAsync, null);
                bI2CFail = true;
            }

            if(bI2CFail) return null;
            //if(null == mCabinet && 0x01 != cmd) DetectCabinet();

            switch (cmd) {
                case 0x01: // CMD_Initial
                    DetectCabinet();
                    return null;

                case 0x02: // CMD_Login
                    LoginAsyncTaskCheckCabinet();
                    break;

                case 0x03: // CMD_TopEEPRomRead   not used at the moment..
                    TopAsyncTaskEEPRomRead();
                    break;

                case 0x04: // CMD_TopInitMode
                    TopAsyncTaskInitMode();
                    break;

                case 0x05: // CMD_DiagnosticsTconStatus
                    DiagnosticsAsyncTaskTconStatus();
                    break;

                case 0x06: // CMD_DiagnosticsTconLvds
                    DiagnosticsAsyncTaskTconLvds();
                    break;

                case 0x07: // CMD_DiagnosticsLedOpenNum
                    DiagnosticsAsyncTaskLedOpenNum();
                    break;

                case 0x08: // CMD_DiagnosticsFullWhite
                    DiagnosticsAsyncTaskFullWhite();
                    break;

                case 0x09 : // CMD_DiagnosticsFullWhiteBackup
                    DiagnosticsAsyncTaskFullWhiteBackup();
                    break;

                case 0x0A : // CMD_DiagnosticsLedOpenDetect
                    DiagnosticsAsyncTaskLedOpenDetect();
                    break;

                case 0x0B : // CMD_DiagnosticsLodRemovalWrite
                    DiagnosticsAsyncTaskLodRemovalWrite();
                    break;

                case 0x0C : // CMD_DiagnosticsLodRemovalRead
                    DiagnosticsAsyncTaskLodRemovalRead();
                    break;

                case 0x0D : // CMD_DiagnosticsCabinetDoor
                    DiagnosticsAsyncTaskCabinetDoor();
                    break;

                case 0x0E : // CMD_DiagnosticsPeriPheral
                    DiagnosticsAsyncTaskPeripheral();
                    break;

                case 0x0F : // CMD_DiagnosticsVersion
                    DiagnosticsAsyncTaskVersion();
                    break;

                case 0x10 : // CMD_DisplayCheckTestPattern
                    DisplayCheckAsyncTaskTestPattern();
                    break;

                case 0x11 : // CMD_DisplayCheckAccumulation
                    DisplayCheckAsyncTaskAccumulation();
                    break;

                case 0x12 : // CMD_DisplayModeCheckCabinet
                    DisplayModeAsyncTaskCheckCabinet();
                    break;

                case 0x13 : // CMD_DisplayModeMasteringWrite
                    DisplayModeAsyncTaskMasteringWrite();
                    break;

                case 0x14 : // CMD_DisplayModeMasteringRead
                    DisplayModeAsyncTaskMasteringRead();
                    break;

                case 0x15 : // CMD_DisplayModeUniformityCorrection
                    DisplayModeAsyncTaskUniformityCorrection();
                    break;

                case 0x16 : // CMD_DisplayModeImageQuality
                    DisplayModeAsyncTaskImageQuality();
                    break;

                case 0x17 : // CMD_DisplayModeAdapterDotCorrection
                    DisplayModeAsyncTaskAdapterDotCorrection();
                    break;

                case 0x18 : // CMD_DisplayModeDotCorrection
                    DisplayModeAsyncTaskPixelCorrection();
                    break;

                case 0x19 : // CMD_DisplayModeDotCorrectionExtract
                    DisplayModeAsyncTaskPixelCorrectionExtract();
                    break;

                case 0x1A : // CMD_DisplayModeWhiteSeamRead
                    DisplayModeAsyncTaskWhiteSeamRead();
                    break;

                case 0x1B : // CMD_DisplayModeWhiteSeamEmulate
                    DisplayModeAsyncTaskWhiteSeamEmulate();
                    break;

                case 0x1C : // CMD_DisplayModeWhiteSeamWrite
                    DisplayModeAsyncTaskWhiteSeamWrite();
                    break;

                case 0x1D : // CMD_StatusDetailAdapterLedPos
                    StatusDetailAdapterAsyncTaskLedPos();
                    break;

                case 0x1E : // CMD_Version
                    Version();
                    break;

                case 0x1F : // CMD_DisplayCheckStopTestPattern
                    StopTestPattern();
                    break;

                case 0x20 : // CMD_DisplayModeUniformityBtn
                    DisplayModeUniformityBtn();
                    break;

                case 0x21 : // CMD_DisplayModeInitWhiteSeam
                    DisplayModeInitWhiteSeam();
                    break;

                case 0x22 : // CMD_DisplayModeApplyMastering
                    DisplayModeApplyMastering();
                    break;

                case 0x23 : // CMD_DisplayModeUpdateUnif
                    DisplayModeUpdateUnif();
                    break;

                case 0x24 : // CMD_DisplayModeonCheckchanged
                    DisplayModeCheckChanged();
                    break;

                case 0x25 : // CMD_DisplayCheckUpdateTestPattern
                    DisplayCheckUpdateTestPattern();
                    break;

                case 0x26 : // CMD_DisplayModeGlobalRead
                    DisplayModeGlobalRead();
                    break;

                case 0x27 : // CMD_DisplayModeLodReset
                    DisplayModeLodReset();
                    break;

                case 0x28 : // CMD_DisplayModeApplySyncWidth
                    DisplayModeApplySyncWidth();
                    break;

                case 0x29 : // CMD_DisplayModeApplySyncDelay
                    DisplayModeApplySyncDelay();
                    break;

                case 0x2A : // CMD_DisplayModeApplySyncReverse
                    DisplayModeApplySyncReverse();
                    break;

                case 0x2B : // CMD_DisplayModeApplyScale
                    DisplayModeApplyScale();
                    break;

                case 0x2C : // CMD_DisplayModeApplyZeroScale
                    DisplayModeApplyZeroScale();
                    break;

                case 0x2D : // CMD_DisplayModeApplySeam
                    DisplayModeApplySeam();
                    break;

                case 0x2E : // CMD_DisplayModeApplyModule
                    DisplayModeApplyModule();
                    break;

                case 0x2F : // CMD_DisplayModeApplyXyzInput
                    DisplayModeApplyXyzInput();
                    break;

                case 0x30 : // CMD_DisplayModeApplyLedOpenDetection
                    DisplayModeApplyLedOpenDetection();
                    break;

                case 0x31 : // CMD_DisplayModeApplyLodRemoval
                    DisplayModeApplyLodRemoval();
                    break;

                case 0x32 : // CMD_AsyncTaskChangeContents
                    AsyncTaskChangeContents();
                    break;

                default:
                    break;
            }

            ret = ctrl.Send(NxCinemaCtrl.CINEMA_CMD_BURST_STOP , null);
            if (null == ret) {
                Log.i(NxTag , "CINEMA_CMD_BURST_STOP failed,, ret is null");
                //CinemaAlert.Show( appContext, "Alert", "BURST_STOP failed, ret is null" , CinemaAlert.TYPE_NXAsync, null);
            }
            else if ( RESOURCE_BUSY == ctrl.ByteArrayToInt(ret) ) {
                Log.i(NxTag , "CINEMA_CMD_BURST_STOP failed,, I2C busy.");
                //CinemaAlert.Show( appContext, "Alert", "BURST_STOP failed, I2C Busy" , CinemaAlert.TYPE_NXAsync, null);
            }
            else if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(ret) ) {
                Log.i(NxTag , "CINEMA_CMD_BURST_STOP failed,, Unknown Error");
                //CinemaAlert.Show( appContext, "Alert", "BURST_STOP failed, Unknown Err" , CinemaAlert.TYPE_NXAsync, null);
            }
            return null;
        } //doInBackground
    } //SendCommand class

    public static class AsyncCallback{
        public AsyncCallback() {}

        public void onPreExe() {
            Log.d(NxTag,"onPreExe  not overrided");
            NXAsync.getInstance().getAsyncSemaphore().release();
        }
        public void onProgress() {
            Log.d(NxTag,"onProgress  not overrided");
            NXAsync.getInstance().getAsyncSemaphore().release();
        }
        public void onPostExe() {
            Log.d(NxTag,"onPostExe  not overrided");
            NXAsync.getInstance().getAsyncSemaphore().release();
        }
    }

    //
    // Common Methods and Vars
    //
    private static final int UNKNOWN_ERR    = 0xFFFFFFFF;
    private static final int RESOURCE_BUSY  = 0xFFFFFFFE;
    private static final int FAILED         = 0x00000000;   //Invalid status
    private static final int SUCCESS        = 0x00000001;   //Valid status

    private NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
    private byte[] mCabinet;                //valid after DetectCabinet()
    private boolean bValidPort0 = false;    //valid after DetectCabinet()
    private boolean bValidPort1 = false;    //valid after DetectCabinet()


    private void DetectCabinet() {  //LoginAsyncTaskCheckCabinet
        for( int i = 0; i < 255; i++ ) {
            if( (i & 0x7F) < CinemaInfo.TCON_ID_OFFSET )
                continue;

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
            if ( result == null ) continue;
            else if ( SUCCESS != ctrl.ByteArrayToInt(result) ) {
                //Log.i(NxTag, "CMD_TCON_STATUS failed.");
                continue;
            }

            ((CinemaInfo)appContext).AddCabinet( (byte)i );

            Log.i(NxTag ,"ctrl.ByteArrayToInt(result) : "+ctrl.ByteArrayToInt(result) );
            Log.i(NxTag, String.format(Locale.US, "Add Cabinet ( Cabinet : %d, port : %d, slave : 0x%02x )", (i & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (i & 0x80) >> 7, i & 0x7F ));
        }
        ((CinemaInfo)appContext).SortCabinet();
        mCabinet = ((CinemaInfo)appContext).GetCabinet();

        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }
    } //DetectCabinet

    private boolean CheckTCONBootingStatus() {
        if( ((CinemaInfo)appContext).IsCheckTconBooting() ) {
            for( byte id : mCabinet ) {
                byte[] result;
                result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                if ( result == null ) {
                    Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / port: %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id));
                    return false;
                }

                if( SUCCESS != ctrl.ByteArrayToInt(result) ) {
                    Log.i(NxTag, "CMD_TCON_BOOTING_STATUS failed.");
                    Log.i(NxTag, String.format(Locale.US, "Fail. ( cabinet : %d / port: %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id, result[0] ));
                    return false;
                }
            }
        }
        return true;
    } //CheckTCONBootingStatus

    private int ParsePREGtxt(int mInitMode) {
        String[] resultPath;
        int enableUniformity = 0;
        resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        for( String file : resultPath ) {
            ConfigPfpgaInfo info = new ConfigPfpgaInfo();
            if( info.Parse( file ) ) {
                enableUniformity = info.GetEnableUpdateUniformity(mInitMode);
                for( int i = 0; i < info.GetRegister(mInitMode).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(info.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(info.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, data);
                    byte[] result = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );

                    if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_PFPGA_REG_WRITE Unknown Error.");
                    if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_PFPGA_REG_WRITE failed.");
                }
            }
        }
        return enableUniformity;
    } //ParsePREGtxt

    private void AutoUniformityCorrectionWriting(int enableUniformity) {
        String[] resultPath;
        resultPath = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
        for( String file : resultPath ) {
            LedUniformityInfo info = new LedUniformityInfo();
            if( info.Parse( file ) ) {
//                if( 0 == enableUniformity ) {
//                    Log.i(NxTag, String.format( "Skip. Update Uniformity. ( %s )", file ));
//                    continue;
//                }
                byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_PFPGA_UNIFORMITY_DATA Unknown Error.");
            }
        }
    } //AutoUniformityCorrectionWriting

    private int[] ParseTREGtxt(int mInitMode , ConfigTconInfo tconEEPRomInfo , ConfigTconInfo tconUsbInfo , boolean bParse) {
        String[] resultPath;
        int[] enableGamma = {0, 0, 0, 0};

        if(bParse) {
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconEEPRomInfo.Parse(file);
            }
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconUsbInfo.Parse(file);
            }
        }

        if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
            enableGamma = tconEEPRomInfo.GetEnableUpdateGamma(mInitMode);
            for( int i = 0; i < tconEEPRomInfo.GetRegister(mInitMode).length; i++ ) {
                byte[] reg = ctrl.IntToByteArray(tconEEPRomInfo.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] data = ctrl.IntToByteArray(tconEEPRomInfo.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, data);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                byte[] result;
                if( bValidPort0 ) {
                    result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if(null == result) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(result) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }

                if( bValidPort1 ) {
                    result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    if(null == result) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(result) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
            }
        }

        if( (10 <= mInitMode) && ((mInitMode-10) < tconUsbInfo.GetModeNum())) {
            enableGamma = tconUsbInfo.GetEnableUpdateGamma(mInitMode-10);
            for( int i = 0; i < tconUsbInfo.GetRegister(mInitMode-10).length; i++ ) {
                byte[] reg = ctrl.IntToByteArray(tconUsbInfo.GetRegister(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] data = ctrl.IntToByteArray(tconUsbInfo.GetData(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, data);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                byte[] result;
                if( bValidPort0 ) {
                    result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if(null == result) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(result) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }

                if( bValidPort1 ) {
                    result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    if(null == result) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(result) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
            }
        }
        return enableGamma;
    } //ParseTREGtxt

    private void WriteGamma(int mInitMode , ConfigTconInfo tconEEPRomInfo , int[] enableGamma , int[] mUpdateGamma) {
        String[] resultPath;
        if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
            resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                        Log.i(NxTag, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                        continue;
                    }

                    if(null != mUpdateGamma){
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(NxTag, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }
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

                    byte[] result;
                    if( bValidPort0 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Failed.");
                    }
                    if( bValidPort1 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData1 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Failed.");
                    }
                }
            }
        }

        if( (10 <= mInitMode) ) {
            resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                        Log.i(NxTag, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                        continue;
                    }

                    if(null != mUpdateGamma){
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(NxTag, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }
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

                    byte[] result;
                    if( bValidPort0 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Failed.");
                    }
                    if( bValidPort1 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData1 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Failed.");
                    }
                }
            }

            resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 2) ) {
                        Log.i(NxTag, String.format( "Skip. Update USB Gamma. ( %s )", file ));
                        continue;
                    }

                    if(null != mUpdateGamma){
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdateGamma[0]) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdateGamma[1]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdateGamma[2]) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdateGamma[3]) ) {
                            Log.i(NxTag, String.format( "Skip. Already Update USB Gamma. ( %s )", file ));
                            continue;
                        }
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

                    byte[] result;
                    if( bValidPort0 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port0 Failed.");
                    }
                    if( bValidPort1 ){
                        result = ctrl.Send( cmd + info.GetChannel(), inData1 );
                        if (( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) )) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Unknown Error.");
                        if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_xGAM_R port1 Failed.");
                    }
                }
            }
        }
    } //WriteGamma

    private void EEPRomRead() {
        byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
        if( result == null ) {
            Log.i(NxTag, "Unknown Error.");
            return;
        }
        else if( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_EEPROM_READ Unknown Error.");
        else if( RESOURCE_BUSY == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_EEPROM_READ I2C busy.");
        else if( FAILED == ctrl.ByteArrayToInt(result) ) Log.i(NxTag, "CMD_TCON_EEPROM_READ Fail.");
    }



    //
    // Methods for AsyncTask
    //

    private void LoginAsyncTaskCheckCabinet() {
        byte[] ret;
        CinemaInfo mCinemaInfo = (CinemaInfo)appContext;

        int mInitMode =
                (((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_INITIAL_MODE) == null) ?
                        0 : Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_INITIAL_MODE));

        boolean bFirstBoot = ((SystemClock.elapsedRealtime() / 1000) < ((CinemaInfo)appContext).GetBootTime());
        if( bFirstBoot ) {
            try {
                while( true ) {
                    Log.i(NxTag, String.format(Locale.US, "elapsed time : %d sec", SystemClock.elapsedRealtime() / 1000));
                    Thread.sleep(1000);

                    if( (SystemClock.elapsedRealtime() / 1000) > ((CinemaInfo)appContext).GetBootTime() )
                        break;
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        DetectCabinet();


        //
        //  Display Version
        //
        Version();
        byte[] napVersion = mVersionAsyncParam.getNapVersion();
        byte[] sapVersion = mVersionAsyncParam.getSapVersion();
        byte[] srvVersion = mVersionAsyncParam.getSrvVersion();
        byte[] clnVersion = mVersionAsyncParam.getClnVersion();
        byte[] pfpgaVersion = mVersionAsyncParam.getPfpgaVersion();

        Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );

        Log.i(NxTag, ">> Version Information");
        Log.i(NxTag, String.format(Locale.US, "-. Application : %s", new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date)));
        Log.i(NxTag, String.format(Locale.US, "-. N.AP        : %s", (napVersion != null && napVersion.length != 0) ? new String(napVersion) : "Unknown"));
        Log.i(NxTag, String.format(Locale.US, "-. S.AP        : %s", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion) : "Unknown"));

        for( byte value : mCabinet ) {
            byte[] tconVersion = ctrl.Send(NxCinemaCtrl.CMD_TCON_VERSION, new byte[]{value});

            int msbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
            int lsbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

            Log.i(NxTag, String.format(Locale.US, "-. TCON #%02d    : %05d - %05d", (value & 0x7F) - CinemaInfo.TCON_ID_OFFSET, msbVersion, lsbVersion));
        }
        Log.i(NxTag, String.format(Locale.US, "-. PFPGA       : %s", (pfpgaVersion != null && pfpgaVersion.length != 0) ? String.format(Locale.US, "%05d", ctrl.ByteArrayToInt(pfpgaVersion)) : "Unknown"));
        Log.i(NxTag, String.format(Locale.US, "-. IPC Server  : %s", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion) : "Unknown"));
        Log.i(NxTag, String.format(Locale.US, "-. IPC Client  : %s", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion) : "Unknown"));

        Log.i(NxTag, String.format(Locale.US, "-. TCON Booting Check : %b", mCinemaInfo.IsCheckTconBooting()));

        if( !mCinemaInfo.IsFirstBootAccessEEPRom() ) EEPRomRead();

        if( bFirstBoot ) {
            if( mCinemaInfo.IsFirstBootAccessEEPRom() ) EEPRomRead();

            if( !CheckTCONBootingStatus() ) return;

            //  PFPGA Mute on
            ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE on fail.");

            //  Parse P_REG.txt
            int enableUniformity = ParsePREGtxt(mInitMode);

            //  Auto Uniformity Correction Writing
            if(1 == enableUniformity) AutoUniformityCorrectionWriting(enableUniformity);

            //  Parse T_REG.txt
            ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
            ConfigTconInfo tconUsbInfo = new ConfigTconInfo();
            int[] enableGamma = ParseTREGtxt( mInitMode , tconEEPRomInfo , tconUsbInfo , true);

            //  Write Gamma
            WriteGamma(mInitMode , tconEEPRomInfo , enableGamma , null);


            //
            //  Update Gamma Status
            //
            mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
            mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
            mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
            mCinemaInfo.SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );
//            //
//            // Test Code by doriya
//            byte[] reg = ctrl.IntToByteArray(0x018D, NxCinemaCtrl.FORMAT_INT16);
//            byte[] data = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
//            byte[] inData = ctrl.AppendByteArray(reg, data);
//
//            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
//            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);
//
//            if( bValidPort0 ) {
//                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
//                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
//                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
//            }
//            if( bValidPort1 ) {
//                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
//                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
//                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
//            }

            //
            // SW Reset
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});


            //
            //  Set Global Register
            //
            {
                int [][] treg = mCinemaInfo.GetDefaultTReg();
                for( int i = 0; i < treg.length; i++ ) {
                    if( treg[i][1] == -1 ) {
                        continue;
                    }

                    byte[] reg = ctrl.IntToByteArray(treg[i][0], NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(treg[i][1], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);

                    if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }

                int [][] preg = mCinemaInfo.GetDefaultPReg();
                for( int i = 0; i < preg.length; i++ ) {
                    if( preg[i][1] == -1 ) {
                        continue;
                    }
                    byte[] reg = ctrl.IntToByteArray(treg[i][0], NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(treg[i][1], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
                }
            }

            //  PFPGA Mute off
            ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE off fail.");

            //  TCON Initialize
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x09});
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x89});
        }
    } //LoginAsyncTaskCheckCabinet


    private TconInfoCarrier mTConInfoCarrier = new TconInfoCarrier();
    public TconInfoCarrier getTconInfoCarrier() {
        if (null == mTConInfoCarrier) {
            Log.e(NxTag , "Param is null..");
        }
        return mTConInfoCarrier;
    }


    private void TopAsyncTaskEEPRomRead() {
        //actually, don't need burst
        EEPRomRead();
    } //TopAsyncTaskEEPRomRead

    private void TopAsyncTaskInitMode() {
        byte[] ret;
        int mInitMode = mTConInfoCarrier.getiMode();
        int mUpdateGamma[] = new int[4];
        mUpdateGamma[0] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM0));
        mUpdateGamma[1] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM1));
        mUpdateGamma[2] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM0));
        mUpdateGamma[3] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM1));

        ConfigTconInfo mTconEEPRomInfo = mTConInfoCarrier.getTconEEPRomInfo();
        ConfigTconInfo mTconUsbInfo = mTConInfoCarrier.getTconUsbInfo();

        //
        if(null == mCabinet) DetectCabinet();

        if( mCabinet.length == 0 ) return;

        if( !CheckTCONBootingStatus() ) return;

        //  PFPGA Mute on
        ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE on fail.");

        //  Parse P_REG.txt
        int enableUniformity = ParsePREGtxt(mInitMode);

        //  Auto Uniformity Correction Writing
        if(1 == enableUniformity) AutoUniformityCorrectionWriting(enableUniformity);

        //  Parse T_REG.txt
        int[] enableGamma = ParseTREGtxt( mInitMode , mTconEEPRomInfo , mTconUsbInfo , false);

        //  Write Gamma
        WriteGamma( mInitMode , mTconEEPRomInfo , enableGamma , mUpdateGamma);

        //
        //  Update Gamma Status
        //
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

//        // Test Code by doriya
//        byte[] reg = ctrl.IntToByteArray(0x018D, NxCinemaCtrl.FORMAT_INT16);
//        byte[] data = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
//        byte[] inData = ctrl.AppendByteArray(reg, data);
//
//        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
//        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);
//
//        if( bValidPort0 ) {
//            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
//            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
//            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
//        }
//        if( bValidPort1 ) {
//            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
//            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
//            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
//        }

        //  SW Reset
        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

        //  PFPGA Mute off
        ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE off fail.");

    } //TopAsyncTaskInitMode



    private DiagnosticsAsyncParam mDiagnosticsAsyncParam = new DiagnosticsAsyncParam();
    public DiagnosticsAsyncParam getDiagnosticsTconStatusAsyncParam() {
        if (null == mDiagnosticsAsyncParam) {
            Log.e(NxTag , "Param is null..");
        }
        return mDiagnosticsAsyncParam;
    }

    private void DiagnosticsAsyncTaskTconStatus() {
        for( byte cabinet : mCabinet ) {
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{cabinet} );
            if (result == null ) {
                Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));

                mDiagnosticsAsyncParam.setTCON((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setTCONStatusResult(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            if( SUCCESS != ctrl.ByteArrayToInt(result)) {
                Log.i(NxTag, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));

                mDiagnosticsAsyncParam.setTCON((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setTCONStatusResult((int)result[0]);
                sendCommand.onProgressUpdate();
            }
        }
    } //DiagnosticsAsyncTaskTconStatus

    private void DiagnosticsAsyncTaskTconLvds() {
        for( byte cabinet : mCabinet ) {
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_LVDS_STATUS, new byte[]{cabinet} );
            if ( result == null ) {
                Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));

                mDiagnosticsAsyncParam.setTCONLvds((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setTCONLvdsResult(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            if( SUCCESS != ctrl.ByteArrayToInt(result)) {
                Log.i(NxTag, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));

                mDiagnosticsAsyncParam.setTCONLvds((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setTCONLvdsResult((int)result[0]);
                sendCommand.onProgressUpdate();
            }
        }
    } //DiagnosticsAsyncTaskTconLvds

    private void DiagnosticsAsyncTaskLedOpenNum() {
        byte[] ret;
        if( bValidPort0 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x09} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_LOD 0x09 fail.");
        }
        if( bValidPort1 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x89} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_LOD 0x89 fail.");
        }

        for( byte cabinet : mCabinet ) {
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{cabinet} );
            if ( result == null ) {
                Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet ));

                mDiagnosticsAsyncParam.setLedOpenNumCabinetID(cabinet & 0xFF);
                mDiagnosticsAsyncParam.setLedOpenNumSlaveAddr((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setLedOpenNumResult(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            if( UNKNOWN_ERR != ctrl.ByteArrayToInt(result) ){
                mDiagnosticsAsyncParam.setLedOpenNumCabinetID(cabinet & 0xFF);
                mDiagnosticsAsyncParam.setLedOpenNumSlaveAddr((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setLedOpenNumResult(ctrl.ByteArrayToInt(result));
                sendCommand.onProgressUpdate();
            }
        }

        if( bValidPort0 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x09} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_NORMAL 0x09 fail.");
        }

        if( bValidPort1 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x89} );
            if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_NORMAL 0x89 fail.");
        }
    } //DiagnosticsAsyncTaskLedOpenNum

    private int[] mDatFullWhite;
    private void DiagnosticsAsyncTaskFullWhite() {
        byte[] ret;
        int[][] mRegDataFullWhite = {
                {0x0004, 0},	// TCON_REG_XYZ_TO_RGB
                {0x0024, 10},   // TCON_REG_PATTERN
                {0x0025, 0},    // TCON_REG_LEFT
                {0x0026, 1024}, // TCON_REG_RIGHT
                {0x0027, 0},    // TCON_REG_TOP
                {0x0028, 2160}, // TCON_REG_DOWN
                {0x0029, 4095}, // TCON_REG_BOX_R
                {0x002A, 4095}, // TCON_REG_BOX_G
                {0x002B, 4095}, // TCON_REG_BOX_B
        };

        if( 0 == mCabinet.length  ) {
            CinemaAlert.Show( appContext, "Alert", "Task Full white(or restore) err,, no valid cabinet" , CinemaAlert.TYPE_NXAsync, null);
            return;
        }

        if( null == mDatFullWhite && mDiagnosticsAsyncParam.isFullWhiteRestore() ) {
            CinemaAlert.Show( appContext, "Alert", "Task Full white restore err,, no backup data" , CinemaAlert.TYPE_NXAsync, null);
            return;
        }

        for( int i = 0; i < mRegDataFullWhite[0].length; i++ )
        {
            byte[] reg = ctrl.IntToByteArray(mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mDiagnosticsAsyncParam.isFullWhiteRestore() ? mDatFullWhite[i] : mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
            if( bValidPort1 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
        }
    } //DiagnosticsAsyncTaskFullWhite

    private void DiagnosticsAsyncTaskFullWhiteBackup() {
        int[][] mRegDataFullWhite = {
                {0x0004, 0},	// TCON_REG_XYZ_TO_RGB
                {0x0024, 10},   // TCON_REG_PATTERN
                {0x0025, 0},    // TCON_REG_LEFT
                {0x0026, 1024}, // TCON_REG_RIGHT
                {0x0027, 0},    // TCON_REG_TOP
                {0x0028, 2160}, // TCON_REG_DOWN
                {0x0029, 4095}, // TCON_REG_BOX_R
                {0x002A, 4095}, // TCON_REG_BOX_G
                {0x002B, 4095}, // TCON_REG_BOX_B
        };

        mDatFullWhite = new int[mRegDataFullWhite.length];

        if( 0 == mCabinet.length  ) {
            CinemaAlert.Show( appContext, "Alert", "Task Full white backup err,, no valid cabinet" , CinemaAlert.TYPE_NXAsync, null);
            return;
        }

        for( int i = 0; i < mRegDataFullWhite.length; i++ )
        {
            byte[] reg = ctrl.IntToByteArray(mRegDataFullWhite[i][0], NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = new byte[] { mCabinet[0] };
            inData = ctrl.AppendByteArray(inData, reg);

            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );

            if( result == null ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mRegDataFullWhite[i][0] ));
                return;
            }

            if( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mRegDataFullWhite[i][0] ));
                return;
            }

            mDatFullWhite[i] = ctrl.ByteArrayToInt(result);

            Log.i(NxTag, String.format(Locale.US, "Full White Backup. ( 0x%04X, %d )", mRegDataFullWhite[i][0], mDatFullWhite[i]));
        }
    } //DiagnosticsAsyncTaskFullWhiteBackup

    private void DiagnosticsAsyncTaskLedOpenDetect() {
        byte[] ret;
        if( mCabinet.length == 0 ) return;

        byte[] reg = ctrl.IntToByteArray(0x0100, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mDiagnosticsAsyncParam.isLedOpenDetect() ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }
        if( bValidPort1 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }
    } //DiagnosticsAsyncTaskLedOpenDetect

    private void DiagnosticsAsyncTaskLodRemovalWrite() {
        byte[] ret;
        if( mCabinet.length == 0 ) return;

        byte[] reg, dat, inData;
        byte[] inData0, inData1;

        //  Removal #1
        reg = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
        dat = ctrl.IntToByteArray(mDiagnosticsAsyncParam.isLodRemovalWrite1() ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
        inData = ctrl.AppendByteArray(reg, dat);

        inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }
        if( bValidPort1 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }

        //  Removal #2
        reg = ctrl.IntToByteArray(0x00E6, NxCinemaCtrl.FORMAT_INT16);
        dat = ctrl.IntToByteArray(mDiagnosticsAsyncParam.isLodRemovalWrite2() ? 1 : 0, NxCinemaCtrl.FORMAT_INT16);
        inData = ctrl.AppendByteArray(reg, dat);

        inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }
        if( bValidPort1 ) {
            ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
            else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
        }
    } //DiagnosticsAsyncTaskLodRemovalWrite

    private void DiagnosticsAsyncTaskLodRemovalRead() {
        if( mCabinet.length == 0 ) return;

        byte[] reg1, reg2;
        byte[] inData1, inData2;
        byte[] result1, result2;

        reg1 = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
        reg2 = ctrl.IntToByteArray(0x00E6, NxCinemaCtrl.FORMAT_INT16);

        inData1 = ctrl.AppendByteArray(new byte[] { mCabinet[0] }, reg1);
        inData2 = ctrl.AppendByteArray(new byte[] { mCabinet[0] }, reg2);

        result1 = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData1 );
        if( result1 == null ) {
            Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x011E ));
        }
        if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(result1) ) {
            Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x011E ));
        }

        result2 = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData2 );
        if( result2 == null ) {
            Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x00E6 ));
        }
        if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(result2) ) {
            Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], 0x00E6 ));
        }

        mDiagnosticsAsyncParam.setLodRemovalReadFlag1((ctrl.ByteArrayToInt(result1) == 1));
        mDiagnosticsAsyncParam.setLodRemovalReadFlag2((ctrl.ByteArrayToInt(result2) == 1));
        sendCommand.onProgressUpdate();
    } //DiagnosticsAsyncTaskLodRemovalRead

    private void DiagnosticsAsyncTaskCabinetDoor() {
        for( byte cabinet : mCabinet ) {
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{cabinet} );
            if ( result == null ) {
                Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));

                mDiagnosticsAsyncParam.setCabinetDoorCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setCabinetDoorResult(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            if (UNKNOWN_ERR == ctrl.ByteArrayToInt(result)) {
                Log.i(NxTag, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                mDiagnosticsAsyncParam.setCabinetDoorCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setCabinetDoorResult(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            if( FAILED == ctrl.ByteArrayToInt(result) ) {//door opened
                Log.i(NxTag, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));

                mDiagnosticsAsyncParam.setCabinetDoorCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setCabinetDoorResult(StatusSimpleInfo.FAIL);
                sendCommand.onProgressUpdate();
            }
//            else if( result[0] == 0 || result[0] == 2 ) {
//                Log.i(NxTag, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
//
//                mDiagnosticsAsyncParam.setCabinetDoorCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
//                mDiagnosticsAsyncParam.setCabinetDoorResult(StatusSimpleInfo.FAIL);
//                sendCommand.onProgressUpdate();
//            }
        }
    } //DiagnosticsAsyncTaskCabinetDoor

    private void DiagnosticsAsyncTaskPeripheral() {
        //  Security AP
        {
            StatusSimpleInfo info = mDiagnosticsAsyncParam.getPeripheralAdapter().getItem(0);
            info.SetStatus(StatusSimpleInfo.FAIL);

            ((CinemaInfo)appContext).SetSecureAlive("false");
            for( int i = 0; i < 16; i++ ) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if( ((CinemaInfo)appContext).GetSecureAlive().equals("true") ) {
                    info.SetStatus(StatusSimpleInfo.PASS);
                    break;
                }
            }
            sendCommand.onProgressUpdate();
        }

        //
        //  P.FPGA
        //
        {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            StatusSimpleInfo info = mDiagnosticsAsyncParam.getPeripheralAdapter().getItem(1);
            info.SetStatus(StatusSimpleInfo.ERROR);

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_STATUS, null);
            if( result == null || result.length == 0 ) {
                Log.i(NxTag,  "Unknown Error.");
                info.SetStatus(StatusSimpleInfo.ERROR);
            }else if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) || FAILED == ctrl.ByteArrayToInt(result) ) {
                info.SetStatus(StatusSimpleInfo.ERROR);
            }
            else {
                info.SetStatus( StatusSimpleInfo.PASS );
            }
            sendCommand.onProgressUpdate();
        }

        //
        //  IMB
        //
        {
            NetworkTools tools = new NetworkTools();
            StatusSimpleInfo info = mDiagnosticsAsyncParam.getPeripheralAdapter().getItem(2);

            String strImbAddress = "";
            try {
                BufferedReader inReader = new BufferedReader(new FileReader("/system/bin/nap_network"));
                try {
                    inReader.readLine();    // param: ip address ( mandatory )
                    inReader.readLine();    // param: netmask    ( mandatory )
                    inReader.readLine();    // param: gateway    ( mandatory )
                    inReader.readLine();    // param: network    ( mandatory but auto generate )
                    inReader.readLine();    // param: dns1       ( optional )
                    inReader.readLine();    // param: dns2       ( optional )
                    strImbAddress = inReader.readLine();// param: imb address( optional )
                    inReader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            if( strImbAddress != null && !strImbAddress.equals("") ) {
                try {
                    if( tools.GetEthLink().equals("true") && tools.Ping(strImbAddress) ) {
                        info.SetStatus(StatusSimpleInfo.PASS);
                    }
                    else {
                        info.SetStatus(StatusSimpleInfo.FAIL);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            else {
                info.SetStatus(StatusSimpleInfo.ERROR);
            }
        }
        sendCommand.onProgressUpdate();
    } //DiagnosticsAsyncTaskPeripheral

    private void DiagnosticsAsyncTaskVersion() {
        for( byte cabinet : mCabinet ) {
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_VERSION, new byte[]{cabinet} );
            if (result == null || result.length == 0 || result.length == 4) {

                mDiagnosticsAsyncParam.setVersionCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                mDiagnosticsAsyncParam.setVersion1(-1);
                mDiagnosticsAsyncParam.setVersion2(-1);
                sendCommand.onProgressUpdate();
                continue;
            }

            int version1 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_MSB);
            int version2 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_LSB);

            mDiagnosticsAsyncParam.setVersionCabinet((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
            mDiagnosticsAsyncParam.setVersion1(version1);
            mDiagnosticsAsyncParam.setVersion2(version2);
            sendCommand.onProgressUpdate();
        }
    } //DiagnosticsAsyncTaskVersion


    private DisplayCheckAsyncParam mDisplayCheckAsyncParam = new DisplayCheckAsyncParam();
    public DisplayCheckAsyncParam getDisplayCheckAsyncParam() {
        if(null == mDisplayCheckAsyncParam){
            Log.i(NxTag , "param is null...");
        }
        return mDisplayCheckAsyncParam;
    }

    private void DisplayCheckAsyncTaskTestPattern() {
        if( mCabinet.length == 0 )
            return;

        int mFuncIndex = mDisplayCheckAsyncParam.getDisplayCheckFuncIndex();
        int mPatternIndex = mDisplayCheckAsyncParam.getDisplayCheckPatternIndex();
        boolean mStatus = mDisplayCheckAsyncParam.isDisplayCheckStatus();

        String[] mDci = {
                "Red-1",            "Green-1",          "Blue-1",
                "Cyan-1",           "Magenta-1",        "Yellow-1",
                "Red-2",            "Green-2",          "Blue-2",
                "Cyan-2",           "Magenta-2",        "Yellow-2",
                "White-1",          "White-2",          "White-3",
        };

        String[] mFullScreenColor = {
                "White 100%",       "Gray 90%",         "Gray 80%",
                "Gray 70%",         "Gray 60%",         "Gray 50%",
                "Gray 40%",         "Gray 30%",         "Gray 20%",
                "Gray 10%",         "Black 0%",         "Red 100%",
                "Green 100%",       "Blue 100%",        "Magenta 100%",
                "Cyan 100%",        "Yellow 100%",
        };

        String[] mGrayScale = {
                "Gray 16-Step",     "Gray 32-Step",     "Gray 64-Step",
                "Gray 128-Step",    "Gray 256-Step",    "Gray 512-Step",
                "Gray 2048-Step",   "Red 2048-Step",    "Green 2048-Step",
                "Blue 2048-Step",
        };

        String[] mDiagonalPattern = {
                "Right Down ( ↘ )", "Right Up ( ↗ )",
        };

        String[][] mPatternName = {
                mDci,
                new String[0],
                mFullScreenColor,
                mGrayScale,
                new String[0],
                mDiagonalPattern,
                new String[0],
                new String[0],
                new String[0],
                new String[0],
                new String[0],
        };

        String[] mFuncName = {
                "DCI",
                "Color Bar",
                "Full Screen Color",
                "Gary Scale ( Horizontal )",
                "Dot Pattern / Checker Board",
                "Diagonal Pattern",
                "Cabinet ID",
                "Dot Correction",
                "Gamut Mapping",
                "XYZ to RGB",
                "White Seam Value",
        };

        int[] mPatternReg = {
                0x0044,     // REG_FLASH_CC
                0x0052,     // REG_CC_MODULE
                0x0004,     // REG_XYZ_TO_RGB
                0x0192,     // REG_SEAM_ON
        };

        Log.i(NxTag, String.format( "funcIndex(%d), patternIndex(%d), status(%s)", mFuncIndex, mPatternIndex, String.valueOf(mStatus)) );

        String strLog;
        byte[] ret;

        if( mFuncIndex < 7 ) {
            byte[] data = { (byte)mFuncIndex, (byte)mPatternIndex };

            byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
            byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

            if( bValidPort0 ) {
                ret = ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data0 );
                if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , " CMD_TCON_PATTERN_ fail.");
            }
            if( bValidPort1 ) {
                ret = ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data1 );
                if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , " CMD_TCON_PATTERN_ fail.");
            }

            if( mPatternName[mFuncIndex].length == 0 ) {
                strLog = String.format( "%s pattern test. ( %s )", mStatus ? "Run" : "Stop", mFuncName[mFuncIndex] );
            }
            else {
                strLog = String.format( "%s pattern test. ( %s / %s )", mStatus ? "Run" : "Stop", mFuncName[mFuncIndex], mPatternName[mFuncIndex][mPatternIndex] );
            }
        }
        else {
            byte[] reg = ctrl.IntToByteArray(mPatternReg[mFuncIndex-7], NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mStatus ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] data = ctrl.AppendByteArray(reg, dat);
            byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
            byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

            if( bValidPort0 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
            if( bValidPort1 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }

            strLog = String.format( "%s %s", mFuncName[mFuncIndex], mStatus ? "Enable" : "Disable" );
        }

        ((CinemaInfo)appContext).InsertLog(strLog);

//
        //
        //
        int [] resDat = { 0x0000, 0x0000, 0x0000, 0x000 };
        for( int i = 0; i < mPatternReg.length; i++ )
        {
            byte[] result, inData;
            byte slave = mCabinet[0];

            inData = new byte[] { slave };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mPatternReg[i], NxCinemaCtrl.FORMAT_INT16));

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
            if( result == null || result.length == 0 ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", slave, mPatternReg[i] ));
                return;
            }

            resDat[i] = ctrl.ByteArrayToInt(result);
        }

        for( int i = 0; i < mPatternReg.length; i++ )
        {
            Log.i(NxTag, String.format(">>> read pattern register. ( reg: 0x%04X, dat: 0x%04X )", mPatternReg[i], resDat[i]) );
        }


        mDisplayCheckAsyncParam.setResDat(resDat);
        sendCommand.onProgressUpdate();
    } //DisplayCheckAsyncTaskTestPattern

    private void DisplayCheckAsyncTaskAccumulation() {
        StatusDescribeExpandableAdapter mAdapter = mDisplayCheckAsyncParam.getDisplayCheckAdapter();
        mAdapter.clear();

        if( mCabinet == null ) return;

        for( int i = 0; i < mCabinet.length; i++ ) {
            mAdapter.addGroup( new StatusDescribeExpandableInfo(String.format( Locale.US, "Cabinet %02d", (mCabinet[i] & 0x7F) - CinemaInfo.TCON_ID_OFFSET )) );

            for( int j = 0; j < CinemaInfo.TCON_MODULE_NUM; j++ ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_ACCUMULATE_TIME, new byte[] { mCabinet[i], (byte)j } );
                if( result == null || result.length == 0 ) {
                    mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), String.valueOf("Error")) );
                    continue;
                }
                mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), new String(result) ));
            }
            sendCommand.onProgressUpdate();
        }
    } //DisplayCheckAsyncTaskAccumulation


    private DisplayModeAsyncParam mDisplayModeAsyncParam = new DisplayModeAsyncParam();
    public DisplayModeAsyncParam getDisplayModeAsyncParam() {
        if (null == mDisplayModeAsyncParam) {
            Log.i(NxTag , "param is null");
        }
        return mDisplayModeAsyncParam;
    }

    private void DisplayModeAsyncTaskCheckCabinet() {
        DetectCabinet();
    } //DisplayModeAsyncTaskCheckCabinet

    private void DisplayModeAsyncTaskMasteringWrite() {
        int mMasteringModePos = mDisplayModeAsyncParam.getMasteringWriteModePos();

        int[] mRegHideMastering = {
                0x0082, 0x0097,
        };
        int[][] mDataHideMastering = {
                { 0x0000, 0x0000 },
                { 0x0001, 0x0001 },
                { 0x0000, 0x0000 },
        };
        int[] mRegMastering = {
                0x0056,     0x0057,     0x0058,     //  REG_CC00,       REG_CC01,       REG_CC02
                0x0059,     0x005A,     0x005B,     //  REG_CC10,       REG_CC11,       REG_CC12
                0x005C,     0x005D,     0x005E,     //  REG_CC20,       REG_CC21,       REG_CC22
                0x00DD,                             //  REG_BC
                0x00B9,     0x00BA,     0x00BB,     //  REG_CC_R01,     REG_CC_G01,     REG_CC_B01
                0x00B6,     0x00B7,     0x00B8,     //  REG_LGSE1_R,    REG_LGSE1_G,    REG_LGSE1_B
        };

        int[] mDataDci = {
                6361,       74,         269,
                272,        6586,       29,
                0,          128,        6701,
                1,
                145,        98,         67,
                14,         15,         12,
        };

        int[] mDataHdr = {
                12869,      1989,       507,
                607,        15065,      177,
                0,          0,          16383,
                4,
                252,        234,        111,
                5,          5,          8,
        };
        String[][] mTextMastering = {
                { "CC00",           "16383" },
                { "CC01",           "16383" },
                { "CC02",           "16383" },
                { "CC10",           "16383" },
                { "CC11",           "16383" },
                { "CC12",           "16383" },
                { "CC20",           "16383" },
                { "CC21",           "16383" },
                { "CC22",           "16383" },
                { "Global",         "7"     },
                { "R Channel",      "511"   },
                { "G Channel",      "511"   },
                { "B Channel",      "511"   },
                { "R Channel(Low)", "15"    },
                { "G Channel(Low)", "15"    },
                { "B Channel(Low)", "15"    },
        };
        int[] mDataMaster = new int[mTextMastering.length];

        int[][] mDataMastering = {
                mDataDci,
                mDataHdr,
                mDataMaster,
        };
        byte[] ret;

        if( mDisplayModeAsyncParam.isMasteringWriteMode() ) {
            for( int i = 0; i < mRegHideMastering.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(reg, data);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
                if( bValidPort1 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
            }
            return;
        }

        for( int i = 0; i < mRegHideMastering.length; i++ ) {
            byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT16 );
            byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
            byte[] inData = ctrl.AppendByteArray(reg, data);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
            if( bValidPort1 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
        }

        for( int i = 0; i < mRegMastering.length; i++ ) {
            byte[] reg = ctrl.IntToByteArray( mRegMastering[i], NxCinemaCtrl.FORMAT_INT16 );
            byte[] data = ctrl.IntToByteArray( mDataMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
            byte[] inData = ctrl.AppendByteArray(reg, data);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
            if( bValidPort1 ) {
                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
        }
    } //DisplayModeAsyncTaskMasteringWrite

    private void DisplayModeAsyncTaskMasteringRead() {
        int[] mRegMastering = {
                0x0056,     0x0057,     0x0058,     //  REG_CC00,       REG_CC01,       REG_CC02
                0x0059,     0x005A,     0x005B,     //  REG_CC10,       REG_CC11,       REG_CC12
                0x005C,     0x005D,     0x005E,     //  REG_CC20,       REG_CC21,       REG_CC22
                0x00DD,                             //  REG_BC
                0x00B9,     0x00BA,     0x00BB,     //  REG_CC_R01,     REG_CC_G01,     REG_CC_B01
                0x00B6,     0x00B7,     0x00B8,     //  REG_LGSE1_R,    REG_LGSE1_G,    REG_LGSE1_B
        };

        if( !mDisplayModeAsyncParam.isMasteringWriteMode() ) return;

        if( mCabinet == null || mCabinet.length == 0 ) return;

        for (int i = 0; i < mRegMastering.length; i++) {
            byte[] reg = ctrl.IntToByteArray( mRegMastering[i], NxCinemaCtrl.FORMAT_INT16 );
            byte[] inData = ctrl.AppendByteArray(new byte[]{mCabinet[0]}, reg);
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );

            if( result == null ) continue;
            else if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) ) continue;

            mDisplayModeAsyncParam.setMasteringReadValue(i , ctrl.ByteArrayToInt(result));
        }
    } //DisplayModeAsyncTaskMasteringRead

    private void DisplayModeAsyncTaskUniformityCorrection() {
        int mIndexUniformity = mDisplayModeAsyncParam.getmIndexUniformity();

        Log.i(NxTag , "mIndexUniformity   :  " + mIndexUniformity);
        int enableUniformity = ParsePREGtxt( mIndexUniformity );

        if(1 == enableUniformity) AutoUniformityCorrectionWriting(enableUniformity);
    }//DisplayModeAsyncTaskUniformityCorrection

    private void DisplayModeAsyncTaskImageQuality() {
        int mModeIndexQuality = mTConInfoCarrier.getiMode();
        int[] mUpdateGamma = new int[4];
        mUpdateGamma[0] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM0));
        mUpdateGamma[1] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM1));
        mUpdateGamma[2] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM0));
        mUpdateGamma[3] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM1));

        ConfigTconInfo mTconEEPRomInfo = mTConInfoCarrier.getTconEEPRomInfo();
        ConfigTconInfo mTconUsbInfo = mTConInfoCarrier.getTconUsbInfo();
        byte[] ret;

        if( mCabinet.length == 0 ) return;

        if( !CheckTCONBootingStatus() ) return;

        //  PFPGA Mute on
        ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE on fail.");

        //  Parse P_REG.txt
//            boolean enableUniformity = false;
        ParsePREGtxt( mModeIndexQuality );

        //  Parse T_REG.txt
        int[] enableGamma = ParseTREGtxt( mModeIndexQuality , mTconEEPRomInfo , mTconUsbInfo , false);

        //  Write Gamma
        WriteGamma( mModeIndexQuality ,  mTconEEPRomInfo , enableGamma , mUpdateGamma);

        //
        //  Update Gamma Status
        //
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

        //
        //  SW Reset
        //
        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

        //  PFPGA Mute off
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_PFPGA_MUTE off fail.");
    } //DisplayModeAsyncTaskImageQuality

    private void DisplayModeAsyncTaskAdapterDotCorrection() {
        //don't need burst
        String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        for( String dir : resultDir ) {
            mDisplayModeAsyncParam.setDir(dir);
            sendCommand.onProgressUpdate();
        }
    } //DisplayModeAsyncTaskAdapterDotCorrection

    private void DisplayModeAsyncTaskPixelCorrection() {
        String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
        if( resultDir == null || resultDir.length == 0 ) return;

        String topdir = resultDir[0].substring(0, resultDir[0].lastIndexOf("/") + 1);
        CheckRunAdapter mAdapter = mDisplayModeAsyncParam.getmAdapterDot();
        for( int i = 0; i < mAdapter.getCount(); i++ ) {
            int success = 0;
            int fail = 0;

            CheckRunInfo item = mAdapter.getItem(i);
            if( !item.GetChecked() ) continue;

            String[] result = FileManager.CheckFile(topdir + item.GetTitle(), LedDotCorrectInfo.PATTERN_NAME);
            for( String file : result ) {
                Log.i(NxTag, "Dot Correct Info : " + file);

                LedDotCorrectInfo info = new LedDotCorrectInfo();
                if( info.Parse(file) ) {
                    byte[] sel = ctrl.IntToByteArray( info.GetModule(), NxCinemaCtrl.FORMAT_INT8 );       // size: 1
                    byte[] data = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );    // size: 61440
                    byte[] inData =  ctrl.AppendByteArray( sel, data );

                    byte[] res = ctrl.Send( NxCinemaCtrl.CMD_TCON_PIXEL_CORRECTION, ctrl.AppendByteArray( new byte[]{(byte)info.GetIndex()}, inData ) );
                    if( res == null || res.length == 0 ) {

                        mDisplayModeAsyncParam.setProgress(i , result.length , success , ++fail);
                        sendCommand.onProgressUpdate();
                        continue;
                    }

                    if ( SUCCESS != ctrl.ByteArrayToInt(res) ) {
                        mDisplayModeAsyncParam.setProgress(i , result.length , success , ++fail);
                        sendCommand.onProgressUpdate();
                        continue;
                    }
                    mDisplayModeAsyncParam.setProgress(i , result.length , ++success , ++fail);
                    sendCommand.onProgressUpdate();
                }
            }
        }
    } //DisplayModeAsyncTaskPixelCorrection

    private void DisplayModeAsyncTaskPixelCorrectionExtract() {
        int mId = mDisplayModeAsyncParam.getmId();
        int mModule = mDisplayModeAsyncParam.getmModule();
        String externalStorage = mDisplayModeAsyncParam.getExternalStorage();

        int start   = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule     : 0;
        int end     = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule + 1 : LedDotCorrectInfo.MAX_MODULE_NUM;
        int idx     = mId;

        for( int i = start; i < end; i++ ) {
            Log.i(NxTag, String.format(Locale.US, "Dot correction extract. ( slave: 0x%02X, module: %d )", (byte)idx, i) );

            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_PIXEL_CORRECTION_EXTRACT, new byte[]{(byte)idx, (byte)i} );
            if( result == null || result.length == 0 || result.length == 4) continue;

            String strDir = String.format(Locale.US, "%s/DOT_CORRECTION_ID%03d", externalStorage, (idx & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
            if( !FileManager.MakeDirectory( strDir ) ) {
                Log.i(NxTag, String.format(Locale.US, "Fail, Create Directory. ( %s )", strDir));
                continue;
            }

            new LedDotCorrectInfo().Make(idx, i, result, strDir);
        }
    } //DisplayModeAsyncTaskPixelCorrectionExtract

    private void DisplayModeAsyncTaskWhiteSeamRead() {
        if( mCabinet.length == 0 )
            return;
        //
        //  Emulate is
        //      true    : Emulate Register --> UI
        //      false   : Flash Register --> Emulate Register --> UI

        int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        int[] mSeamVal = new int[4];
        int mIndexPos = mDisplayModeAsyncParam.getmIndexPos();
        boolean mEmulate = mDisplayModeAsyncParam.ismEmulate();

        int indexPos = (mIndexPos != 0) ? mIndexPos - 1 : 0;
        byte slave = mCabinet[indexPos];

        //
        //  1. Read White Seam Value in Flash Memory.
        //
        if( !mEmulate ) {
            Log.i(NxTag, ">>> White Seam Read in Flash Memory.");
            byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_WHITE_SEAM_READ, new byte[] {slave} );
            if( result == null ) {
                Log.i(NxTag, "Fail, WhiteSeam Read. ret is null ");
                return;
            }
            if( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) ) {
                Log.i(NxTag, "Fail, WhiteSeam Read.");
                return;
            }
        }

        //
        //  2. Read White Seam Value in Emulate Register.
        //
        Log.i(NxTag, ">>> White Seam Read in Emulate Register.");
        for( int i = 0; i < mSeamReg.length; i++ ) {
            byte[] result, inData;
            inData = new byte[] { slave };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[i], NxCinemaCtrl.FORMAT_INT16));

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
            if( result == null ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", slave, mSeamReg[i] ));
                return;
            }

            if( UNKNOWN_ERR == ctrl.ByteArrayToInt(result) ) {
                Log.i(NxTag, "WhiteSeam Read, CMD_TCON_REG_READ Fail.");
                return;
            }

            mSeamVal[i] = ctrl.ByteArrayToInt(result);
        }

        Log.i(NxTag, String.format(Locale.US, ">>> WhiteSeam Read Done. ( pos: %d, slave: 0x%02X, emulate: %b, top: %d, bottom: %d, left: %d, right: %d )",
                indexPos, slave, mEmulate, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));

        mDisplayModeAsyncParam.setmSeamVal(mSeamVal);
        sendCommand.onProgressUpdate();
        //publishProgress(mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]);
    } //DisplayModeAsyncTaskWhiteSeamRead

    private void DisplayModeAsyncTaskWhiteSeamEmulate() {
        int mIndexPos = mDisplayModeAsyncParam.getIndexPos();
        byte[] ret;
        int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        int[] mSeamVal = mDisplayModeAsyncParam.getSeamVal();

        Log.i(NxTag, String.format(Locale.US, "pos: %d", mIndexPos));

        if( mIndexPos == 0 ) {
            Log.i(NxTag, "WhiteSeam Emulate. ( index: all )");

            for( int j = 0; j < mSeamReg.length; j++ ) {
                byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                byte[] inData1 = new byte[] {(byte)0x09};
                inData1 = ctrl.AppendByteArray(inData1, reg);
                inData1 = ctrl.AppendByteArray(inData1, val);

                byte[] inData2 = new byte[] {(byte)0x89};
                inData2 = ctrl.AppendByteArray(inData2, reg);
                inData2 = ctrl.AppendByteArray(inData2, val);

                if( bValidPort0 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
                if( bValidPort1 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
            }
        }
        else {
            int pos = mIndexPos - 1;
            byte slave = mCabinet[pos];
            Log.i(NxTag, String.format(Locale.US, "WhiteSeam Emulate. ( index: %d, slave: 0x%02x )", pos, slave));

            for( int j = 0; j < mSeamReg.length; j++ ) {
                byte[] inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
        }
    } //DisplayModeAsyncTaskWhiteSeamEmulate

    private void DisplayModeAsyncTaskWhiteSeamWrite() {
        int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        int[] mSeamVal = mDisplayModeAsyncParam.getSeamvalue();
        int mIndexPos = mDisplayModeAsyncParam.getPos();
        byte[] ret;

        //
        //  1. Update White Seam in Emulate Register.
        //
        if( mIndexPos == 0 ) {

            for( int j = 0; j < mSeamReg.length; j++ ) {
                byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                byte[] inData1 = new byte[] {(byte)0x09};
                inData1 = ctrl.AppendByteArray(inData1, reg);
                inData1 = ctrl.AppendByteArray(inData1, val);

                byte[] inData2 = new byte[] {(byte)0x89};
                inData2 = ctrl.AppendByteArray(inData2, reg);
                inData2 = ctrl.AppendByteArray(inData2, val);

                if( bValidPort0 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
                if( bValidPort1 ) {
                    ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                    if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                    else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
                }
            }
        }
        else {
            int pos = mIndexPos - 1;
            byte slave = mCabinet[pos];
            for( int j = 0; j < mSeamReg.length; j++ ) {
                byte[] inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                if(null == ret) Log.i(NxTag , "CMD_TCON_REG_WRITE NULL fail.");
                else if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_REG_WRITE fail.");
            }
        }


        //
        //  2. Update White Seam in Flash Data.
        //
        int start   = (mIndexPos != 0) ? mIndexPos - 1  : 0;
        int end     = (mIndexPos != 0) ? mIndexPos      : mCabinet.length;

        for( int i = start; i < end; i++ ) {
            byte slave = mCabinet[i];
            byte[] result, inData;
            inData = new byte[] { slave };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[0], NxCinemaCtrl.FORMAT_INT16));
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[1], NxCinemaCtrl.FORMAT_INT16));
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[2], NxCinemaCtrl.FORMAT_INT16));
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[3], NxCinemaCtrl.FORMAT_INT16));

            result = ctrl.Send(NxCinemaCtrl.CMD_TCON_WHITE_SEAM_WRITE, inData);
            if( result == null || result.length == 0 || ctrl.ByteArrayToInt(result) != 0x01 ) {
                Log.i(NxTag, "Fail, Write WhiteSeam.");
                return;
            }
            if(SUCCESS != ctrl.ByteArrayToInt(result)) {
                Log.i(NxTag, "Fail, Write WhiteSeam.");
                return;
            }

            Log.i(NxTag, String.format(Locale.US, "WhiteSeam Write. ( pos: %d, slave: 0x%02x, top: %d, bottom: %d, left: %d, right: %d )",
                    i, slave, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));
        }
    } //DisplayModeAsyncTaskWhiteSeamWrite


    private StatusDetailAdapterAsyncParam mStatusDetailAdapterAsyncParam = new StatusDetailAdapterAsyncParam();
    public StatusDetailAdapterAsyncParam getStatusDetailAdapterAsyncParam () {
        if (null == mStatusDetailAdapterAsyncParam) {
            Log.i(NxTag, "param is null");
        }
        return mStatusDetailAdapterAsyncParam;
    }

    private void StatusDetailAdapterAsyncTaskLedPos() {
        byte mId = mStatusDetailAdapterAsyncParam.getmId();
        LedPosAdapter mAdapter = mStatusDetailAdapterAsyncParam.getmAdapter();
        byte[] ret;

        ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{mId} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_LOD fail.");

        byte[] resultNum = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{mId});
        if ( resultNum == null ) return;
        else if ( UNKNOWN_ERR == ctrl.ByteArrayToInt(resultNum) ) return;

        int numOfValue = ctrl.ByteArrayToInt16(resultNum, NxCinemaCtrl.FORMAT_INT16);
        for( int i = 0; i < numOfValue; i++ ) {

            byte[] resultPos = ctrl.Send( NxCinemaCtrl. CMD_TCON_OPEN_POS, new byte[]{mId} );
            if ( resultPos == null ) continue;
            else if( 0xFFFFFFFF == ctrl.ByteArrayToInt(resultPos) ) continue;   //UNKNOWN_ERR

            int posX = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_MSB);
            int posY = ctrl.ByteArrayToInt16( resultPos, NxCinemaCtrl.MASK_INT16_LSB);
            mAdapter.add( new LedPosInfo(String.valueOf(i), String.valueOf(posX), String.valueOf(posY)));
        }

        ret = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{mId} );
        if ( SUCCESS != ctrl.ByteArrayToInt(ret) ) Log.i(NxTag , "CMD_TCON_MODE_NORMAL 0x09 fail.");
    } //StatusDetailAdapterAsyncTaskLedPos


    private VersionAsyncParam mVersionAsyncParam = new VersionAsyncParam();
    public VersionAsyncParam getVersionAsyncParam () {
        if(null == mVersionAsyncParam ){
            Log.i(NxTag, " mVersionAsyncParam null");
        }
        return mVersionAsyncParam;
    }

    private void Version() {
        byte[] ret;
        ret = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
        mVersionAsyncParam.setNapVersion(ret);

        ret = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
        mVersionAsyncParam.setSapVersion(ret);

        ret = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
        mVersionAsyncParam.setSrvVersion(ret);

        ret = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
        mVersionAsyncParam.setClnVersion(ret);

        ret = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );
        mVersionAsyncParam.setPfpgaVersion(ret);
    } // Version

    private void StopTestPattern() {
        if( mCabinet.length == 0 )
            return;

        String[] mFuncName = {
                "DCI",
                "Color Bar",
                "Full Screen Color",
                "Gary Scale ( Horizontal )",
                "Dot Pattern / Checker Board",
                "Diagonal Pattern",
                "Cabinet ID",
                "Dot Correction",
                "Gamut Mapping",
                "XYZ to RGB",
                "White Seam Value",
        };
        int[] mPatternReg = {
                0x0044,     // REG_FLASH_CC
                0x0052,     // REG_CC_MODULE
                0x0004,     // REG_XYZ_TO_RGB
                0x0192,     // REG_SEAM_ON
        };

        int[] mPatternDat = mDisplayCheckAsyncParam.getmPatternDat();

//        SelectRunAdapter mAdapterTestPattern = mDisplayCheckAsyncParam.getmAdapterTestPattern();
//
//        for( int i = 0; i < mAdapterTestPattern.getCount(); i++ ) {
//            mAdapterTestPattern.reset(i);
//        }

        Log.i(NxTag, ">>> restore default pattern register.");
        for( int i = 0; i < mFuncName.length; i++ ) {
            if( i < 7 ) {
                byte[] data0 = { (byte)0x09, (byte)i, (byte)0x00 };
                byte[] data1 = { (byte)0x89, (byte)i, (byte)0x00 };

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data1 );
            }
            else {
                byte[] reg = ctrl.IntToByteArray(mPatternReg[i-7], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mPatternDat[i-7], NxCinemaCtrl.FORMAT_INT16);
                byte[] data = ctrl.AppendByteArray(reg, dat);
                byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
                byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
            }
        }
        Log.i(NxTag, ">>> restore default pattern register done.");
    } // StopTestPattern

    private void DisplayModeUniformityBtn () {
        boolean bEnable = mDisplayModeAsyncParam.isbEnable();
        if( bEnable ) {
            byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN , NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
            if( result == null || result.length == 0 || result[0] == (byte)0xFF ) {
                Log.i(NxTag, String.format(Locale.US, "i2c write fail.( reg: 0x%04X, dat: 0x%04X )", NxCinemaCtrl.REG_PFPGA_NUC_EN, 0x0000 ));
            }
        }
        else {
            byte[] reg = ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN , NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);
            if( result == null || result.length == 0 || result[0] == (byte)0xFF ) {
                Log.i(NxTag, String.format(Locale.US, "i2c write fail.( reg: 0x%04X, dat: 0x%04X )", NxCinemaCtrl.REG_PFPGA_NUC_EN, 0x0001 ));
            }
        }
    } //DisplayModeUniformityBtn

    private void DisplayModeInitWhiteSeam() {

        byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
    } //DisplayModeInitWhiteSeam

    private void DisplayModeApplyMastering() {
        int itemIdx = mDisplayModeAsyncParam.getItemIdx();
        int value = mDisplayModeAsyncParam.getValue();
        int[] mRegMastering = {
                0x0056,     0x0057,     0x0058,     //  REG_CC00,       REG_CC01,       REG_CC02
                0x0059,     0x005A,     0x005B,     //  REG_CC10,       REG_CC11,       REG_CC12
                0x005C,     0x005D,     0x005E,     //  REG_CC20,       REG_CC21,       REG_CC22
                0x00DD,                             //  REG_BC
                0x00B9,     0x00BA,     0x00BB,     //  REG_CC_R01,     REG_CC_G01,     REG_CC_B01
                0x00B6,     0x00B7,     0x00B8,     //  REG_LGSE1_R,    REG_LGSE1_G,    REG_LGSE1_B
        };

        byte[] reg = ctrl.IntToByteArray(mRegMastering[itemIdx], NxCinemaCtrl.FORMAT_INT16);
        byte[] data = ctrl.IntToByteArray(value, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg ,data);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
    } //DisplayModeApplyMastering

    private void DisplayModeUpdateUnif () {
        byte[] resultEnable = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_READ, ctrl.IntToByteArray(NxCinemaCtrl.REG_PFPGA_NUC_EN, NxCinemaCtrl.FORMAT_INT16) );
        mDisplayModeAsyncParam.setResultEnable(resultEnable);
    } // DisplayModeUpdateUnif

    private void DisplayModeCheckChanged () {
        int selectedPos = mDisplayModeAsyncParam.getmSpinnerWhiteSeamCabinetIdSelected();
        boolean isChecked = mDisplayModeAsyncParam.isChecked();
        if( selectedPos == 0 ) {
            byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(isChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }
        else {
            int pos = selectedPos - 1;
            byte slave = ((mCabinet[pos] % 16) < 8) ? (mCabinet[pos]) : (byte)(mCabinet[pos] | 0x80);
            byte[] inData;
            inData = new byte[] { slave };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16));
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(isChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16));

            ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
        }
    } //DisplayModeCheckChanged

    private void DisplayCheckUpdateTestPattern() {
        int[] mPatternReg = {
                0x0044,     // REG_FLASH_CC     :: restore default value
                0x0052,     // REG_CC_MODULE    :: restore default value
                0x0004,     // REG_XYZ_TO_RGB   :: restore default value
                0x0192,     // REG_SEAM_ON      :: restore default value
        };
        int[] mPatternDat = mDisplayCheckAsyncParam.getmPatternDat();

        if( mCabinet.length == 0 )
            return;

        for( int i = 0; i < mPatternReg.length; i++ )
        {
            byte[] result, inData;
            inData = new byte[] { mCabinet[0] };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mPatternReg[i], NxCinemaCtrl.FORMAT_INT16));

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
            if( result == null || result.length == 0 ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mPatternReg[i] ));
                return;
            }

            mPatternDat[i] = ctrl.ByteArrayToInt(result);
        }

        mDisplayCheckAsyncParam.setmPatternDat(mPatternDat);
        for( int i = 0 ; i < mPatternReg.length; i++ )
        {
            Log.i(NxTag, String.format(">>> read default pattern register. ( reg: 0x%04X, dat: 0x%04X )", mPatternReg[i], mPatternDat[i]) );
        }
    } //DisplayCheckUpdateTestPattern

    private void DisplayModeGlobalRead() {
        if( mCabinet.length == 0 )
            return;

        int [] globalReg = { 0x018B, 0x018C, 0x018A, 0x018D, 0x018E, 0x0192, 0x0055, 0x0004, 0x0100, 0x011E };
        int [] globalVal = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

        for( int i = 0; i < globalReg.length; i++ )
        {
            byte[] result, inData;
            inData = new byte[] { mCabinet[0] };
            inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(globalReg[i], NxCinemaCtrl.FORMAT_INT16));

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
            if( result == null || result.length == 0 ) {
                Log.i(NxTag, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], globalReg[i] ));
                return;
            }

            globalVal[i] = ctrl.ByteArrayToInt(result);
        }

        mDisplayModeAsyncParam.setGlobalVal(globalVal);
        sendCommand.onProgressUpdate();

    } //DisplayModeGlobalRead

    private void DisplayModeLodReset() {
        if( mCabinet.length == 0 ) {
            return;
        }
        byte[] reg, dat, inData, inData0, inData1;
        {
            reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
            dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
            inData = ctrl.AppendByteArray(reg, dat);
            inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
            inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
            if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
            if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

            try {
                Thread.sleep(75);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        {
            reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
            dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
            inData = ctrl.AppendByteArray(reg, dat);
            inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
            inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
            if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
            if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

            try {
                Thread.sleep(75);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        {
            reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
            dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
            inData = ctrl.AppendByteArray(reg, dat);
            inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
            inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
            if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
            if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

            try {
                Thread.sleep(75);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

    } //DisplayModeLodReset

    private void DisplayModeApplySyncWidth() {
        int SpinSyncWidthValue = mDisplayModeAsyncParam.getmSpinSyncWidthValue();
        Log.i(NxTag, String.format(Locale.US, "Apply SYNC_WIDTH. ( %d )", SpinSyncWidthValue));

        if( mCabinet.length == 0 )
            return ;

        byte[] reg = ctrl.IntToByteArray(0x018B, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(SpinSyncWidthValue, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x018B, String.valueOf(SpinSyncWidthValue) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();

    } //DisplayModeApplySyncWidth

    private void DisplayModeApplySyncDelay() {
        int mSpinSyncDelayValue = mDisplayModeAsyncParam.getmSpinSyncDelayValue();
        Log.i(NxTag, String.format(Locale.US, "Apply SYNC_DELAY. ( %d )", mSpinSyncDelayValue));
        if( mCabinet.length == 0 )
            return ;

        byte[] reg = ctrl.IntToByteArray(0x018C, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mSpinSyncDelayValue, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x018C, String.valueOf(mSpinSyncDelayValue) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplySyncDelay

    private void DisplayModeApplySyncReverse() {

        boolean mCheckSyncReverseChecked = mDisplayModeAsyncParam.isUIChecked();

        Log.i(NxTag, String.format(Locale.US, "Change SYNC_REVERSE. ( %b )", mCheckSyncReverseChecked));
        if( mCabinet.length == 0 )
            return ;

        byte[] reg = ctrl.IntToByteArray(0x018A, NxCinemaCtrl.FORMAT_INT16);
        byte[] dat = ctrl.IntToByteArray(mCheckSyncReverseChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
        byte[] inData = ctrl.AppendByteArray(reg, dat);

        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x018A, String.valueOf(mCheckSyncReverseChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplySyncReverse

    private void DisplayModeApplyScale() {
        boolean mCheckScaleChecked = mDisplayModeAsyncParam.isUIChecked();

        Log.i(NxTag, String.format(Locale.US, "Change Scale. ( %b )", mCheckScaleChecked));
        if( mCabinet.length == 0 )
            return ;

        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        {
            byte[] reg = ctrl.IntToByteArray(0x0199, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckScaleChecked ? 0x0000 : 0x0001, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
        }
        {
            byte[] reg = ctrl.IntToByteArray(0x018D, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckScaleChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_PREG_0x0199, String.valueOf(mCheckScaleChecked ? 0 : 1) );
        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x018D, String.valueOf(mCheckScaleChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyScale

    private void DisplayModeApplyZeroScale() {
        boolean mCheckZeroScaleChecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change Zero Scale. ( %b )", mCheckZeroScaleChecked));
        if( mCabinet.length == 0 )
            return ;

        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
        {
            byte[] reg = ctrl.IntToByteArray(0x018E, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckZeroScaleChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x018E, String.valueOf(mCheckZeroScaleChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyZeroScale

    private void DisplayModeApplySeam() {
        boolean mCheckSeamChecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change Seam. ( %b )", mCheckSeamChecked));
        if( mCabinet.length == 0 )
            return ;

        {
            byte[] reg = ctrl.IntToByteArray(0x0192, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckSeamChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x0192, String.valueOf(mCheckSeamChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplySeam

    private void DisplayModeApplyModule() {
        boolean mCheckModuleChecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change Module. ( %b )", mCheckModuleChecked ));
        if( mCabinet.length == 0 )
            return ;

        {
            byte[] reg = ctrl.IntToByteArray(0x0055, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckModuleChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x0055, String.valueOf(mCheckModuleChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyModule

    private void DisplayModeApplyXyzInput() {
        boolean mCheckXyzInputhecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change XyzInput. ( %b )", mCheckXyzInputhecked));
        if( mCabinet.length == 0 )
            return ;

        {
            byte[] reg = ctrl.IntToByteArray(0x0004, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckXyzInputhecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x0004, String.valueOf(mCheckXyzInputhecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyXyzInput

    private void DisplayModeApplyLedOpenDetection() {
        boolean mCheckLedOpenDetectionChecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change Led Open Detection. ( %b )", mCheckLedOpenDetectionChecked));
        if( mCabinet.length == 0 )
            return ;

        {
            byte[] reg = ctrl.IntToByteArray(0x0100, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckLedOpenDetectionChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x0100, String.valueOf(mCheckLedOpenDetectionChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyLedOpenDetection

    private void DisplayModeApplyLodRemoval() {
        boolean mCheckLodRemovalChecked = mDisplayModeAsyncParam.isUIChecked();
        Log.i(NxTag, String.format(Locale.US, "Change Lod Removal. ( %b )", mCheckLodRemovalChecked));
        if( mCabinet.length == 0 )
            return ;

        {
            byte[] reg = ctrl.IntToByteArray(0x011E, NxCinemaCtrl.FORMAT_INT16);
            byte[] dat = ctrl.IntToByteArray(mCheckLodRemovalChecked ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
            byte[] inData = ctrl.AppendByteArray(reg, dat);

            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
        }

        ((CinemaInfo)appContext).SetValue(CinemaInfo.KEY_TREG_0x011E, String.valueOf(mCheckLodRemovalChecked ? 1 : 0) );
        ((CinemaInfo)appContext).UpdateDefaultRegister();
    } //DisplayModeApplyLodRemoval

    int socketVal = 0;

    public int getSocketVal() {
        return socketVal;
    }

    public void setSocketVal(int socketVal) {
        this.socketVal = socketVal;
    }

    private void AsyncTaskChangeContents() {
        int mInitMode = getSocketVal();
        int mUpdateGamma[] = new int[4];
        mUpdateGamma[0] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM0));
        mUpdateGamma[1] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM1));
        mUpdateGamma[2] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM0));
        mUpdateGamma[3] = Integer.parseInt(((CinemaInfo)appContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM1));

        Log.i(NxTag, String.format(">>> MODE( %d ), GAM0( %d ), GAM1( %d ), GAM2( %d ), GAM3( %d )",
                mInitMode, mUpdateGamma[0], mUpdateGamma[1], mUpdateGamma[2], mUpdateGamma[3] ));


        if( mCabinet.length == 0 )
            return;

        //
        //  Check TCON Booting Status
        //
        if( ((CinemaInfo)appContext).IsCheckTconBooting() ) {
            boolean bTconBooting = CheckTCONBootingStatus();

            if( !bTconBooting ) {
                Log.i(NxTag, "Fail, TCON booting.");
                return;
            }
        }

        //
        //  PFPGA Mute on
        //
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

        //  Parse P_REG.txt
        int enableUniformity = ParsePREGtxt(mInitMode);

        //  Auto Uniformity Correction Writing
        if(1 == enableUniformity) AutoUniformityCorrectionWriting(enableUniformity);

        //  Parse T_REG.txt
        ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
        ConfigTconInfo tconUsbInfo = new ConfigTconInfo();
        int[] enableGamma = ParseTREGtxt( mInitMode , tconEEPRomInfo , tconUsbInfo , true);

        //  Write Gamma
        WriteGamma(mInitMode , tconEEPRomInfo , enableGamma , mUpdateGamma);

        //
        //  Update Gamma Status
        //
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
        ((CinemaInfo)appContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

        //
        //  SW Reset
        //
        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

        //
        //  PFPGA Mute off
        //
        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );
    } //AsyncTaskChangeContents
} //NXAsync class
