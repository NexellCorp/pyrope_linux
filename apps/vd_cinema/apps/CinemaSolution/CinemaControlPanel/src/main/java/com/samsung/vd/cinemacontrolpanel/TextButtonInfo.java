package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 10/20/17.
 */

public class TextButtonInfo {
    private String mTitle = "";
    private String mText = "";
    private boolean mEnable = false;
    private TextButtonAdapter.OnClickListener mOnClickListener = null;

    public TextButtonInfo( String title, String text) {
        mTitle = title;
        mText = text;
        mEnable = !mTitle.equals("") && !mText.equals("");
    }

    public TextButtonInfo( String title, String text, TextButtonAdapter.OnClickListener listener ) {
        mTitle = title;
        mText = text;
        mEnable = !mTitle.equals("") && !mText.equals("");
        mOnClickListener = listener;
    }

    public String GetTitle() {
        return mTitle;
    }

    public void SetTitle( String title ) {
        mTitle = title;
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

}

