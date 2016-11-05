package cc.along.uvcsimple;


import android.graphics.Bitmap;

public class UVCJni {
	static {
		System.loadLibrary("ImageProc");
	}
	
	public final static boolean V4L2_DEVICE = false;
	public final static boolean DEBUG = true;
	
	// JNI functions
	public native static int setDirectBuffer(Object buffer, int length);
	
	public native static int prepareCamera(int videoid);

	public native static int prepareCameraWithBase(int videoid, int camerabase);

	public native static void processCamera();

	public native static void stopCamera();
	
	public native static void startRecording();
	
	public native static void stopRecording();
	
	public native static int getRealImageSize();
	
 	public native static int getBufferSize();
 	
	public native static int tryOpen();
}
