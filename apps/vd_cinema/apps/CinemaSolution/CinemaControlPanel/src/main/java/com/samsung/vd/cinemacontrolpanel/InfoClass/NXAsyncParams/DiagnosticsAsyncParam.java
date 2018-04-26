package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import android.widget.CheckBox;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDescribeAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusDetailAdapter;
import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.StatusSimpleAdapter;

/**
 * Created by spjun on 18. 4. 10.
 */

public class DiagnosticsAsyncParam {

    //
    //AsyncTconStatus
    //
    StatusSimpleAdapter TCONStatusAdapter;

    public StatusSimpleAdapter getTCONStatusAdapter() {
        return TCONStatusAdapter;
    }

    public void setTCONStatusAdapter(StatusSimpleAdapter statusSimpleAdapter) {
        this.TCONStatusAdapter = statusSimpleAdapter;
    }
    //Async to Activity
    int TCON , TCONStatusResult;

    public int getTCON() {
        return TCON;
    }

    public void setTCON(int TCON) {
        this.TCON = TCON;
    }

    public int getTCONStatusResult() {
        return TCONStatusResult;
    }

    public void setTCONStatusResult(int Result) {
        this.TCONStatusResult = Result;
    }



    //
    //AsyncTconLvds
    //
    StatusSimpleAdapter TconLvdsAdapter;

    public StatusSimpleAdapter getTconLvdsAdapter() {
        return TconLvdsAdapter;
    }

    public void setTconLvdsAdapter(StatusSimpleAdapter tconLvdsAdapter) {
        TconLvdsAdapter = tconLvdsAdapter;
    }
    //Async to Activity
    int TCONLvds , TCONLvdsResult;

    public int getTCONLvds() {
        return TCONLvds;
    }

    public void setTCONLvds(int TCONLvds) {
        this.TCONLvds = TCONLvds;
    }

    public int getTCONLvdsResult() {
        return TCONLvdsResult;
    }

    public void setTCONLvdsResult(int TCONLvdsResult) {
        this.TCONLvdsResult = TCONLvdsResult;
    }


    //
    //AsyncTaskLedOpenNum
    //
    StatusDetailAdapter LedOpenNumAdapter;

    public StatusDetailAdapter getLedOpenNumAdapter() {
        return LedOpenNumAdapter;
    }

    public void setLedOpenNumAdapter(StatusDetailAdapter ledOpenNumAdapter) {
        LedOpenNumAdapter = ledOpenNumAdapter;
    }
    //Async to Activity
    int LedOpenNumCabinetID , LedOpenNumSlaveAddr , LedOpenNumResult;

    public int getLedOpenNumCabinetID() {
        return LedOpenNumCabinetID;
    }

    public void setLedOpenNumCabinetID(int ledOpenNumCabinetID) {
        LedOpenNumCabinetID = ledOpenNumCabinetID;
    }

    public int getLedOpenNumSlaveAddr() {
        return LedOpenNumSlaveAddr;
    }

    public void setLedOpenNumSlaveAddr(int ledOpenNumSlaveAddr) {
        LedOpenNumSlaveAddr = ledOpenNumSlaveAddr;
    }

    public int getLedOpenNumResult() {
        return LedOpenNumResult;
    }

    public void setLedOpenNumResult(int ledOpenNumResult) {
        LedOpenNumResult = ledOpenNumResult;
    }


    //
    //AsyncTaskFullWhite
    //
    boolean FullWhiteRestore;

    public boolean isFullWhiteRestore() {
        return FullWhiteRestore;
    }

    public void setFullWhiteRestore(boolean fullWhiteRestore) {
        FullWhiteRestore = fullWhiteRestore;
    }


    //
    //AsyncTaskLedOpenDetect
    //
    boolean LedOpenDetect;

    public boolean isLedOpenDetect() {
        return LedOpenDetect;
    }

    public void setLedOpenDetect(boolean ledOpenDetect) {
        LedOpenDetect = ledOpenDetect;
    }


