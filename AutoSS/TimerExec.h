
#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <chrono>

/*
 * �����ϐ���p���ăX���b�h�Ԃ̓��������N���X
*/
class ThreadSync {
public:
	
	ThreadSync() {
		flag = false;
	}
	
	void Wait() {
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [this]{ return flag; });
		flag = false;
	}
	
	void Notify() {
		{
			std::lock_guard<std::mutex> lock(mtx);
			flag = true;
		}
		cv.notify_all();
	}
	
private:
	std::mutex mtx;
	std::condition_variable cv;
	bool flag;
};



/*
 * ��莞�Ԃ����Ɋ֐����Ăяo���N���X
*/
class TimerExec {
public:
	
	TimerExec(std::function<void(void*)> func, int waitTime) {
		this->FuncExec = func;
		this->WaitTime = waitTime;
		this->pFuncParam = nullptr;
		this->RunningFlag = true;
	}
	
	~TimerExec() {
		Stop();
	}
	
	void SetFuncParam(void *pFuncParam) {
		this->pFuncParam = pFuncParam;
	}
	
	void Start() {
		RunningFlag = true;
		TimerThread = std::thread([this]() { TimerThreadFunc(this); });
		RunnerThread = std::thread([this]() { RunnerThreadFunc(this); });
	}
	
	void Stop() {
		RunningFlag = false;
		if( TimerThread.joinable() ) TimerThread.join();
		if( RunnerThread.joinable() ) RunnerThread.join();
	}
	
private:
	static void RunnerThreadFunc(TimerExec *pSelf) {
		while( pSelf->RunningFlag ) {
			pSelf->FuncExec(pSelf->pFuncParam);
			pSelf->Sync.Wait();
		}
	}
	
	static void TimerThreadFunc(TimerExec *pSelf) {
		while( pSelf->RunningFlag ) {
			std::this_thread::sleep_for(
				std::chrono::milliseconds(pSelf->WaitTime));
			pSelf->Sync.Notify();
		}
	}
	
private:
	std::function<void(void*)> FuncExec;
	void *pFuncParam;
	std::thread TimerThread, RunnerThread;
	int WaitTime;
	ThreadSync Sync;
	bool RunningFlag;
};


