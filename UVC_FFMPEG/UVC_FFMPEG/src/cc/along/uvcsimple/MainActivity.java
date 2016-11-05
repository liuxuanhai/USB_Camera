package cc.along.uvcsimple;

import java.nio.ByteBuffer;

import cc.along.ui.UVCPreViewView;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;
import android.os.Build;

public class MainActivity extends ActionBarActivity {

	private static final String TAG = "UVCSimple_main";
	protected static final int MSG_START_RECORDING = 0;
	protected static final int MSG_STOP_RECORDING = 1;
	protected static final int MSG_SHOW_INFO = 2;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		if (savedInstanceState == null) {
			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, new PlaceholderFragment()).commit();
		}
	}

	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class PlaceholderFragment extends Fragment implements
			OnClickListener {

		private boolean isCameraExist = true;
		private boolean isRecording = false;
		private Button mStartBt;
		private Button mStopBt;
		private int mImageBufSize;
		private ByteBuffer mPreViewBuffer;
		private UVCPreViewView mPreViewSurface;

		public PlaceholderFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			int ret;

			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);

			mPreViewSurface = (UVCPreViewView) rootView.findViewById(R.id.preview_id);
			mStartBt = (Button) rootView.findViewById(R.id.bt_start);
			mStopBt = (Button) rootView.findViewById(R.id.bt_stop);
			mStartBt.setOnClickListener(this);
			mStopBt.setOnClickListener(this);

			if (UVCJni.DEBUG) {
				UVCUtil.listDevFile(getActivity());
			}
			ret = UVCJni.prepareCamera(0);
			if (ret < 0) {
				Log.e(TAG, "prepareCamera failed");
				showMsgInfo("prepareCamera failed");
			}

			if (ret > 0) {
				mImageBufSize = UVCJni.getBufferSize();
				mPreViewBuffer = ByteBuffer.allocateDirect(mImageBufSize);
				UVCJni.setDirectBuffer(mPreViewBuffer, mImageBufSize);
				isCameraExist = true;
			}

			return rootView;
		}

		@Override
		public void onClick(View v) {
			switch (v.getId()) {
			case R.id.bt_start:

				// showMsgInfo("tryOpen " + UVCJni.tryOpen() );

				// try {
				// Thread.sleep(3000);
				// } catch (InterruptedException e) {
				// // TODO Auto-generated catch block
				// e.printStackTrace();
				// }

				sendMsg(MSG_START_RECORDING, "");
				break;
			case R.id.bt_stop:
				Log.i(TAG, "Record stop entry");
				sendMsg(MSG_STOP_RECORDING, "");
				Log.i(TAG, "Record stop exit");
				break;
			}

		}

		private void showMsgInfo(String info) {
			sendMsg(MSG_SHOW_INFO, info);
		}

		private void sendMsg(int msgID, Object obj) {
			Message msg = handler.obtainMessage();
			msg.what = msgID;
			msg.obj = obj;
			handler.sendMessage(msg);
		}

		Handler handler = new Handler() {

			@Override
			public void handleMessage(Message msg) {
				super.handleMessage(msg);

				switch (msg.what) {
				case MSG_START_RECORDING:
					

					if (isCameraExist && !isRecording) {
						isRecording = true;
						new Thread() {
							int bufferLength;

							public void run() {

								Log.i(TAG, "Record start entry");
								showMsgInfo("Record start entry");

								while (isRecording) {
									UVCJni.startRecording();
									bufferLength = UVCJni.getRealImageSize();
									Log.i(TAG, "getRealImageSize " + bufferLength);
									mPreViewSurface.updatePreview(mPreViewBuffer.array(), bufferLength);
								}

								UVCJni.stopRecording();
								isRecording = false;
								Log.i(TAG, "Record start exit");
								showMsgInfo("Record start exit");
							}
						}.start();
					} else {
						showMsgInfo("Camera not exist");
					}

					break;

				case MSG_STOP_RECORDING:
					isRecording = false;
					break;

				case MSG_SHOW_INFO:
					String info = (String) msg.obj;
					Toast.makeText(getActivity(), "" + info, 0).show();
					break;

				default:
					break;
				}
			}
		};
	}

}
