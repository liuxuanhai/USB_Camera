package luo.uvc.jni;

import android.support.v7.appcompat.R.integer;

public class ImageProc
{
	static
	{
		System.loadLibrary("image_proc");
	}
	
	public static native int[] grayProc(int[] pixels, int w, int h);
	
	public static native int connectCamera(int device);
	
	public static native FrameInfo getFrame();	
	
	public static native int releaseCamera();
	
	public static native int startRecord(String dateString);
	
	public static native int stopRecord();
	
	public static native int getWidth();
	
	public static native int getHeight();
	
	public static class FrameInfo
	{
		private int mWidth;			//private: int类型
		private int mHeight;			//private: int类型
		private int mImageSize;		
		private int[] mPixels;		//private: int基本数组类型
		
		public int getWidth()
		{
			return mWidth;
		}
		
		public int getHeigth()
		{
			return mHeight;
		}
		
		public int getImageSize()
		{
			return mImageSize;
		}
		
		public int[] getPixels()
		{
			return mPixels;
		}		
	}
}

