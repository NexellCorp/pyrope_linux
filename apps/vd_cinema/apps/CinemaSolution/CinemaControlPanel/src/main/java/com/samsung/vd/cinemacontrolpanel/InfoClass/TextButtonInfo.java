package com.samsung.vd.cinemacontrolpanel.InfoClass;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.TextButtonAdapter;

/**
 * Created by doriya on 10/20/17.
 */

public class TextButtonInfo {
    private String mIndex = "";
    private String mMode = "";
    private String mText = "";
    private boolean mEnable = false;
    private TextButtonAdapter.OnClickListener mOnClickListener = null;
    private TextButtonAdapter.OnClickListener mRemoveOnClickListener = null;
    private TextButtonAdapter.OnClickListener mUpdateOnClickListener = null;

    public TextButtonInfo( String title, String text) {
        mIndex = title;
        mText = text;
        mEnable = !mIndex.equals("") && !mText.equals("");
    }

    public TextButtonInfo( String title, String text, TextButtonAdapter.OnClickListener listener ) {
        mIndex = title;
        mText = text;
        mEnable = !mIndex.equals("") && !mText.equals("");
        mOnClickListener = listener;
    }

    public TextButtonInfo( String title, String mode ,String text, TextButtonAdapter.OnClickListener listener , TextButtonAdapter.OnClickListener removelistener , TextButtonAdapter.OnClickListener updatelistener ) {
        mIndex = title;
        mMode = mode;
        mText = text;
        mEnable = !mIndex.equals("") && !mText.equals("");
        mOnClickListener = listener;
        mRemoveOnClickListener = removelistener;
        mUpdateOnClickListener = updatelistener;
    }

    public String GetMode() {
        return mMode;
    }

    public void SetMode(String mode) {
        mMode = mode;
    }

    public String GetIndex() {
        return mIndex;
    }

    public void SetIndex(String title ) {
        mIndex = title;
    }

    public String GetText() {
        return mText;
    }

    public void SetText( String text) {
        mText = text;
    }

    public boolean GetEnable() {
        return mEnable;
    }

    public void SetEnable( boolean enable ) {
        mEnable = enable;
    }

    public void SetOnClickListener(TextButtonAdapter.OnClickListener listener ) {
        mOnClickListener = listener;
    }

    public TextButtonAdapter.OnClickListener GetOnClickListener() {
        return mOnClickListener;
    }

    public TextButtonAdapter.OnClickListener GetRemoveOnClickListener() {
        return mRemoveOnClickListener;
    }

    public TextButtonAdapter.OnClickListener GetUpdateOnClickListener() {
        return mUpdateOnClickListener;
    }

}