    //
    //AsyncTaskLodRemovalWrite  not used at the moment..
    //
    boolean LodRemovalWrite1 , LodRemovalWrite2;

    public boolean isLodRemovalWrite1() {
        return LodRemovalWrite1;
    }

    public void setLodRemovalWrite1(boolean lodRemovalWrite1) {
        LodRemovalWrite1 = lodRemovalWrite1;
    }

    public boolean isLodRemovalWrite2() {
        return LodRemovalWrite2;
    }

    public void setLodRemovalWrite2(boolean lodRemovalWrite2) {
        LodRemovalWrite2 = lodRemovalWrite2;
    }


    //
    //AsyncTaskLodRemovalRead  not used at the moment..
    //

    CheckBox LodRemovalRead1 , LodRemovalRead2;

    public CheckBox getLodRemovalRead1() {
        return LodRemovalRead1;
    }

    public void setLodRemovalRead1(CheckBox lodRemovalRead1) {
        LodRemovalRead1 = lodRemovalRead1;
    }

    public CheckBox getLodRemovalRead2() {
        return LodRemovalRead2;
    }

    public void setLodRemovalRead2(CheckBox lodRemovalRead2) {
        LodRemovalRead2 = lodRemovalRead2;
    }

    //Async to Activity
    boolean LodRemovalReadFlag1 , LodRemovalReadFlag2;

    public boolean isLodRemovalReadFlag1() {
        return LodRemovalReadFlag1;
    }

    public void setLodRemovalReadFlag1(boolean lodRemovalReadFlag1) {
        LodRemovalReadFlag1 = lodRemovalReadFlag1;
    }

    public boolean isLodRemovalReadFlag2() {
        return LodRemovalReadFlag2;
    }

    public void setLodRemovalReadFlag2(boolean lodRemovalReadFlag2) {
        LodRemovalReadFlag2 = lodRemovalReadFlag2;
    }


    //
    //AsyncTaskCabinetDoor
    //
    StatusSimpleAdapter CabinetDoorAdapter;

    public StatusSimpleAdapter getCabinetDoorAdapter() {
        return CabinetDoorAdapter;
    }

    public void setCabinetDoorAdapter(StatusSimpleAdapter cabinetDoorAdapter) {
        CabinetDoorAdapter = cabinetDoorAdapter;
    }

    //Async to Activity
    int CabinetDoorCabinet , CabinetDoorResult;

    public int getCabinetDoorCabinet() {
        return CabinetDoorCabinet;
    }

    public void setCabinetDoorCabinet(int cabinetDoorCabinet) {
        CabinetDoorCabinet = cabinetDoorCabinet;
    }

    public int getCabinetDoorResult() {
        return CabinetDoorResult;
    }

    public void setCabinetDoorResult(int cabinetDoorResult) {
        CabinetDoorResult = cabinetDoorResult;
    }


    //
    //AsyncTaskPeripheral
    //
    StatusSimpleAdapter PeripheralAdapter;

    public StatusSimpleAdapter getPeripheralAdapter() {
        return PeripheralAdapter;
    }

    public void setPeripheralAdapter(StatusSimpleAdapter peripheralAdapter) {
        PeripheralAdapter = peripheralAdapter;
    }


    //
    //AsyncTaskVersion
    //
    StatusDescribeAdapter VersionAdapter;

    public StatusDescribeAdapter getVersionAdapter() {
        return VersionAdapter;
    }

    public void setVersionAdapter(StatusDescribeAdapter versionAdapter) {
        VersionAdapter = versionAdapter;
    }

    //Async to Activity
    int VersionCabinet , Version1 , Version2;

    public int getVersionCabinet() {
        return VersionCabinet;
    }

    public void setVersionCabinet(int versionCabinet) {
        VersionCabinet = versionCabinet;
    }

    public int getVersion1() {
        return Version1;
    }

    public void setVersion1(int version1) {
        Version1 = version1;
    }

    public int getVersion2() {
        return Version2;
    }

    public void setVersion2(int version2) {
        Version2 = version2;
    }
} //DiagnosticsAsyncParam
