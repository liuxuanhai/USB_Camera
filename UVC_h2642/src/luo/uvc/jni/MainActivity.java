package luo.uvc.jni;

import java.util.TimerTask;


import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import luo.uvc.jni.R;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.R.bool;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

public class MainActivity extends Activity implements OnClickListener, callback
{

	private String TAG = "UVCCameraPreview";
	private Button mBtnPreview;
	private Button mBtnRecord;
	private CameraPreview mCameraPreview;
	
	protected static final int MSG_PREVIEW = 0;
	protected static final int MSG_RECORD = 1;
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mBtnPreview = (Button) findViewById(R.id.btn_start_preview);
		mBtnRecord = (Button) findViewById(R.id.btn_start_record);
		mCameraPreview = (CameraPreview)findViewById(R.id.preview);

		mBtnRecord.setOnClickListener(this);
		mBtnPreview.setOnClickListener(this);
		
		//ªÿµ˜£∫…Ë÷√button text
		mCameraPreview.textCallback = this;
		
		mCameraPreview.initPreview();
	}
	

	@Override
	public void onClick(View v)
	{
		switch (v.getId())
		{
		case R.id.btn_start_preview:
			sendMsg(MSG_PREVIEW, "");
			break;
		case R.id.btn_start_record:
			sendMsg(MSG_RECORD, "");
			break;
		}
	}
	
	Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);

			switch (msg.what) {
			case MSG_PREVIEW:
				
				mCameraPreview.startPreview();
				break;


			case MSG_RECORD:
				mCameraPreview.initRecord();
				
				break;

			default:
				break;
			}
		}
	};
	
	private void sendMsg(int msgID, Object obj) {
		Message msg = mHandler.obtainMessage();
		msg.what = msgID;
		msg.obj = obj;
		mHandler.sendMessage(msg);
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
	}

	@Override
	protected void onDestroy()
	{
		mCameraPreview.uninitPreview();		
		
		Log.d(TAG, "onCreate method is onDestroy");
		super.onDestroy();
	}


	@Override
	public void setViewText(int id, String str)
	{
		// TODO Auto-generated method stub
		switch (id)
		{
		case CameraPreview.SET_PREVIEW_TEXT:
			mBtnPreview.setText(str);	
			break;
		case CameraPreview.SET_RECORD_TEXT:
			mBtnRecord.setText(str);	
			break;

		default:
			break;
		}
		
	}
}
