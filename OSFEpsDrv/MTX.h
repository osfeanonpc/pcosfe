#pragma once

template<typename TLock>
struct MTX {
	MTX(TLock& lock) : m_Lock(lock) {
		lock.Lock();
	}
	~MTX() {
		m_Lock.Unlock();
	}

private:
	TLock& m_Lock;
};
