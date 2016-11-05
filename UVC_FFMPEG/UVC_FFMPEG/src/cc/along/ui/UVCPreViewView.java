package cc.along.ui;

import java.nio.ByteBuffer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;


public class UVCPreViewView extends SurfaceView implements SurfaceHolder.Callback{

	private Context mContext;
	private SurfaceHolder mHolder;
	private int width = 320;
	private int height = 240;
	private Rect mRect;
		
	public UVCPreViewView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		initView(context);
	}

	public UVCPreViewView(Context context, AttributeSet attrs) {
		super(context, attrs);
		initView(context);
	}

	public UVCPreViewView(Context context) {
		super(context);
		initView(context);
	}

	private void initView(Context context){
		mHolder = getHolder();
		mHolder.addCallback(this);
		mHolder.setFixedSize(width, height);
	}

	public void setWidth(int width) {
		this.width = width;
	}

	public void setHeight(int height) {
		this.height = height;
	}

	public void  updatePreview(byte[] previewBuffer, int length){
		Canvas canvas = mHolder.lockCanvas();		
		
		Bitmap bmp = BitmapFactory.decodeByteArray(previewBuffer, 0, length);
		canvas.drawBitmap(bmp, null, mRect, null);
		
		mHolder.unlockCanvasAndPost(canvas);
	}
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		mRect = new Rect(0, 0, getWidth(), getHeight());
		
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		
	}
	
}
