package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import android.util.Log;
import android.widget.Button;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.CheckRunAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;

/**
 * Created by spjun on 18. 4. 11.
 */

public class DisplayModeAsyncParam {
    //
    //AsyncTaskMasteringWrite
    //
    boolean MasteringWriteMode;
    int MasteringWriteModePos;

    public boolean isMasteringWriteMode() {
        return MasteringWriteMode;
    }

    public void setMasteringWriteMode(boolean masteringWriteMode) {
        MasteringWriteMode = masteringWriteMode;
    }

    public int getMasteringWriteModePos() {
        return MasteringWriteModePos;
    }

    public void setMasteringWriteModePos(int masteringWriteModePos) {
        MasteringWriteModePos = masteringWriteModePos;
    }



    //
    //AsyncTaskMasteringRead
    //
    int[] MasteringReadValue;

    public int[] getMasteringReadValue() {
        return MasteringReadValue;
    }

    public void allocMasteringReadValue(int size) {
        MasteringReadValue = new int[size];
    }

    public void setMasteringReadValue(int pos , int val) {
        if (null == MasteringReadValue){
            Log.i("setMasteringReadValue" , "MasteringReadValue is null");
        }else
        {
            Log.i("setMasteringReadValue" , "length :  "+MasteringReadValue.length);
        }
        Log.i("setMasteringReadValue" , "pos :  "+pos + "val :  "+val);
        MasteringReadValue[pos] = val;
    }

    public int getMasteringReadValueinPos(int pos) {
        return MasteringReadValue[pos];
    }


    //
    //AsyncTaskUniformityCorrection
    //
    int mIndexUniformity;

    public int getmIndexUniformity() {
        return mIndexUniformity;
    }

    public void setmIndexUniformity(int mIndexUniformity) {
        this.mIndexUniformity = mIndexUniformity;
    }

    //
    //AsyncTaskAdapterDotCorrection
    //
    CheckRunAdapter mAdapter;

    public CheckRunAdapter getmAdapter() {
        return mAdapter;
    }

    public void setmAdapter(CheckRunAdapter mAdapter) {
        this.mAdapter = mAdapter;
    }

    //Async ret
    String dir;

    public String getDir() {
        return dir;
    }

    public void setDir(String dir) {
        this.dir = dir;
    }


    //
    //AsyncTaskDotCorrection
    //
    CheckRunAdapter mAdapterDot;

    public CheckRunAdapter getmAdapterDot() {
        return mAdapterDot;
    }

    public void setmAdapterDot(CheckRunAdapter mAdapterDot) {
        this.mAdapterDot = mAdapterDot;
    }

    //async ret
    int idx , len , success , fail;

    public void setProgress(int index ,int length ,int bSuccess ,int bFail) {
        idx = index;
        len = length;
        success = bSuccess;
        fail = bFail;
    }
    public int getIdx() {
        return idx;
    }

    public int getLen() {
        return len;
    }

    public int getSuccess() {
        return success;
    }

    public int getFail() {
        return fail;
    }



    //
    //AsyncTaskDotCorrectionExtract
    //
    int mId , mModule;
    String externalStorage;

    public int getmId() {
        return mId;
    }

    public void setmId(int mId) {
        this.mId = mId;
    }

    public int getmModule() {
        return mModule;
    }

    public void setmModule(int mModule) {
        this.mModule = mModule;
    }

    public String getExternalStorage() {
        return externalStorage;
    }

    public void setExternalStorage(String externalStorage) {
        this.externalStorage = externalStorage;
    }


    //
    //AsyncTaskWhiteSeamRead
    //
    int mIndexPos;
    boolean mEmulate;

    public void setIdxPosbEmulate(int idx , boolean bEmul) {
        mIndexPos = idx;
        mEmulate = bEmul;
    }

    public int getmIndexPos() {
        return mIndexPos;
    }


    public boolean ismEmulate() {
        return mEmulate;
    }


    //async ret
    int[] mSeamVal = new int[4];

    public void setmSeamVal(int[] mSeamVal) {
        this.mSeamVal = mSeamVal;
    }

    public int[] getmSeamVal() {
        return mSeamVal;
    }



    //
    //AsyncTaskWhiteSeamEmulate
    //
    int indexPos;
    int[] seamVal;

    public int getIndexPos() {
        return indexPos;
    }

    public void setIndexPos(int indexPos) {
        this.indexPos = indexPos;
    }

    public int[] getSeamVal() {
        return seamVal;
    }

    public void setSeamVal(int[] seamVal) {
        this.seamVal = seamVal;
    }


    //
    //AsyncTaskWhiteSeamWrite
    //

    int pos;
    int[] seamvalue;

    public int getPos() {
        return pos;
    }

    public void setPos(int pos) {
        this.pos = pos;
    }

    public int[] getSeamvalue() {
        return seamvalue;
    }

    public void setSeamvalue(int[] seamval) {
        this.seamvalue = seamval;
    }


    //
    //mBtnEnableUniformity
    byte[] resultEnable;
    boolean bEnable;

    public byte[] getResultEnable() {
        return resultEnable;
    }

    public void setResultEnable(byte[] resultEnable) {
        this.resultEnable = resultEnable;
    }

    public boolean isbEnable() {
        return bEnable;
    }

    public void setbEnable(boolean bEnable) {
        this.bEnable = bEnable;
    }

    //
    //ApplyMasteringMode
    int itemIdx , value;

    public int getItemIdx() {
        return itemIdx;
    }

    public void setItemIdx(int itemIdx) {
        this.itemIdx = itemIdx;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }


    //
    //UpdateUniformityCorrection
//    Button mBtnEnableUniformity;
    //use above


    //
    //mCheckWhiteSeamEmulation onCheckedChanged
    int mSpinnerWhiteSeamCabinetIdSelected;
    boolean isChecked;

    public int getmSpinnerWhiteSeamCabinetIdSelected() {
        return mSpinnerWhiteSeamCabinetIdSelected;
    }

    public void setmSpinnerWhiteSeamCabinetIdSelected(int mSpinnerWhiteSeamCabinetIdSelected) {
        this.mSpinnerWhiteSeamCabinetIdSelected = mSpinnerWhiteSeamCabinetIdSelected;
    }

    public boolean isChecked() {
        return isChecked;
    }

    public void setChecked(boolean checked) {
        isChecked = checked;
    }


    //GlobalRead
    int [] globalVal;

    public int[] getGlobalVal() {
        return globalVal;
    }

    public void setGlobalVal(int[] globalVal) {
        this.globalVal = globalVal;
    }




    //ApplySyncWidth
    int mSpinSyncWidthValue;

    public int getmSpinSyncWidthValue() {
        return mSpinSyncWidthValue;
    }

    public void setmSpinSyncWidthValue(int mSpinSyncWidthValue) {
        this.mSpinSyncWidthValue = mSpinSyncWidthValue;
    }




    //ApplySyncDelay
    int mSpinSyncDelayValue;

    public int getmSpinSyncDelayValue() {
        return mSpinSyncDelayValue;
    }

    public void setmSpinSyncDelayValue(int mSpinSyncDelayValue) {
        this.mSpinSyncDelayValue = mSpinSyncDelayValue;
    }






    //ApplySyncReverse
    //ApplyScale
    //ApplyZeroScale
    //ApplySeam
    //ApplyModule
    //ApplyXyzInput
    //ApplyLedOpenDetection
    //ApplyLodRemoval
    boolean UIChecked;

    public boolean isUIChecked() {
        return UIChecked;
    }

    public void setUIChecked(boolean UIChecked) {
        this.UIChecked = UIChecked;
    }





}

