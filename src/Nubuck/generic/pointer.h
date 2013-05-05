#pragma once

#ifndef NULL
#define NULL 0
#endif

namespace GEN {

    template<class T>
	class Pointer {
		template<class U> friend class Pointer;
	private:
		T*			_raw;
		unsigned*	_cnt;

		void Drop(void);
		template<class U> void Copy(const Pointer<U>& other);
	public:
		Pointer(void);
		Pointer(const Pointer& other);
		Pointer(T* const raw);
		template<class U> Pointer(const Pointer<U>& other);
		~Pointer(void);

		Pointer& operator=(const Pointer& other);
		template<class U> Pointer& operator=(const Pointer<U>& other);

		const T*	Raw(void) const;
		T*			Raw(void);
		unsigned	Count(void) const;
	
		bool		IsValid(void) const;

		const T*	operator->(void) const;
		const T&	operator*(void) const;
		T*			operator->(void);
		T&			operator*(void);
	};

	template<class T>
	void Pointer<T>::Drop(void) {
		if(NULL != _raw /* ie. pointer is valid */) {
			if(0 >= --(*_cnt)) {
				delete _raw;
				_raw = NULL;

				delete _cnt;
				_cnt = NULL;
			}
		}
	}

	template<class T>
	template<class U>
	void Pointer<T>::Copy(const Pointer<U>& other) {
		Drop();

		_raw = other._raw;
		_cnt = other._cnt;

		if(NULL != _raw /* ie. pointer is valid */) {
			(*_cnt)++;
		}
	}

	template<class T>
	inline Pointer<T>::Pointer(void) : _raw(NULL), _cnt(NULL) {
	}

	template<class T>
	inline Pointer<T>::Pointer(const Pointer& other) : _raw(other._raw), _cnt(other._cnt) {
		if(NULL != _raw /* ie. pointer is valid */) {
			(*_cnt)++;
		}
	}

	template<class T>
	inline Pointer<T>::Pointer(T* const raw) : _raw(NULL), _cnt(NULL) {
		if(NULL != raw) {
			unsigned* cnt = new unsigned(1);
			_raw = raw;
			_cnt = cnt;
		}
	}

	template<class T>
	template<class U>
	inline Pointer<T>::Pointer(const Pointer<U>& other) : _raw(other._raw), _cnt(other._cnt) {
		if(NULL != _raw) {
			(*_cnt)++;
		}
	}

	template<class T>
	inline Pointer<T>::~Pointer(void) {
		Drop();
	}

	template<class T>
	inline Pointer<T>& Pointer<T>::operator=(const Pointer& other) {
		if(&other != this) Copy(other);
		return *this;
	}

	template<class T>
	template<class U>
	inline Pointer<T>& Pointer<T>::operator=(const Pointer<U>& other) {
		Copy(other);
		return *this;
	}

	template<class T>
	inline const T* Pointer<T>::Raw(void) const {
		return _raw;
	}

	template<class T>
	inline T* Pointer<T>::Raw(void){
		return _raw;
	}

	template<class T>
	inline unsigned Pointer<T>::Count(void) const {
		return *_cnt;
	}

	template<class T>
	inline bool Pointer<T>::IsValid(void) const {
		return NULL != _raw;
	}

	template<class T> 
	const T* Pointer<T>::operator->(void) const {
		return _raw;
	}

	template<class T>
	const T& Pointer<T>::operator*(void) const {
		return *_raw;
	}

	template<class T>
	T* Pointer<T>::operator->(void) {
		return _raw;
	}

	template<class T>
	T& Pointer<T>::operator*(void) {
		return *_raw;
	}

} // namespace GEN
