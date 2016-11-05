package luo.uvc.jni;

public abstract interface MyRunnable extends Runnable
{
	
	
	//是否挂起线程
	public void setSuspend(boolean susp);
	
	public boolean isSuspend();
	
	public void runPesonelLogic();
}
