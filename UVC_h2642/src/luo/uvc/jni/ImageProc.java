package luo.uvc.jni;

import android.graphics.Bitmap;
import android.support.v7.appcompat.R.integer;

public class ImageProc
{
	static
	{
		System.loadLibrary("ImageProc");
	}
	
	public static native int connectCamera();	
	
	public static native int releaseCamera();
	
	public static native void getPreviewFrame(Bitmap bitmap);	
	
	public static native int startRecord(String dateString);
	
	public static native int stopRecord();
	
}

