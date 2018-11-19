package com.samsung.vd.cinemacontrolpanel;

/**
 * Created by doriya on 10/20/17.
 */

public class TextButtonInfo {
    public static final int MAX_BTN_NUM = 4;

    private String mTitle = "";
    private String mText = "";
    private boolean mTextEnable = false;
    private boolean[] mBtnEnable = new boolean[MAX_BTN_NUM];
    private TextButtonAdapter.OnClickListener[] mOnClickListener = new TextButtonAdapter.OnClickListener[MAX_BTN_NUM];

    public TextButtonInfo( String title, String text) {
        mTitle      = title;
        mText       = text;
        mTextEnable = !mTitle.equals("") && !mText.equals("");

        for( int i = 0; i < mBtnEnable.length; i++ )
            mBtnEnable[i] = mTextEnable;
    }

    public TextButtonInfo( String title, String text, TextButtonAdapter.OnClickListener[] listener ) {
        mTitle      = title;
        mText       = text;
        mTextEnable = !mTitle.equals("") && !mText.equals("");

        for( int i = 0; i < mBtnEnable.length; i++ )
            mBtnEnable[i] = mTextEnable;

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

    public boolean GetTextEnable() {
        return mTextEnable;
    }

    public void SetTextEnable( boolean enable ) {
        mTextEnable = enable;
    }

    public boolean GetBtnEnable(int button) {
        return (button < MAX_BTN_NUM) && mBtnEnable[button];
    }

    public void SetBtnEnable(int button, boolean enable) {
        if( button < MAX_BTN_NUM )
            mBtnEnable[button] = enable;
    }

    public void SetOnClickListener(TextButtonAdapter.OnClickListener[] listener ) {
        mOnClickListener = listener;
    }

    public TextButtonAdapter.OnClickListener GetOnClickListener( int button ) {
        return (button < MAX_BTN_NUM) ? mOnClickListener[button] : null;
    }

}

